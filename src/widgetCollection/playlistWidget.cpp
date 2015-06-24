/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2015 Sergey Vlasov <sergey@vlasov.me>
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

#include "settings.h"
#include "common.h"
#include "trash.h"

#include "playlistWidgetItem.h"
#include "playlistStorage.h"

#include "pluginLoader.h"
#include "tagReaderInterface.h"
#include "playbackEngineInterface.h"

#include <QContextMenuEvent>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QScrollBar>
#include <QMessageBox>
#include <QShortcut>
#include <QUrl>

NPlaylistWidget::NPlaylistWidget(QWidget *parent) : QListWidget(parent)
{
	m_fileDropBorderColor = QColor(Qt::transparent);
	m_fileDropBackground = QBrush(Qt::NoBrush);
	m_failedTextColor = QColor(Qt::red);
	m_currentTextColor = QColor(Qt::black);
	m_fileDropRadius = 0;

	m_tagReader = dynamic_cast<NTagReaderInterface *>(NPluginLoader::getPlugin(N::TagReader));
	m_playbackEngine = dynamic_cast<NPlaybackEngineInterface *>(NPluginLoader::getPlugin(N::PlaybackEngine));

	connect(this, SIGNAL(itemActivated(QListWidgetItem *)), this, SLOT(on_itemActivated(QListWidgetItem *)));
	setItemDelegate(new NPlaylistWidgetItemDelegate(this));
	m_currentItem = NULL;


	QShortcut *revealShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Return), this);
	connect(revealShortcut, SIGNAL(activated()), this, SLOT(on_revealAction_triggered()));
	QAction *revealAction = new QAction(QIcon::fromTheme("fileopen",
	                                    style()->standardIcon(QStyle::SP_DirOpenIcon)),
	                                    tr("Reveal in File Manager..."), this);
	revealAction->setShortcut(revealShortcut->key());
	connect(revealAction, SIGNAL(triggered()), this, SLOT(on_revealAction_triggered()));


	QShortcut *removeShortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
	connect(removeShortcut, SIGNAL(activated()), this, SLOT(on_removeAction_triggered()));
	QAction *removeAction = new QAction(QIcon::fromTheme("edit-clear",
	                                    style()->standardIcon(QStyle::SP_DialogResetButton)),
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

	m_repeatMode = NSettings::instance()->value("Repeat").toBool();
	m_shuffleMode = false;
	m_currentShuffledIndex = 0;
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
}

void NPlaylistWidget::wheelEvent(QWheelEvent *event)
{
	if (event->orientation() == Qt::Horizontal) {
		QListWidget::wheelEvent(event);
		return;
	}

	QScrollBar *vbar = verticalScrollBar();
	int value = vbar->value();
	int delta = event->delta();

	if ((delta < 0 && value == vbar->maximum()) || (delta > 0 && value == vbar->minimum()))
		event->accept();
	else
		QListWidget::wheelEvent(event);
}

void NPlaylistWidget::contextMenuEvent(QContextMenuEvent *event)
{
	if (selectedItems().size() != 0 && itemAt(event->pos()))
		m_contextMenu->exec(mapToGlobal(event->pos()));
	else
		QListWidget::contextMenuEvent(event);
}

void NPlaylistWidget::on_trashAction_triggered()
{
	QStringList files;
	foreach (QListWidgetItem *item, selectedItems())
		files << QFileInfo(item->data(N::PathRole).toString()).canonicalFilePath();

	QStringList undeleted = NTrash::moveToTrash(files);
	foreach (QListWidgetItem *item, selectedItems()) {
		if (undeleted.contains(QFileInfo(item->data(N::PathRole).toString()).canonicalFilePath()))
			continue;

		delete takeItem(row(item));
	}

	viewport()->update();
}

void NPlaylistWidget::on_removeAction_triggered()
{
	foreach (QListWidgetItem *item, selectedItems())
		delete takeItem(row(item));

	viewport()->update();
}

