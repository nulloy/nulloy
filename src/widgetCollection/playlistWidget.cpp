/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2022 Sergey Vlasov <sergey@vlasov.me>
**
**  This program can be distributed under the terms of the GNU
**  General Public License version 3.0 as published by the Free
**  Software Foundation and appearing in the file LICENSE.GPL3
**  included in the packaging of this file.  Please review the
**  following information to ensure the GNU General Public License
**  version 3.0 requirements will be met:
**
**  http://www.gnu.org/licenses/gpl-3.0.html
**
*********************************************************************/

#include "playlistWidget.h"

#include <QContextMenuEvent>
#include <QDir>
#include <QDrag>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QScrollBar>
#include <QShortcut>
#include <QUrl>

#include "playbackEngineInterface.h"
#include "playlistDataItem.h"
#include "playlistStorage.h"
#include "playlistWidgetItem.h"
#include "pluginLoader.h"
#include "settings.h"
#include "tagReaderInterface.h"
#include "trash.h"
#include "utils.h"

NPlaylistWidget::NPlaylistWidget(QWidget *parent) : QListWidget(parent)
{
    m_fileDropBorderColor = QColor(Qt::transparent);
    m_fileDropBackground = QBrush(Qt::NoBrush);
    m_failedTextColor = QColor(Qt::red);
    m_currentTextColor = QColor(Qt::black);
    m_fileDropRadius = 0;

    m_tagReader = dynamic_cast<NTagReaderInterface *>(NPluginLoader::getPlugin(N::TagReader));
    Q_ASSERT(m_tagReader);
    m_playbackEngine = dynamic_cast<NPlaybackEngineInterface *>(
        NPluginLoader::getPlugin(N::PlaybackEngine));
    Q_ASSERT(m_playbackEngine);

    connect(this, SIGNAL(itemActivated(QListWidgetItem *)), this,
            SLOT(on_itemActivated(QListWidgetItem *)));
    setItemDelegate(new NPlaylistWidgetItemDelegate(this));
    m_currentItem = NULL;

    QShortcut *revealShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Return), this);
    connect(revealShortcut, SIGNAL(activated()), this, SLOT(on_revealAction_triggered()));
    QAction *revealAction = new QAction(QIcon::fromTheme("fileopen", style()->standardIcon(
                                                                         QStyle::SP_DirOpenIcon)),
                                        tr("Reveal in File Manager..."), this);
    revealAction->setShortcut(revealShortcut->key());
    connect(revealAction, SIGNAL(triggered()), this, SLOT(on_revealAction_triggered()));

    QShortcut *removeShortcut = new QShortcut(QKeySequence(QKeySequence::Delete), this);
    connect(removeShortcut, SIGNAL(activated()), this, SLOT(on_removeAction_triggered()));
    QAction *removeAction = new QAction(QIcon::fromTheme("edit-clear",
                                                         style()->standardIcon(
                                                             QStyle::SP_DialogResetButton)),
                                        tr("Remove From Playlist"), this);
    removeAction->setShortcut(removeShortcut->key());
    connect(removeAction, SIGNAL(triggered()), this, SLOT(on_removeAction_triggered()));

    QShortcut *trashShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Delete), this);
    connect(trashShortcut, SIGNAL(activated()), this, SLOT(on_trashAction_triggered()));
    QAction *trashAction = new QAction(QIcon::fromTheme("trashcan_empty",
                                                        style()->standardIcon(QStyle::SP_TrashIcon)),
                                       tr("Move To Trash"), this);
    trashAction->setShortcut(trashShortcut->key());
    connect(trashAction, SIGNAL(triggered()), this, SLOT(on_trashAction_triggered()));

    m_contextMenu = new QMenu(this);
    m_contextMenu->addAction(revealAction);
    m_contextMenu->addAction(removeAction);
    m_contextMenu->addAction(trashAction);

    m_itemDrag = NULL;
    m_fileDrop = false;
    m_dragStart = DragStartInside;
    m_dropEnd = DropEndInside;

    m_repeatMode = NSettings::instance()->value("Repeat").toBool();
    m_shuffleMode = false;
    m_currentShuffledIndex = 0;
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDefaultDropAction(Qt::MoveAction);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDragEnabled(true);

    m_processVisibleItemsTimer = new QTimer(this);
    m_processVisibleItemsTimer->setSingleShot(true);
    connect(m_processVisibleItemsTimer, SIGNAL(timeout()), this, SLOT(processVisibleItems()));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this,
            SLOT(startProcessVisibleItemsTimer()));
}

