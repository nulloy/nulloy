/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2024 Sergey Vlasov <sergey@vlasov.me>
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
#include <QDrag>
#include <QMap>
#include <QMenu>
#include <QMessageBox>
#include <QScrollBar>
#include <QShortcut>

#include "action.h"
#include "playbackEngineInterface.h"
#include "playlistDataItem.h"
#include "playlistStorage.h"
#include "playlistWidgetItem.h"
#include "pluginLoader.h"
#include "settings.h"
#include "trackInfoReader.h"
#include "trash.h"
#include "utils.h"

#ifdef Q_OS_WIN
#include "winIcon.h"
#endif

NPlaylistWidget::NPlaylistWidget(QWidget *parent) : QListWidget(parent)
{
    m_fileDropBorderColor = QColor(Qt::transparent);
    m_fileDropBackground = QBrush(Qt::NoBrush);
    m_failedTextColor = QColor(Qt::red);
    m_playingTextColor = QColor(Qt::black);
    m_fileDropRadius = 0;

    m_trackInfoReader = NULL;

    m_playbackEngine = dynamic_cast<NPlaybackEngineInterface *>(
        NPluginLoader::getPlugin(N::PlaybackEngine));
    Q_ASSERT(m_playbackEngine);

    QList<QIcon> winIcons;
#ifdef Q_OS_WIN
    winIcons = NWinIcon::getIcons(QProcessEnvironment::systemEnvironment().value("SystemRoot") +
                                  "/system32/imageres.dll");
#endif

    // triggered bu user input (double click or enter, depending on platform):
    connect(this, &QListWidget::itemActivated, [this](QListWidgetItem *item) {
        playItem(reinterpret_cast<NPlaylistWidgetItem *>(item));
    });
    connect(m_playbackEngine, SIGNAL(mediaFinished(const QString &, int)), this,
            SLOT(on_playbackEngine_mediaFinished(const QString &, int)));
    connect(m_playbackEngine, SIGNAL(mediaFailed(const QString &, int)), this,
            SLOT(on_playbackEngine_mediaFailed(const QString &, int)));
    connect(m_playbackEngine, SIGNAL(mediaChanged(const QString &, int)), this,
            SLOT(on_playbackEngine_mediaChanged(const QString &, int)));
    connect(m_playbackEngine, SIGNAL(nextMediaRequested()), this,
            SLOT(on_playbackEngine_prepareNextMediaRequested()), Qt::BlockingQueuedConnection);

    setItemDelegate(new NPlaylistWidgetItemDelegate(this));
    m_playingItem = NULL;

    NAction *revealAction = new NAction(QIcon::fromTheme("fileopen", winIcons.value(13)),
                                        tr("Reveal in File Manager..."), this);
    revealAction->setObjectName("RevealInFileManagerAction");
    revealAction->setStatusTip(tr("Open file manager for selected file"));
    revealAction->setCustomizable(true);
    this->addAction(revealAction);
    connect(revealAction, SIGNAL(triggered()), this, SLOT(on_revealAction_triggered()));

    NAction *removeAction = new NAction(QIcon::fromTheme("remove"), tr("Remove From Playlist"),
                                        this);
    removeAction->setObjectName("RemoveFromPlaylistAction");
    removeAction->setStatusTip(tr("Remove selected files from playlist"));
    removeAction->setCustomizable(true);
    this->addAction(removeAction);
    connect(removeAction, SIGNAL(triggered()), this, SLOT(on_removeAction_triggered()));

    NAction *trashAction = new NAction(QIcon::fromTheme("trashcan_empty", winIcons.value(49)),
                                       tr("Move To Trash"), this);
    trashAction->setObjectName("MoveToTrashAction");
    trashAction->setStatusTip(tr("Move selected files to trash bin"));
    trashAction->setCustomizable(true);
    this->addAction(trashAction);
    connect(trashAction, SIGNAL(triggered()), this, SLOT(on_trashAction_triggered()));

    m_contextMenu = new QMenu(this);
    m_contextMenu->addAction(revealAction);
    m_contextMenu->addAction(removeAction);
    m_contextMenu->addAction(trashAction);

    if (dynamic_cast<NTagReaderInterface *>(NPluginLoader::getPlugin(N::TagReader))
            ->isWriteSupported()) {
        NAction *tagEditorAction = new NAction(QIcon::fromTheme("edit", winIcons.value(289)),
                                               tr("Tag Editor"), this);
        tagEditorAction->setObjectName("TagEditorAction");
        tagEditorAction->setStatusTip(tr("Open tag editor for selected file"));
        tagEditorAction->setCustomizable(true);
        this->addAction(tagEditorAction);
        connect(tagEditorAction, SIGNAL(triggered()), this, SLOT(on_tagEditorAction_triggered()));
        m_contextMenu->addAction(tagEditorAction);
    }

    m_itemDrag = NULL;
    m_fileDrop = false;
    m_dragStart = DragStartInside;
    m_dropEnd = DropEndInside;

    m_repeatMode = NSettings::instance()->value("Repeat").toBool();
    m_shuffleMode = false;
    m_playingShuffledIndex = 0;
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDefaultDropAction(Qt::MoveAction);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDragEnabled(true);

    m_processVisibleItemsTimer = new QTimer(this);
    m_processVisibleItemsTimer->setSingleShot(true);
    connect(m_processVisibleItemsTimer, &QTimer::timeout, [this]() {
        processVisibleItems();
        calculateDuration();
    });
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this,
            SLOT(startProcessVisibleItemsTimer()));

    NSettings::instance()->initShortcuts(this);
}