void NPlaylistWidget::on_revealAction_triggered()
{
	QString error;
	if (!revealInFileManager(selectedItems().first()->data(N::PathRole).toString(), &error))
		QMessageBox::warning(this, QObject::tr("Reveal in File Manager Error"), error, QMessageBox::Close);
}

bool NPlaylistWidget::revealInFileManager(const QString &file, QString *error)
{
	QFileInfo fileInfo(file);

	if (!fileInfo.exists()) {
		*error = QString(QObject::tr("File doesn't exist: <b>%1</b>")).arg(QFileInfo(file).fileName());
		return false;
	}

	QString cmd;

	bool customFileManager = NSettings::instance()->value("CustomFileManager").toBool();
	if (customFileManager) {
		cmd = NSettings::instance()->value("CustomFileManagerCommand").toString();
		if (cmd.isEmpty()) {
			*error = QString(QObject::tr("Custom File Manager is enabled but not configured."));
			return false;
		}
		cmd.replace("%f", fileInfo.fileName());
		cmd.replace("%d", fileInfo.canonicalPath());
	} else {
		QString path = fileInfo.canonicalFilePath();
#if defined Q_WS_WIN
		cmd = "explorer.exe /n,/select,\"" + path.replace('/', '\\') + "\"";
#elif defined Q_WS_X11
		cmd = "xdg-open \"" + fileInfo.canonicalPath() + "\"";
#elif defined Q_WS_MAC
		cmd = "open -R \"" + path + "\"";
#endif
	}

	int res = QProcess::execute(cmd);
	if (res != 0) {
		*error = QString(QObject::tr("Custom File Manager command failed with exit code <b>%1</b>.")).arg(res);
		return false;
	}

	return true;
}

NPlaylistWidget::~NPlaylistWidget() {}

void NPlaylistWidget::setCurrentItem(NPlaylistWidgetItem *item)
{
	if (!item) {
		m_currentItem = NULL;
		m_tagReader->setSource("");
		emit setMedia("");
		return;
	}

	QString file = item->data(N::PathRole).toString();
	// check if it's a playlist file:
	QList<NPlaylistDataItem> dataItemsList = NPlaylistStorage::readPlaylist(file);
	if (dataItemsList.count() > 1) {
		int index = row(item);
		int index_bkp = index;

		delete takeItem(row(item));

		foreach (NPlaylistDataItem dataItem, dataItemsList) {
			insertItem(index, new NPlaylistWidgetItem(dataItem));
			++index;
		}

		setCurrentItem(NPlaylistWidget::item(index_bkp));
		return;
	}

	// trying to read tags
	m_tagReader->setSource(file);
	if (m_tagReader->isValid()) {
		item->setText(m_tagReader->toString(NSettings::instance()->value("PlaylistTrackInfo").toString()));
		item->setData(N::DurationRole, m_tagReader->toString("%D").toInt());
	} else {
		item->setText(QFileInfo(file).fileName());
	}

	item->setData(N::FailedRole, false); // reset failed role

	// setting currently playing font to bold, colors set in delegate
	QFont f = item->font();
	if (m_currentItem) {
		// reset old item to defaults
		f.setBold(false);
		m_currentItem->setFont(f);

		m_currentItem->setData(N::PositionRole, m_playbackEngine->position());
		m_currentItem->setData(N::CountRole, m_currentItem->data(N::CountRole).toInt() + 1);
	}
	f.setBold(true);
	item->setFont(f);

	scrollToItem(item);
	m_currentItem = item;
	update();

	emit setMedia(file);
}

void NPlaylistWidget::currentFailed()
{
	m_currentItem->setData(N::FailedRole, true);
}

int NPlaylistWidget::currentRow()
{
	if (m_currentItem)
		return row(m_currentItem);
	else
		return -1;
}

bool NPlaylistWidget::hasCurrent()
{
	return currentRow() != -1;
}