void NPlaylistWidget::resizeEvent(QResizeEvent *event)
{
    startProcessVisibleItemsTimer();
    QListWidget::resizeEvent(event);
}

void NPlaylistWidget::startProcessVisibleItemsTimer()
{
    if (m_processVisibleItemsTimer->isActive()) {
        m_processVisibleItemsTimer->stop();
    }

    m_processVisibleItemsTimer->start(30);
}

void NPlaylistWidget::processVisibleItems()
{
    int minRow = row(itemAt(0, 0));
    QListWidgetItem *maxItem = itemAt(0, this->height());
    int maxRow = maxItem ? row(maxItem) : count() - 1;

    int totalRows = maxRow - minRow + 1;
    minRow = qMax(0, minRow - totalRows);
    maxRow = qMin(maxRow + totalRows, count() - 1);
    QString titleFormat = NSettings::instance()->value("PlaylistTrackInfo").toString();
    for (int i = minRow; i <= maxRow; ++i) {
        formatItemTitle(reinterpret_cast<NPlaylistWidgetItem *>(item(i)), titleFormat);
    }
}

void NPlaylistWidget::wheelEvent(QWheelEvent *event)
{
    QListWidget::wheelEvent(event);
    event->accept();
}

void NPlaylistWidget::contextMenuEvent(QContextMenuEvent *event)
{
    if (selectedItems().size() != 0 && itemAt(event->pos())) {
        m_contextMenu->exec(mapToGlobal(event->pos()));
    } else {
        QListWidget::contextMenuEvent(event);
    }
}

void NPlaylistWidget::on_trashAction_triggered()
{
    QStringList files;
    foreach (QListWidgetItem *item, selectedItems())
        files << QFileInfo(item->data(N::PathRole).toString()).canonicalFilePath();

    QStringList undeleted = NTrash::moveToTrash(files);
    foreach (QListWidgetItem *item, selectedItems()) {
        if (undeleted.contains(QFileInfo(item->data(N::PathRole).toString()).canonicalFilePath())) {
            continue;
        }

        delete takeItem(row(item));
    }

    viewport()->update();
}

void NPlaylistWidget::on_removeAction_triggered()
{
    int firstSelectedRow = count() - 1; // set to last row
    bool currentWithinRemoved = false;
    foreach (QListWidgetItem *item, selectedItems()) {
        int rowToDelete = row(item);
        if (firstSelectedRow > rowToDelete) {
            firstSelectedRow = rowToDelete;
        }
        NPlaylistWidgetItem *itemToDelete = reinterpret_cast<NPlaylistWidgetItem *>(
            takeItem(rowToDelete));

        if (m_currentItem == itemToDelete) {
            currentWithinRemoved = true;
            resetCurrentItem();
        }

        if (m_currentShuffledIndex >= 0 &&
            itemToDelete == m_shuffledItems.at(m_currentShuffledIndex)) {
            --m_currentShuffledIndex;
        }
        m_shuffledItems.removeAll(itemToDelete);

        delete itemToDelete;
    }
    viewport()->update();

    int newCount = count();

    if (newCount == 0) {
        return;
    }

    int newCurrentRow = firstSelectedRow;
    if (firstSelectedRow >= newCount) { // last row was within removed
        if (currentWithinRemoved && NSettings::instance()->value("LoopPlaylist").toBool()) {
            newCurrentRow = 0;
        } else {
            newCurrentRow = newCount - 1;
        }
    }
    NPlaylistWidgetItem *newCurrentItem = item(newCurrentRow);

    if (currentWithinRemoved) {
        if (m_playbackEngine->state() != N::PlaybackStopped) {
            activateItem(newCurrentItem);
        } else {
            setCurrentItem(newCurrentItem); // doesn't set keyboard focus
        }
    }
    // sets keyboard focus
    if (newCurrentItem) {
        QListWidget::setCurrentItem(newCurrentItem);
    }
}