void NPlaylistWidget::setTrackInfoReader(NTrackInfoReader *reader)
{
    m_trackInfoReader = reader;
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
        NPlaylistWidgetItem *item = itemAtRow(i);
        formatItemTitle(item, titleFormat);

        // assuming m_trackInfoReader is still set to correct file after formatItemTitle():
        if (item->data(N::DurationRole).toInt() == -1 && m_trackInfoReader) {
            item->setData(N::DurationRole, m_trackInfoReader->toString("%D").toInt());
        }
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
    bool playingItemRemoved = false;
    foreach (QListWidgetItem *item, selectedItems()) {
        int rowToDelete = row(item);
        if (firstSelectedRow > rowToDelete) {
            firstSelectedRow = rowToDelete;
        }
        NPlaylistWidgetItem *itemToDelete = reinterpret_cast<NPlaylistWidgetItem *>(
            takeItem(rowToDelete));

        if (m_playingItem == itemToDelete) {
            playingItemRemoved = true;
            m_playingItem = NULL;
        }

        if (m_playingShuffledIndex >= 0 &&
            itemToDelete == m_shuffledItems.at(m_playingShuffledIndex)) {
            --m_playingShuffledIndex;
        }
        m_shuffledItems.removeAll(itemToDelete);

        m_itemMap.remove(itemToDelete->data(N::IdRole).toInt());
        delete itemToDelete;
    }
    viewport()->update();

    updateTrackIndexes();
    calculateDuration();
    emit itemsChanged();

    int newCount = count();
    if (newCount == 0) {
        playItem(NULL);
        return;
    }

    int newCurrentRow = firstSelectedRow;
    if (firstSelectedRow >= newCount) { // last row was within removed
        if (playingItemRemoved && NSettings::instance()->value("LoopPlaylist").toBool()) {
            newCurrentRow = 0;
        } else {
            newCurrentRow = newCount - 1;
        }
    }
    NPlaylistWidgetItem *newCurrentItem = itemAtRow(newCurrentRow);

    if (playingItemRemoved && m_playbackEngine->state() != N::PlaybackStopped) {
        playItem(newCurrentItem);
    }
    // sets keyboard focus:
    if (newCurrentItem) {
        QListWidget::setCurrentItem(newCurrentItem);
    }
}

void NPlaylistWidget::on_revealAction_triggered()
{
    QString error;
    if (!revealInFileManager(selectedItems().first()->data(N::PathRole).toString(), &error)) {
        QMessageBox::warning(this, tr("Reveal in File Manager Error"), error, QMessageBox::Close);
    }
}