QModelIndex NPlaylistWidget::currentIndex() const
{
	if (m_currentItem)
		return indexFromItem(m_currentItem);
	else
		return QModelIndex();
}

QString NPlaylistWidget::currentTitle()
{
	if (m_currentItem)
		return m_currentItem->text();
	else
		return "";
}

void NPlaylistWidget::setCurrentRow(int row)
{
	if (row < 0)
		return;

	if (row < count())
		setCurrentItem(item(row));
}

void NPlaylistWidget::playRow(int row)
{
	if (row > -1)
		activateItem(item(row));
}

void NPlaylistWidget::activateItem(NPlaylistWidgetItem *item)
{
	if (count() > 0)
		emit itemActivated(item);
	else
		emit activateEmptyFail();
}

void NPlaylistWidget::on_itemActivated(QListWidgetItem *item)
{
	setCurrentItem(reinterpret_cast<NPlaylistWidgetItem *>(item));
	emit currentActivated();

	if (m_shuffleMode)
		m_currentShuffledIndex = m_shuffledItems.indexOf(m_currentItem);
}

void NPlaylistWidget::addFiles(const QStringList &files)
{
	foreach (QString path, files)
		addItem(new NPlaylistWidgetItem(QFileInfo(path)));
}

void NPlaylistWidget::setFiles(const QStringList &files)
{
	clear();
	m_shuffledItems.clear();
	m_currentItem = NULL;
	foreach (QString path, files)
		addItem(new NPlaylistWidgetItem(QFileInfo(path)));
}

bool NPlaylistWidget::setPlaylist(const QString &file)
{
	clear();
	m_shuffledItems.clear();
	m_currentItem = NULL;

	QList<NPlaylistDataItem> dataItemsList = NPlaylistStorage::readPlaylist(file);

	if (dataItemsList.isEmpty())
		return false;

	for (int i = 0; i < dataItemsList.count(); ++i)
		addItem(new NPlaylistWidgetItem(dataItemsList.at(i)));

	return true;
}

void NPlaylistWidget::playFiles(const QStringList &files)
{
	setFiles(files);
	playRow(0);
}

void NPlaylistWidget::playNextItem()
{
	int row = currentRow();
	if (m_shuffleMode) {
		m_currentShuffledIndex++;
		if (m_currentShuffledIndex >= m_shuffledItems.count())
			m_currentShuffledIndex = 0;
		activateItem(m_shuffledItems.at(m_currentShuffledIndex));
	} else {
		if (row < count() - 1) {
			activateItem(item(row + 1));
		} else if (NSettings::instance()->value("LoopPlaylist").toBool()) {
			activateItem(item(0));
		} else if (NSettings::instance()->value("LoadNext").toBool()) {
			QDir::SortFlag flag = (QDir::SortFlag)NSettings::instance()->value("LoadNextSort").toInt();
			QString file = m_currentItem->data(N::PathRole).toString();
			QString path = QFileInfo(file).path();
			QStringList entryList = QDir(path).entryList(NSettings::instance()->value("FileFilters").toString().split(' '), QDir::Files | QDir::NoDotAndDotDot, flag);
			int index = entryList.indexOf(QFileInfo(file).fileName());
			if (index != -1 && entryList.size() > index + 1) {
				addItem(new NPlaylistWidgetItem(QFileInfo(path + "/" + entryList.at(index + 1))));
				activateItem(item(row + 1));
			}
		}
	}
}

void NPlaylistWidget::currentFinished()
{
	if (m_repeatMode)
		activateItem(m_currentItem);
	else
		playNextItem();
}

void NPlaylistWidget::playPrevItem()
{
	int row = currentRow();
	if (m_shuffleMode) {
		m_currentShuffledIndex--;
		if (m_currentShuffledIndex < 0)
			m_currentShuffledIndex = m_shuffledItems.count() - 1;
		activateItem(m_shuffledItems.at(m_currentShuffledIndex));
	} else {
		if (row > 0) {
			activateItem(item(row - 1));
		} else if (NSettings::instance()->value("LoopPlaylist").toBool()) {
			activateItem(item(count() - 1));
		}
	}
}