void NPlaylistWidget::on_revealAction_triggered()
{
    QString error;
    if (!revealInFileManager(selectedItems().first()->data(N::PathRole).toString(), &error)) {
        QMessageBox::warning(this, QObject::tr("Reveal in File Manager Error"), error,
                             QMessageBox::Close);
    }
}

bool NPlaylistWidget::revealInFileManager(const QString &file, QString *error) const
{
    QFileInfo fileInfo(file);

    if (!fileInfo.exists()) {
        *error =
            QString(QObject::tr("File doesn't exist: <b>%1</b>")).arg(QFileInfo(file).fileName());
        return false;
    }

    QString cmd;
    int res = 0;

    bool customFileManager = NSettings::instance()->value("CustomFileManager").toBool();
    if (customFileManager) {
        cmd = NSettings::instance()->value("CustomFileManagerCommand").toString();
        if (cmd.isEmpty()) {
            *error = QString(QObject::tr("Custom File Manager is enabled but not configured."));
            return false;
        }
        QString filePath = file;
        QString fileName = fileInfo.fileName();
        QString canonicalPath = fileInfo.canonicalPath();
#if defined Q_OS_WIN
        filePath.replace('/', '\\');
        fileName.replace('/', '\\');
        canonicalPath.replace('/', '\\');
#else
        // escape single quote
        filePath.replace("'", "'\\''");
        fileName.replace("'", "'\\''");
        canonicalPath.replace("'", "'\\''");
#endif
        cmd.replace("%p", filePath);
        cmd.replace("%F", fileName);
        cmd.replace("%P", canonicalPath);

#if !defined Q_OS_WIN
        res = QProcess::execute("sh", QStringList{"-c", cmd});
#else
        res = QProcess::execute(cmd);
#endif
    } else {
        QString path = fileInfo.canonicalFilePath();
#if defined Q_OS_WIN
        res = QProcess::execute("explorer.exe",
                                QStringList{"/n", ",", "/select", ",", path.replace('/', '\\')});
#elif defined Q_OS_LINUX
        res = QProcess::execute("xdg-open",
                                QStringList{fileInfo.canonicalPath().replace("'", "'\\''")});
#elif defined Q_OS_MAC
        res = QProcess::execute("open", QStringList{"-R", path.replace("'", "'\\''")});
#endif
    }

#ifndef Q_OS_WIN
    if (res != 0) {
        *error = QString(QObject::tr("Command failed with exit code <b>%1</b>.")).arg(res);
        return false;
    }
#endif

    return true;
}

NPlaylistWidget::~NPlaylistWidget() {}

void NPlaylistWidget::resetCurrentItem()
{
    m_currentItem = NULL;
    emit setMedia("");
}

void NPlaylistWidget::formatItemTitle(NPlaylistWidgetItem *item, QString titleFormat, bool force)
{
    if (!force && titleFormat == item->data(N::TitleFormatRole).toString() &&
        !item->text().isEmpty()) {
        return;
    }

    QString file = item->data(N::PathRole).toString();
    QString encoding = NSettings::instance()->value("EncodingTrackInfo").toString();
    QString title = m_tagReader->toString(file, titleFormat, encoding);
    if (title.isEmpty()) { // reading tags failed
        title = QFileInfo(file).fileName();
    }

    item->setText(title);
    item->setData(N::TitleFormatRole, titleFormat);
}

