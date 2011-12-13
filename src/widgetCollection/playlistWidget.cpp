/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2011 Sergey Vlasov <sergey@vlasov.me>
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
#include "core.h"
#include "trash.h"
#include "tagReader.h"

#include <QtGui>

#include <QDebug>

NPlaylistWidget::NPlaylistWidget(QWidget *parent) : QListWidget(parent)
{
	connect(this, SIGNAL(itemActivated(QListWidgetItem *)), this, SLOT(on_itemActivated(QListWidgetItem *)));
	setItemDelegate(new NPlaylistItemDelegate(this));
	m_currentItem = NULL;

	QShortcut *removeShortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
	connect(removeShortcut, SIGNAL(activated()), this, SLOT(removeFromPlaylist()));
	QAction *removeAction = new QAction(QIcon::fromTheme("edit-clear",
												style()->standardIcon(QStyle::SP_DialogResetButton)),
												tr("Remove From Playlist"), this);
	removeAction->setShortcut(removeShortcut->key());
	connect(removeAction, SIGNAL(triggered()), this, SLOT(removeFromPlaylist()));

	QShortcut *trashShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Delete), this);
	connect(trashShortcut, SIGNAL(activated()), this, SLOT(moveToTrash()));
	QAction *trashAction = new QAction(QIcon::fromTheme("trashcan_empty",
										style()->standardIcon(QStyle::SP_TrashIcon)),
										tr("Move To Trash"), this);
	trashAction->setShortcut(trashShortcut->key());
	connect(trashAction, SIGNAL(triggered()), this, SLOT(moveToTrash()));

	m_contextMenu = new QMenu(this);
	m_contextMenu->addAction(removeAction);
	m_contextMenu->addAction(trashAction);
}

void NPlaylistWidget::contextMenuEvent(QContextMenuEvent *event)
{
	if (selectedItems().size() != 0)
		m_contextMenu->exec(mapToGlobal(event->pos()));
	else
		QListWidget::contextMenuEvent(event);
}

void NPlaylistWidget::moveToTrash()
{
	foreach (QListWidgetItem *item, selectedItems()) {
		QString path = QFileInfo(item->data(NPlaylistItem::PathRole).toString()).canonicalFilePath();
		QString error;
		if (NTrash(path, &error) != 0) {
			QMessageBox box(QMessageBox::Warning, "File Delete Error", "", QMessageBox::Yes | QMessageBox::Cancel, this);
			box.setDefaultButton(QMessageBox::Cancel);
			box.setText("Failed to move to Trash \"" + path + "\"" + (error.isEmpty() ? "" : "\n" + error));
			box.setInformativeText("Do you want to delete permanently?");
			if (box.exec() == QMessageBox::Yes) {
				if (!QFile::remove(path)) {
					QMessageBox::critical(this, "File Delete Error", "Failed to delete \"" + path + "\"");
					return;
				}
			} else {
				return;
			}
		}

		QListWidgetItem *takenItem = takeItem(row(item));
		delete takenItem;
	}
	update();
}

void NPlaylistWidget::removeFromPlaylist()
{
	foreach (QListWidgetItem *item, selectedItems()) {
		QListWidgetItem *takenItem = takeItem(row(item));
		delete takenItem;
	}
	update();
}

NPlaylistWidget::~NPlaylistWidget() {}

void NPlaylistWidget::setCurrentItem(NPlaylistItem *item)
{
	QString file = item->data(NPlaylistItem::PathRole).toString();
	QString fileName = QFileInfo(file).fileName();
	if (fileName.endsWith(".m3u") || fileName.endsWith(".m3u8")) {
		int index = row(item);
		int index_bkp = index;
		QList<NM3uItem> m3uItems = NM3uPlaylist::read(file);

		QListWidgetItem *takenItem = takeItem(row(item));
		delete takenItem;

		foreach (NM3uItem m3uItem, m3uItems) {
			NPlaylistItem *newItem = createItemFromM3uItem(m3uItem);
			insertItem(index, newItem);
			++index;
		}

		setCurrentItem(NPlaylistWidget::item(index_bkp));
		return;
	}

	NTagReader tagReader(file);
	if (tagReader.isValid()) {
		item->setText(tagReader.toString(NSettings::instance()->value("GUI/PlaylistTitleFormat").toString()));
		item->setData(NPlaylistItem::DurationRole, tagReader.toString("%D").toInt());
	} else {
		item->setText(fileName);
	}
	item->setData(NPlaylistItem::FailedRole, FALSE);

	QFont f = item->font();
	if (m_currentItem) {
		f.setBold(FALSE);
		m_currentItem->setFont(f);
	}
	f.setBold(TRUE);
	item->setFont(f);

	scrollToItem(item);
	m_currentItem = item;

	emit mediaSet(file);
}

void NPlaylistWidget::setCurrentFailed()
{
	m_currentItem->setData(NPlaylistItem::FailedRole, TRUE);
}