void NPlaylistWidget::rowsInserted(const QModelIndex &parent, int start, int end)
{
	for (int i = start; i < end + 1; ++i) {
		m_shuffledItems.append(item(i));
	}
	if (m_shuffleMode)
		setShuffleMode(true);
	QListWidget::rowsInserted(parent, start, end);
}

void NPlaylistWidget::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
	bool currentRemoved = false;
	for (int i = start; i < end + 1; ++i) {
		if (item(i) == m_currentItem)
			currentRemoved = true;
		m_shuffledItems.removeAll(item(i));
	}

	NPlaylistWidgetItem *nextItem = NULL;
	if (end < count() - 1)
		nextItem = item(end + 1);

	// set cursor focus
	if (nextItem)
		QListWidget::setCurrentItem(nextItem);
	else
		QListWidget::setCurrentItem(item(start - 1));

	if (currentRemoved) {
		if (nextItem && m_playbackEngine->state() != N::PlaybackStopped)
			activateItem(nextItem);
		else
			setCurrentItem(nextItem);
	}

	QListWidget::rowsAboutToBeRemoved(parent, start, end);
}

bool NPlaylistWidget::shuffleMode()
{
	return m_shuffleMode;
}

void NPlaylistWidget::setShuffleMode(bool enable)
{
	if (m_shuffleMode != enable)
		emit shuffleModeChanged(enable);
	m_shuffleMode = enable;

	NSettings::instance()->setValue("Shuffle", enable);

	if (!enable)
		return;

	for (int i = count() - 1; i > 0; --i)
		m_shuffledItems.swap(i, qrand() % (i + 1));

	// move current item to the beginning
	if (m_currentItem)
		m_shuffledItems.swap(m_shuffledItems.indexOf(m_currentItem), 0);
	m_currentShuffledIndex = 0;
}

bool NPlaylistWidget::repeatMode()
{
	return m_repeatMode;
}

void NPlaylistWidget::setRepeatMode(bool enable)
{
	if (m_repeatMode != enable)
		emit repeatModeChanged(enable);
	m_repeatMode = enable;
	NSettings::instance()->setValue("Repeat", enable);
}

NPlaylistWidgetItem* NPlaylistWidget::item(int row)
{
	return reinterpret_cast<NPlaylistWidgetItem *>(QListWidget::item(row));
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
		painter.drawText(viewport()->rect(), Qt::AlignHCenter | Qt::AlignVCenter, tr("Drop media here"));

		painter.setPen(Qt::NoPen);
		QBrush brush = painter.brush();
		brush.setColor(pen.color());
		brush.setStyle(Qt::SolidPattern);
		painter.setBrush(brush);
		static const QPoint points[7] = {
			QPoint(33, 78),
			QPoint(66, 39), QPoint(51, 39),
			QPoint(51, 0),  QPoint(15, 0),
			QPoint(15, 39), QPoint(0, 39)
		};
		painter.translate((rect.width() - iconSide) / 2 + 26, (rect.height() - iconSide)/ 2 - 54);
		painter.drawPolygon(points, 7);
	}

	if (m_fileDrop) {
		QPainter painter(viewport());
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setPen(m_fileDropBorderColor);
		painter.setBrush(m_fileDropBackground);
		painter.drawRoundedRect(QRectF(viewport()->rect()).adjusted(0.5, 0.5, -0.5, -0.5), m_fileDropRadius, m_fileDropRadius);
	}
}