void NPlaylistWidget::setCurrentItem(NPlaylistWidgetItem *item)
{
    if (m_currentItem) {
        QFont f = m_currentItem->font();
        f.setBold(false);
        m_currentItem->setFont(f);
        m_currentItem->setData(N::PositionRole, m_playbackEngine->position());
        m_currentItem->setData(N::CountRole, m_currentItem->data(N::CountRole).toInt() + 1);
    }

    if (!item) {
        resetCurrentItem();
        return;
    }

    item->setData(N::FailedRole, false); // reset failed role
    formatItemTitle(item, NSettings::instance()->value("PlaylistTrackInfo").toString(),
                    true); // with force

    // setting currently playing font to bold, colors set in delegate
    QFont f = item->font();
    f.setBold(true);
    item->setFont(f);

    scrollToItem(item);
    m_currentItem = item;
    update();

    QString file = item->data(N::PathRole).toString();
    emit setMedia(file);
}

void NPlaylistWidget::currentFailed()
{
    m_currentItem->setData(N::FailedRole, true);
}

int NPlaylistWidget::currentRow() const
{
    if (m_currentItem) {
        return row(m_currentItem);
    } else {
        return -1;
    }
}

bool NPlaylistWidget::hasCurrent() const
{
    return currentRow() != -1;
}

QModelIndex NPlaylistWidget::currentIndex() const
{
    if (m_currentItem) {
        return indexFromItem(m_currentItem);
    } else {
        return QModelIndex();
    }
}

QString NPlaylistWidget::currentTitle() const
{
    if (m_currentItem) {
        return m_currentItem->text();
    } else {
        return "";
    }
}

void NPlaylistWidget::setCurrentRow(int row)
{
    if (row < 0) {
        return;
    }

    if (row < count()) {
        setCurrentItem(item(row));
    }
}

void NPlaylistWidget::playRow(int row)
{
    if (row > -1) {
        activateItem(item(row));
    }
}

void NPlaylistWidget::activateItem(NPlaylistWidgetItem *item)
{
    if (count() > 0) {
        emit itemActivated(item);
    } else {
        resetCurrentItem();
    }
}

void NPlaylistWidget::on_itemActivated(QListWidgetItem *item)
{
    setCurrentItem(reinterpret_cast<NPlaylistWidgetItem *>(item));
    emit currentActivated();

    if (m_shuffleMode) {
        m_currentShuffledIndex = m_shuffledItems.indexOf(m_currentItem);
    }
}

void NPlaylistWidget::addFiles(const QStringList &files)
{
    foreach (QString path, files)
        addItem(new NPlaylistWidgetItem(QFileInfo(path)));
    processVisibleItems();
}

void NPlaylistWidget::addItems(const QList<NPlaylistDataItem> &dataItems)
{
    foreach (NPlaylistDataItem dataItem, dataItems)
        addItem(new NPlaylistWidgetItem(dataItem));
    processVisibleItems();
}

void NPlaylistWidget::setFiles(const QStringList &files)
{
    clear();
    m_shuffledItems.clear();
    m_currentItem = NULL;
    foreach (QString path, files)
        addItem(new NPlaylistWidgetItem(QFileInfo(path)));
    processVisibleItems();
}

void NPlaylistWidget::setItems(const QList<NPlaylistDataItem> &dataItems)
{
    clear();
    m_shuffledItems.clear();
    m_currentItem = NULL;
    foreach (NPlaylistDataItem dataItem, dataItems)
        addItem(new NPlaylistWidgetItem(dataItem));
    processVisibleItems();
}