int NPlaylistWidget::currentRow()
{
	if (m_currentItem)
		return row(m_currentItem);
	else
		return -1;
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

void NPlaylistWidget::activateRow(int row)
{
	if (row > -1)
		activateItem(item(row));
}

void NPlaylistWidget::activateItem(NPlaylistItem *item)
{
	emit itemActivated(item);
}

void NPlaylistWidget::on_itemActivated(QListWidgetItem *item)
{
	NPlaylistItem *item2 = dynamic_cast<NPlaylistItem *>(item);
	if (m_currentItem != item2)
		setCurrentItem(item2);
	emit currentActivated();
}

QStringList NPlaylistWidget::mediaList()
{
	QStringList list;
	for (int i = 0; i < count(); ++i)
		list << item(i)->data(NPlaylistItem::PathRole).toString();

	return list;
}

void NPlaylistWidget::appendMediaList(const QStringList &pathList)
{
	foreach (QString path, pathList)
		addItem(createItemFromPath(path));
}

void NPlaylistWidget::setMediaList(const QStringList &pathList)
{
	clear();
	m_currentItem = NULL;
	foreach (QString path, pathList)
		addItem(createItemFromPath(path));
}

void NPlaylistWidget::setMediaListFromPlaylist(const QString &file)
{
	clear();
	m_currentItem = NULL;

	QList<NM3uItem> m3uItems = NM3uPlaylist::read(file);
	for (int i = 0; i < m3uItems.count(); ++i) {
		addItem(createItemFromM3uItem(m3uItems.at(i)));
	}
}

void NPlaylistWidget::writePlaylist(const QString &file)
{
	QList<NM3uItem> m3uItems;
	for (int i = 0; i < count(); ++i) {
		NM3uItem m3uItem = {"", "", 0, 0};

		m3uItem.path =  item(i)->data(NPlaylistItem::PathRole).toString();
		m3uItem.title = item(i)->text();
		m3uItem.duration = item(i)->data(NPlaylistItem::DurationRole).toInt();

		if (!item(i)->data(NPlaylistItem::FailedRole).toBool())
			m3uItem.position = 0;
		else
			m3uItem.position = -1;

		m3uItems << m3uItem;
	}
	NM3uPlaylist::write(file, m3uItems);
}

void NPlaylistWidget::activateNext()
{
	int row = currentRow();
	if (row < count() - 1) {
		activateItem(item(row + 1));
	} else if (NSettings::instance()->value("LoadNext").toBool()) {
		QDir::SortFlag flag = (QDir::SortFlag)NSettings::instance()->value("LoadNextSort").toInt();
		QString file = m_currentItem->data(NPlaylistItem::PathRole).toString();
		QString path = QFileInfo(file).path();
		QStringList entryList = QDir(path).entryList(QDir::Files | QDir::NoDotAndDotDot, flag);
		int index = entryList.indexOf(QFileInfo(file).fileName());
		if (index != -1 && entryList.size() > index + 1) {
			addItem(createItemFromPath(path + "/" + entryList.at(index + 1)));
			activateItem(item(row + 1));
		}
	}
}

void NPlaylistWidget::activatePrev()
{
	int row = currentRow();
	if (row > 0)
		activateItem(item(row - 1));
}

void NPlaylistWidget::activateFirst()
{
	activateItem(item(0));
}

void NPlaylistWidget::activateCurrent()
{
	if (m_currentItem)
		activateItem(m_currentItem);
	else activateFirst();
}

void NPlaylistWidget::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
	Q_UNUSED(parent);
	for (int i = start; i < end + 1; ++i) {
		if (item(i) == m_currentItem) {
			m_currentItem = NULL;
			break;
		}
	}
}

NPlaylistItem* NPlaylistWidget::createItemFromPath(const QString &file)
{
	NPlaylistItem *item = new NPlaylistItem();
	item->setData(NPlaylistItem::PathRole, file);
	item->setText(QFileInfo(file).fileName());
	return item;
}

NPlaylistItem* NPlaylistWidget::createItemFromM3uItem(NM3uItem m3uItem)
{
	NPlaylistItem *item = new NPlaylistItem();
	item->setData(NPlaylistItem::PathRole, m3uItem.path);
	item->setText(m3uItem.title);
	if (m3uItem.position == -1)
		item->setData(NPlaylistItem::FailedRole, TRUE);
	return item;
}

bool NPlaylistWidget::dropMimeData(int index, const QMimeData *data, Qt::DropAction action)
{
	Q_UNUSED(action);
	foreach (QUrl url, data->urls()) {
		foreach (QString file, NCore::dirListRecursive(url.toLocalFile())) {
			NPlaylistItem *item = createItemFromPath(file);
			insertItem(index, item);
			++index;
		}
	}

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

QMimeData* NPlaylistWidget::mimeData(const QList<NPlaylistItem *> items) const
{
	QList<QUrl> urls;

	foreach (NPlaylistItem *item, items)
		urls << QUrl::fromLocalFile(item->data(NPlaylistItem::PathRole).toString());

	QMimeData *data = new QMimeData();
	data->setUrls(urls);

	return data;
}

NPlaylistItem* NPlaylistWidget::item(int row)
{
	return dynamic_cast<NPlaylistItem *>(QListWidget::item(row));
}

/* vim: set ts=4 sw=4: */
