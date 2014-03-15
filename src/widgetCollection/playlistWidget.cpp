/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2014 Sergey Vlasov <sergey@vlasov.me>
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
#include <QMessageBox>
#include <QShortcut>
#include <QUrl>

NPlaylistWidget::NPlaylistWidget(QWidget *parent) : QListWidget(parent)
{
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

	m_drag = NULL;

	m_repeatMode = NSettings::instance()->value("Repeat").toBool();
	m_shuffleMode = FALSE;
	m_currentShuffledIndex = 0;
}

void NPlaylistWidget::contextMenuEvent(QContextMenuEvent *event)
{
	if (selectedItems().size() != 0)
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

		QListWidgetItem *takenItem = takeItem(row(item));
		delete takenItem;
	}
	
	viewport()->update();
}

void NPlaylistWidget::on_removeAction_triggered()
{
	foreach (QListWidgetItem *item, selectedItems()) {
		QListWidgetItem *takenItem = takeItem(row(item));
		delete takenItem;
	}
	viewport()->update();
}

void NPlaylistWidget::on_revealAction_triggered()
{
	if (!NCore::revealInFileManager(selectedItems().first()->data(N::PathRole).toString()))
		QMessageBox::warning(this, "File Manager Error", "File doesn't exist: " + selectedItems().first()->text());
}

NPlaylistWidget::~NPlaylistWidget() {}

void NPlaylistWidget::setCurrentItem(NPlaylistWidgetItem *item)
{
	QString file = item->data(N::PathRole).toString();
	// check if it's a playlist file:
	QList<NPlaylistDataItem> dataItemsList = NPlaylistStorage::readPlaylist(file);
	if (dataItemsList.count() > 1) {
		int index = row(item);
		int index_bkp = index;

		QListWidgetItem *takenItem = takeItem(row(item));
		delete takenItem;

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

	item->setData(N::FailedRole, FALSE); // reset failed role

	// setting currently playing font to bold, colors set in delegate
	QFont f = item->font();
	if (m_currentItem) {
		// reset old item to defaults
		f.setBold(FALSE);
		m_currentItem->setFont(f);

		m_currentItem->setData(N::PositionRole, m_playbackEngine->position());
		m_currentItem->setData(N::CountRole, m_currentItem->data(N::CountRole).toInt() + 1);
	}
	f.setBold(TRUE);
	item->setFont(f);

	scrollToItem(item);
	m_currentItem = item;
	update();

	emit mediaSet(file);
}

void NPlaylistWidget::currentFailed()
{
	m_currentItem->setData(N::FailedRole, TRUE);
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
	setCurrentItem(dynamic_cast<NPlaylistWidgetItem *>(item));
	emit currentActivated();
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
	m_currentItem = NULL;
	foreach (QString path, files)
		addItem(new NPlaylistWidgetItem(QFileInfo(path)));
}

bool NPlaylistWidget::setPlaylist(const QString &file)
{
	clear();
	m_currentItem = NULL;

	QList<NPlaylistDataItem> dataItemsList = NPlaylistStorage::readPlaylist(file);

	if (dataItemsList.isEmpty())
		return FALSE;

	for (int i = 0; i < dataItemsList.count(); ++i)
		addItem(new NPlaylistWidgetItem(dataItemsList.at(i)));

	return TRUE;
}

void NPlaylistWidget::playFiles(const QStringList &files)
{
	setFiles(files);
	playRow(0);
}

void NPlaylistWidget::playNextRow()
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
		} else if (NSettings::instance()->value("LoadNext").toBool()) {
			QDir::SortFlag flag = (QDir::SortFlag)NSettings::instance()->value("LoadNextSort").toInt();
			QString file = m_currentItem->data(N::PathRole).toString();
			QString path = QFileInfo(file).path();
			QStringList entryList = QDir(path).entryList(QDir::Files | QDir::NoDotAndDotDot, flag);
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
		playNextRow();
}

void NPlaylistWidget::playPreviousRow()
{
	int row = currentRow();
	if (m_shuffleMode) {
		m_currentShuffledIndex--;
		if (m_currentShuffledIndex < 0)
			m_currentShuffledIndex = m_shuffledItems.count() - 1;
		activateItem(m_shuffledItems.at(m_currentShuffledIndex));
	} else {
		if (row > 0)
			activateItem(item(row - 1));
	}
}

void NPlaylistWidget::rowsInserted(const QModelIndex &parent, int start, int end)
{
	for (int i = start; i < end + 1; ++i) {
		m_shuffledItems.append(item(i));
	}
	if (m_shuffleMode)
		setShuffleMode(TRUE);
	QListWidget::rowsInserted(parent, start, end);
}

void NPlaylistWidget::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
	for (int i = start; i < end + 1; ++i) {
		if (item(i) == m_currentItem)
			m_currentItem = NULL;

		m_shuffledItems.removeAll(item(i));
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
	return dynamic_cast<NPlaylistWidgetItem *>(QListWidget::item(row));
}


// DRAG & DROP >>

bool NPlaylistWidget::dropMimeData(int index, const QMimeData *data, Qt::DropAction action)
{
	Q_UNUSED(action);
	foreach (QUrl url, data->urls()) {
		foreach (QString file, NCore::dirListRecursive(url.toLocalFile())) {
			insertItem(index, new NPlaylistWidgetItem(QFileInfo(file)));
			++index;
		}
	}

	m_drag = NULL;
	return TRUE;
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

	m_drag = new QDrag(this);
	m_drag->setMimeData(mimeData);
	// restrct to move action
	m_drag->start(Qt::MoveAction);
}

void NPlaylistWidget::dropEvent(QDropEvent *event)
{
	if (m_drag) // moving withing playlist
		event->setDropAction(Qt::MoveAction);
	else // dropping from file manager
		event->setDropAction(Qt::CopyAction);

	QListWidget::dropEvent(event);
}

void NPlaylistWidget::dragEnterEvent(QDragEnterEvent *event)
{
	if (m_drag && !m_mimeDataUrls.isEmpty())
		m_drag->mimeData()->setUrls(m_mimeDataUrls); // recover old data

	// change to move action
	event->setDropAction(Qt::MoveAction);
	QListWidget::dragEnterEvent(event);
}

void NPlaylistWidget::dragMoveEvent(QDragMoveEvent *event)
{
	// change to move action
	event->setDropAction(Qt::MoveAction);
	QListWidget::dragMoveEvent(event);
}

void NPlaylistWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
	if (m_drag) {
		m_mimeDataUrls = m_drag->mimeData()->urls(); // backup

		// forbid drag outside, set dummy mime data
		m_drag->mimeData()->clear();
	}
	event->ignore();
}

// << DRAG & DROP


// STYLESHEET PROPERTIES >>

void NPlaylistWidget::setCurrentTextColor(QColor color)
{
	m_currentTextColor = color;
}

QColor NPlaylistWidget::getCurrentTextColor() const
{
	return m_currentTextColor;
}

void NPlaylistWidget::setFailedTextColor(QColor color)
{
	m_failedTextColor = color;
}

QColor NPlaylistWidget::getFailedTextColor() const
{
	return m_failedTextColor;
}

// << STYLESHEET PROPERTIES