bool NPlaylistWidget::setPlaylist(const QString &file)
{
    clear();
    m_shuffledItems.clear();
    m_currentItem = NULL;

    QList<NPlaylistDataItem> dataItemsList = NPlaylistStorage::readM3u(file);

    if (dataItemsList.isEmpty()) {
        return false;
    }

    for (int i = 0; i < dataItemsList.count(); ++i) {
        addItem(new NPlaylistWidgetItem(dataItemsList.at(i)));
    }

    processVisibleItems();

    return true;
}

void NPlaylistWidget::playFiles(const QStringList &files)
{
    setFiles(files);
    playRow(0);
}

void NPlaylistWidget::playItems(const QList<NPlaylistDataItem> &dataItems)
{
    setItems(dataItems);
    playRow(0);
}

void NPlaylistWidget::playNextItem()
{
    if (count() == 0) {
        return;
    }

    if (m_shuffleMode) {
        m_currentShuffledIndex++;
        if (m_currentShuffledIndex >= m_shuffledItems.count()) {
            m_currentShuffledIndex = 0;
        }
        NPlaylistWidgetItem *nextItem = m_shuffledItems.at(m_currentShuffledIndex);
        activateItem(nextItem);
    } else {
        NPlaylistWidgetItem *nextItem = NULL;

        if (currentRow() == count() - 1 &&
            NSettings::instance()->value("LoadNext").toBool()) { // last row
            QDir::SortFlag flag =
                (QDir::SortFlag)NSettings::instance()->value("LoadNextSort").toInt();
            QString file = m_currentItem->data(N::PathRole).toString();
            QString path = QFileInfo(file).path();
            QStringList entryList = QDir(path).entryList(
                NSettings::instance()->value("FileFilters").toString().split(' '),
                QDir::Files | QDir::NoDotAndDotDot, flag);
            int index = entryList.indexOf(QFileInfo(file).fileName());
            if (index != -1 && entryList.size() > index + 1) {
                nextItem = new NPlaylistWidgetItem(QFileInfo(path + "/" + entryList.at(index + 1)));
                addItem(nextItem);
                activateItem(item(currentRow() + 1));
            }
        } else {
            nextItem = item(currentRow() + 1, NSettings::instance()->value("LoopPlaylist").toBool());
        }

        if (nextItem) {
            activateItem(nextItem);
        }
    }
}

void NPlaylistWidget::currentFinished()
{
    if (m_repeatMode) {
        activateItem(m_currentItem);
    } else {
        playNextItem();
    }
}

void NPlaylistWidget::playPrevItem()
{
    if (m_shuffleMode) {
        m_currentShuffledIndex--;
        if (m_currentShuffledIndex < 0) {
            m_currentShuffledIndex = m_shuffledItems.count() - 1;
        }
        activateItem(m_shuffledItems.at(m_currentShuffledIndex));
    } else {
        NPlaylistWidgetItem *prevItem = item(currentRow() - 1,
                                             NSettings::instance()->value("LoopPlaylist").toBool());
        if (prevItem) {
            activateItem(prevItem);
        }
    }
}

void NPlaylistWidget::rowsInserted(const QModelIndex &parent, int start, int end)
{
    for (int i = start; i < end + 1; ++i) {
        m_shuffledItems.append(item(i));
    }
    if (m_shuffleMode) {
        setShuffleMode(true);
    }
    QListWidget::rowsInserted(parent, start, end);
}

bool NPlaylistWidget::shuffleMode() const
{
    return m_shuffleMode;
}

void NPlaylistWidget::setShuffleMode(bool enable)
{
    if (m_shuffleMode != enable) {
        emit shuffleModeChanged(enable);
    }
    m_shuffleMode = enable;

    NSettings::instance()->setValue("Shuffle", enable);

    if (!enable) {
        return;
    }

    for (int i = count() - 1; i > 0; --i) {
        m_shuffledItems.swap(i, qrand() % (i + 1));
    }

    // move current item to the beginning
    if (m_currentItem) {
        m_shuffledItems.swap(m_shuffledItems.indexOf(m_currentItem), 0);
    }
    m_currentShuffledIndex = 0;
}