void NPlaylistWidget::on_tagEditorAction_triggered()
{
    if (selectedItems().isEmpty()) {
        return;
    }
    emit tagEditorRequested(selectedItems().first()->data(N::PathRole).toString());
}

bool NPlaylistWidget::revealInFileManager(const QString &file, QString *error) const
{
    QFileInfo fileInfo(file);

    if (!fileInfo.exists()) {
        *error = tr("File doesn't exist: <b>%1</b>").arg(QFileInfo(file).fileName());
        return false;
    }

    QString cmd;
    int res = 0;

    bool customFileManager = NSettings::instance()->value("CustomFileManager").toBool();
    if (customFileManager) {
        cmd = NSettings::instance()->value("CustomFileManagerCommand").toString();
        if (cmd.isEmpty()) {
            *error = tr("Custom File Manager is enabled but not configured.");
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
        QStringList args;
#if defined Q_OS_WIN
        cmd = "explorer.exe";
        args = QStringList{"/n", ",", "/select", ",", path.replace('/', '\\')};
#elif defined Q_OS_LINUX
        cmd = "xdg-open";
        args = QStringList{fileInfo.canonicalPath().replace("'", "'\\''")};
#elif defined Q_OS_MAC
        cmd = "open";
        args = QStringList{"-R", path.replace("'", "'\\''")};
#endif
        res = QProcess::execute(cmd, args);
        cmd += " " + args.join(' ');
    }

#ifndef Q_OS_WIN
    if (res != 0) {
        *error = tr("File manager command failed with exit code <b>%1</b>:").arg(res) +
                 QString("<br><br><span style=\"font-family: 'Lucida Console', Monaco, "
                         "monospace\">%1</span>")
                     .arg(cmd);
        return false;
    }
#endif

    return true;
}

void NPlaylistWidget::calculateDuration()
{
    int secondsTotal = 0;
    for (int i = 0; i < count(); ++i) {
        NPlaylistWidgetItem *item = itemAtRow(i);
        int seconds = item->data(N::DurationRole).toInt();
        if (seconds > 0) {
            secondsTotal += seconds;
        }
    }

    emit durationChanged(secondsTotal);
}

void NPlaylistWidget::updateTrackIndexes()
{
    for (int i = 0; i < count(); ++i) {
        itemAtRow(i)->setData(N::TrackIndexRole, i);
    }
}

NPlaylistWidget::~NPlaylistWidget() {}

void NPlaylistWidget::resetPlayingItem()
{
    if (m_playingItem) {
        m_playingItem->setData(N::PlayingRole, false);
    }
    m_playingItem = NULL;
}

void NPlaylistWidget::formatItemTitle(NPlaylistWidgetItem *item, QString titleFormat, bool force)
{
    if (!force && titleFormat == item->data(N::TitleFormatRole).toString() &&
        !item->text().isEmpty()) {
        return;
    }

    QString file = item->data(N::PathRole).toString();
    QString title;
    if (m_trackInfoReader) {
        m_trackInfoReader->setSource(file);
        title = m_trackInfoReader->toString(titleFormat);
    }

    item->setText(title);
    item->setData(N::TitleFormatRole, titleFormat);
}

void NPlaylistWidget::setPlayingItem(NPlaylistWidgetItem *item)
{
    item->setData(N::PlayingRole, true);
    item->setData(N::FailedRole, false); // reset failed role
    formatItemTitle(item, NSettings::instance()->value("PlaylistTrackInfo").toString(),
                    true); // with force

    if (NSettings::instance()->value("ScrollToItem").toBool()) {
        scrollToItem(item);
    }
    m_playingItem = item;
    emit playingItemChanged();
    QListWidget::viewport()->update();
}

void NPlaylistWidget::on_playbackEngine_mediaChanged(const QString &file, int id)
{
    if (m_playingItem) {
        m_playingItem->setData(N::PlayingRole, false);
        m_playingItem->setData(N::PositionRole, m_playbackEngine->position());
        m_playingItem->setData(N::CountRole, m_playingItem->data(N::CountRole).toInt() + 1);
    }

    resetPlayingItem();

    if (!m_itemMap.contains(id)) {
        emit playingItemChanged();
        QListWidget::viewport()->update();
        return;
    }

    NPlaylistWidgetItem *item = m_itemMap[id];
    setPlayingItem(item);

    if (m_shuffleMode) {
        m_playingShuffledIndex = m_shuffledItems.indexOf(m_playingItem);
    }
}

void NPlaylistWidget::on_playbackEngine_prepareNextMediaRequested()
{
    NPlaylistWidgetItem *item = NULL;
    if (m_repeatMode) {
        item = playingItem();
    } else {
        item = nextItem(m_playingItem);
    }
    if (!item) {
        return;
    }
    m_playbackEngine->nextMediaRespond(item->data(N::PathRole).toString(),
                                       item->data(N::IdRole).toInt());
}

void NPlaylistWidget::on_playbackEngine_mediaFinished(const QString &, int id)
{
    if (!m_itemMap.contains(id)) {
        return;
    }

    NPlaylistWidgetItem *item = m_itemMap[id];
    if (m_repeatMode) {
        playItem(item);
    } else {
        playItem(nextItem(item));
    }
}

void NPlaylistWidget::on_playbackEngine_mediaFailed(const QString &file, int id)
{
    if (!m_itemMap.contains(id)) {
        return;
    }

    resetPlayingItem();

    NPlaylistWidgetItem *item = m_itemMap[id];
    item->setData(N::PlayingRole, true);
    item->setData(N::FailedRole, true);

    if (NSettings::instance()->value("ScrollToItem").toBool()) {
        scrollToItem(item);
    }
    m_playingItem = item;
    emit playingItemChanged();
    QListWidget::viewport()->update();
}

void NPlaylistWidget::playItem(NPlaylistWidgetItem *item)
{
    if (item) {
        m_playbackEngine->setMedia(item->data(N::PathRole).toString(),
                                   item->data(N::IdRole).toInt());
        m_playbackEngine->play();
    } else {
        m_playbackEngine->setMedia("", 0);
    }
}

void NPlaylistWidget::playRow(int row)
{
    if (row < 0 || row >= count()) {
        resetPlayingItem();
        return;
    }

    playItem(itemAtRow(row));
}

int NPlaylistWidget::playingRow() const
{
    if (m_playingItem) {
        return row(m_playingItem);
    } else {
        return -1;
    }
}

NPlaylistWidgetItem *NPlaylistWidget::playingItem() const
{
    return m_playingItem;
}

bool NPlaylistWidget::hasPlayingItem() const
{
    return playingRow() != -1;
}

void NPlaylistWidget::addItem(NPlaylistWidgetItem *item)
{
    QListWidget::addItem(item);
    m_itemMap[item->data(N::IdRole).toInt()] = item;
}

void NPlaylistWidget::addFiles(const QStringList &files)
{
    foreach (QString path, files)
        addItem(new NPlaylistWidgetItem(QFileInfo(path)));

    processVisibleItems();
    calculateDuration();
    updateTrackIndexes();
    emit itemsChanged();
}

void NPlaylistWidget::addItems(const QList<NPlaylistDataItem> &dataItems)
{
    foreach (NPlaylistDataItem dataItem, dataItems)
        addItem(new NPlaylistWidgetItem(dataItem));

    processVisibleItems();
    calculateDuration();
    updateTrackIndexes();
    emit itemsChanged();
}

void NPlaylistWidget::setFiles(const QStringList &files)
{
    clear();
    m_shuffledItems.clear();
    m_playingItem = NULL;
    foreach (QString path, files)
        addItem(new NPlaylistWidgetItem(QFileInfo(path)));

    processVisibleItems();
    calculateDuration();
    updateTrackIndexes();
    emit itemsChanged();
}

void NPlaylistWidget::setItems(const QList<NPlaylistDataItem> &dataItems)
{
    clear();
    m_shuffledItems.clear();
    m_playingItem = NULL;

    foreach (NPlaylistDataItem dataItem, dataItems)
        addItem(new NPlaylistWidgetItem(dataItem));

    processVisibleItems();
    calculateDuration();
    updateTrackIndexes();
    emit itemsChanged();
}

bool NPlaylistWidget::setPlaylist(const QString &file)
{
    clear();
    m_shuffledItems.clear();
    m_playingItem = NULL;

    QList<NPlaylistDataItem> dataItemsList = NPlaylistStorage::readM3u(file);

    if (dataItemsList.isEmpty()) {
        calculateDuration();
        emit itemsChanged();
        return false;
    }

    for (int i = 0; i < dataItemsList.count(); ++i) {
        addItem(new NPlaylistWidgetItem(dataItemsList.at(i)));
    }

    processVisibleItems();
    calculateDuration();
    updateTrackIndexes();
    emit itemsChanged();

    return true;
}

void NPlaylistWidget::playNextItem()
{
    NPlaylistWidgetItem *item = NULL;

    if (m_shuffleMode) {
        m_playingShuffledIndex++;
        if (m_playingShuffledIndex >= m_shuffledItems.count()) {
            m_playingShuffledIndex = 0;
        }
        item = m_shuffledItems.at(m_playingShuffledIndex);
    } else {
        item = nextItem(m_playingItem);
        if (!item) {
            emit addMoreRequested();
            item = nextItem(m_playingItem);
        }
    }

    if (item) {
        playItem(item);
    }
}

void NPlaylistWidget::playPrevItem()
{
    NPlaylistWidgetItem *item = NULL;

    if (m_shuffleMode) {
        m_playingShuffledIndex--;
        if (m_playingShuffledIndex < 0) {
            m_playingShuffledIndex = m_shuffledItems.count() - 1;
        }
        item = m_shuffledItems.at(m_playingShuffledIndex);
    } else {
        item = prevItem(m_playingItem);
    }

    if (item) {
        playItem(item);
    }
}

void NPlaylistWidget::rowsInserted(const QModelIndex &parent, int start, int end)
{
    for (int i = start; i < end + 1; ++i) {
        m_shuffledItems.append(itemAtRow(i));
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

    // move playing item to the beginning
    if (m_playingItem) {
        m_shuffledItems.swap(m_shuffledItems.indexOf(m_playingItem), 0);
    }
    m_playingShuffledIndex = 0;
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

NPlaylistWidgetItem *NPlaylistWidget::itemAtRow(int row) const
{
    if (count() == 0) {
        return NULL;
    }

    if (qBound(0, row, count() - 1) != row) {
        return NULL;
    }

    return reinterpret_cast<NPlaylistWidgetItem *>(QListWidget::item(row));
}

NPlaylistWidgetItem *NPlaylistWidget::nextItem(NPlaylistWidgetItem *item) const
{
    if (!item) {
        return NULL;
    }

    int nextRow = QListWidget::row(item) + 1;
    if (nextRow >= count() && NSettings::instance()->value("LoopPlaylist").toBool()) {
        nextRow = 0;
    }

    return NPlaylistWidget::itemAtRow(nextRow);
}

NPlaylistWidgetItem *NPlaylistWidget::prevItem(NPlaylistWidgetItem *item) const
{
    if (!item) {
        return NULL;
    }

    int prevRow = QListWidget::row(item) - 1;
    if (prevRow < 0 && NSettings::instance()->value("LoopPlaylist").toBool()) {
        prevRow = count() - 1;
    }

    return NPlaylistWidget::itemAtRow(prevRow);
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
        updateTrackIndexes();
        emit itemsChanged();

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
            QListWidget::insertItem(index, item);
            m_itemMap[item->data(N::IdRole).toInt()] = item;
            ++index;
        }
    }

    if (wasEmpty) {
        playRow(0);
    }

    m_itemDrag = NULL;

    processVisibleItems();
    calculateDuration();
    updateTrackIndexes();
    emit itemsChanged();

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

    updateTrackIndexes();
    emit itemsChanged();

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
void NPlaylistWidget::setPlayingTextColor(QColor color)
{
    m_playingTextColor = color;
}

QColor NPlaylistWidget::playingTextColor() const
{
    return m_playingTextColor;
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