// DRAG & DROP >>
bool NPlaylistWidget::dropMimeData(int index, const QMimeData *data, Qt::DropAction)
{
	bool wasEmpty = false;
	if (count() == 0)
		wasEmpty = true;

	foreach (QUrl url, data->urls()) {
		foreach (QString file, NCore::dirListRecursive(url.toLocalFile(), NSettings::instance()->value("FileFilters").toString().split(' '))) {
			insertItem(index, new NPlaylistWidgetItem(QFileInfo(file)));
			++index;
		}
	}

	if (wasEmpty)
		playRow(0);

	m_itemDrag = NULL;
	return true;
}

QStringList NPlaylistWidget::mimeTypes() const
{
	QStringList qstrList;
	qstrList.append("text/uri-list");
	return qstrList;
}

#ifdef Q_WS_MAC
Qt::DropActions NPlaylistWidget::supportedDropActions() const
{
	return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}
#endif

QMimeData* NPlaylistWidget::mimeData(const QList<NPlaylistWidgetItem *> items) const
{
	QList<QUrl> urls;
	foreach (NPlaylistWidgetItem *item, items)
		urls << QUrl::fromLocalFile(item->data(N::PathRole).toString());

	QPointer<QMimeData> data = new QMimeData();
	data->setUrls(urls);

	return data;
}

void NPlaylistWidget::mouseMoveEvent(QMouseEvent *event)
{
	if (!(event->buttons() & Qt::LeftButton))
		return;

	if (!itemAt(event->pos())) {
		selectionModel()->clearSelection();
		return;
	}

	if (selectedItems().isEmpty())
		return;

	QList<QUrl> urls;
	urls << QUrl::fromLocalFile(currentItem()->data(N::PathRole).toString());

	QMimeData *mimeData = new QMimeData;
	mimeData->setUrls(urls);
	m_mimeDataUrls.clear();

	m_itemDrag = new QDrag(this);
	m_itemDrag->setMimeData(mimeData);
	// restrct to move action
	m_itemDrag->start(Qt::MoveAction);
}

void NPlaylistWidget::dropEvent(QDropEvent *event)
{
	if (m_itemDrag) // moving withing playlist
		event->setDropAction(Qt::MoveAction);
	else // dropping from file manager
		event->setDropAction(Qt::CopyAction);

	QListWidget::dropEvent(event);

	m_fileDrop = false;
	viewport()->update();
}

void NPlaylistWidget::dragEnterEvent(QDragEnterEvent *event)
{
	if (m_itemDrag && !m_mimeDataUrls.isEmpty())
		m_itemDrag->mimeData()->setUrls(m_mimeDataUrls); // recover old data

	if (!m_itemDrag) {
		m_fileDrop = true;
		viewport()->update();
	}

	// change to move action
	event->setDropAction(Qt::MoveAction);
	QListWidget::dragEnterEvent(event);
}

void NPlaylistWidget::dragMoveEvent(QDragMoveEvent *event)
{
	// change to move action
	event->setDropAction(Qt::MoveAction);
	QListWidget::dragMoveEvent(event);

	if (!m_itemDrag)
		m_fileDrop = (!itemAt(event->pos())) ? true : false;
}

void NPlaylistWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
	if (m_itemDrag) {
		m_mimeDataUrls = m_itemDrag->mimeData()->urls(); // backup

		// forbid drag outside, set dummy mime data
		m_itemDrag->mimeData()->clear();
	}
	event->ignore();

	m_fileDrop = false;
	viewport()->update();
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

QColor NPlaylistWidget::fileDropBorderColor()
{
	return m_fileDropBorderColor;
}

void NPlaylistWidget::setFileDropBorderColor(QColor color)
{
	m_fileDropBorderColor = color;
}

QBrush NPlaylistWidget::fileDropBackground()
{
	return m_fileDropBackground;
}

void NPlaylistWidget::setFileDropBackground(QBrush brush)
{
	m_fileDropBackground = brush;
}

int NPlaylistWidget::fileDropRadius()
{
	return m_fileDropRadius;
}

void NPlaylistWidget::setFileDropRadius(int radius)
{
	m_fileDropRadius = radius;
}
// << STYLESHEET PROPERTIES