bool NPlaylistWidget::repeatMode() const
{
    return m_repeatMode;
}

void NPlaylistWidget::setRepeatMode(bool enable)
{
    if (m_repeatMode != enable) {
        emit repeatModeChanged(enable);
    }
    m_repeatMode = enable;
    NSettings::instance()->setValue("Repeat", enable);
}

NPlaylistWidgetItem *NPlaylistWidget::item(int row, bool loop) const
{
    if (count() == 0) {
        return NULL;
    }

    int resultRow = row;
    if (loop) {
        if (row < 0) {
            resultRow = count() - 1;
        } else if (row >= count()) {
            resultRow = 0;
        }
    } else {
        if (qBound(0, row, count() - 1) != row) {
            return NULL;
        }
    }

    return reinterpret_cast<NPlaylistWidgetItem *>(QListWidget::item(resultRow));
}

void NPlaylistWidget::paintEvent(QPaintEvent *e)
{
    QListWidget::paintEvent(e);

    if (count() == 0) {
        QPainter painter(viewport());
        painter.setRenderHint(QPainter::Antialiasing);

        int iconSide = 120;
        painter.translate(0, (iconSide + fontMetrics().height()) / 2);
        QRect iconRect = QRect(0, 0, iconSide, iconSide);
        QRect rect = viewport()->rect();
        iconRect.moveCenter(rect.center());
        iconRect.moveBottom(rect.center().y() - fontMetrics().height());

        QPen pen = painter.pen();
        pen.setWidth(4);
        pen.setStyle(Qt::CustomDashLine);
        pen.setDashOffset(6);
        pen.setDashPattern(QVector<qreal>() << 3 << 4);
        painter.setPen(pen);
        painter.drawRoundedRect(iconRect, 20, 20);
        painter.drawText(viewport()->rect(), Qt::AlignHCenter | Qt::AlignVCenter,
                         tr("Drop media here"));

        painter.setPen(Qt::NoPen);
        QBrush brush = painter.brush();
        brush.setColor(pen.color());
        brush.setStyle(Qt::SolidPattern);
        painter.setBrush(brush);
        static const QPoint points[7] = {QPoint(33, 78), QPoint(66, 39), QPoint(51, 39),
                                         QPoint(51, 0),  QPoint(15, 0),  QPoint(15, 39),
                                         QPoint(0, 39)};
        painter.translate((rect.width() - iconSide) / 2 + 26, (rect.height() - iconSide) / 2 - 54);
        painter.drawPolygon(points, 7);
    }

    if (m_fileDrop) {
        QPainter painter(viewport());
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(m_fileDropBorderColor);
        painter.setBrush(m_fileDropBackground);
        painter.drawRoundedRect(QRectF(viewport()->rect()).adjusted(0.5, 0.5, -0.5, -0.5),
                                m_fileDropRadius, m_fileDropRadius);
    }
}

// DRAG & DROP >>
bool NPlaylistWidget::dropMimeData(int index, const QMimeData *data, Qt::DropAction action)
{
    if (action == Qt::MoveAction && m_dragStart == DragStartInside) { // moving within playlist
        return false;
    }

    bool wasEmpty = false;
    if (count() == 0) {
        wasEmpty = true;
    }

    QString titleFormat = NSettings::instance()->value("PlaylistTrackInfo").toString();
    foreach (QUrl url, data->urls()) {
        foreach (NPlaylistDataItem dataItem, NUtils::dirListRecursive(url.toLocalFile())) {
            NPlaylistWidgetItem *item = new NPlaylistWidgetItem(dataItem);
            insertItem(index, item);
            formatItemTitle(item, titleFormat);
            ++index;
        }
    }

    if (wasEmpty) {
        playRow(0);
    }

    m_itemDrag = NULL;
    return true;
}

QStringList NPlaylistWidget::mimeTypes() const
{
    QStringList qstrList;
    qstrList.append("text/uri-list");
    return qstrList;
}

QMimeData *NPlaylistWidget::mimeData(const QList<QListWidgetItem *> items) const
{
    QList<QUrl> urls;
    foreach (QListWidgetItem *item, items)
        urls << QUrl::fromLocalFile(item->data(N::PathRole).toString());
    QPointer<QMimeData> data = new QMimeData();

    data->setUrls(urls);

    return data;
}

void NPlaylistWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton)) {
        return;
    }

    if (!itemAt(event->pos())) {
        selectionModel()->clearSelection();
        return;
    }

    if (selectedItems().isEmpty()) {
        return;
    }

    QList<QUrl> urls;
    foreach (QListWidgetItem *item, selectedItems())
        urls << QUrl::fromLocalFile(item->data(N::PathRole).toString());

    QMimeData *mimeData = new QMimeData;
    mimeData->setUrls(urls);
    m_mimeDataUrls.clear();

    m_itemDrag = new QDrag(this);
    m_itemDrag->setMimeData(mimeData);
    Qt::DropAction dropAction = m_itemDrag->exec(Qt::CopyAction | Qt::MoveAction,
                                                 Qt::MoveAction);      // blocking
    if (dropAction == Qt::MoveAction && m_dropEnd == DropEndOutside) { // dropping to file manager
        on_removeAction_triggered();
    }
}

void NPlaylistWidget::dropEvent(QDropEvent *event)
{
    if (event->source()) { // moving within playlist
        m_dragStart = DragStartInside;
    } else { // dropping from file manager
        m_dragStart = DragStartOutside;
    }

    QListWidget::dropEvent(event);

    m_fileDrop = false;
    viewport()->update();
}

void NPlaylistWidget::dragEnterEvent(QDragEnterEvent *event)
{
    m_dropEnd = DropEndInside;

    if (m_itemDrag && !m_mimeDataUrls.isEmpty()) {
        m_itemDrag->mimeData()->setUrls(m_mimeDataUrls); // recover old data
    }

    if (!m_itemDrag) {
        m_fileDrop = true;
        viewport()->update();
    }

    QListWidget::dragEnterEvent(event);
}

void NPlaylistWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (!m_itemDrag) {
        m_fileDrop = (!itemAt(event->pos())) ? true : false;
    }

    QListWidget::dragMoveEvent(event);
}

void NPlaylistWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    m_dropEnd = DropEndOutside;

    if (m_itemDrag) {
        m_mimeDataUrls = m_itemDrag->mimeData()->urls(); // backup
    }

    m_fileDrop = false;
    viewport()->update();

    QListWidget::dragLeaveEvent(event);
}
// << DRAG & DROP

// STYLESHEET PROPERTIES >>
void NPlaylistWidget::setCurrentTextColor(QColor color)
{
    m_currentTextColor = color;
}

QColor NPlaylistWidget::currentTextColor() const
{
    return m_currentTextColor;
}

void NPlaylistWidget::setFailedTextColor(QColor color)
{
    m_failedTextColor = color;
}

QColor NPlaylistWidget::failedTextColor() const
{
    return m_failedTextColor;
}

QColor NPlaylistWidget::fileDropBorderColor() const
{
    return m_fileDropBorderColor;
}

void NPlaylistWidget::setFileDropBorderColor(QColor color)
{
    m_fileDropBorderColor = color;
}

QBrush NPlaylistWidget::fileDropBackground() const
{
    return m_fileDropBackground;
}

void NPlaylistWidget::setFileDropBackground(QBrush brush)
{
    m_fileDropBackground = brush;
}

int NPlaylistWidget::fileDropRadius() const
{
    return m_fileDropRadius;
}

void NPlaylistWidget::setFileDropRadius(int radius)
{
    m_fileDropRadius = radius;
}
// << STYLESHEET PROPERTIES
