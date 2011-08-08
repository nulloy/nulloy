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
#include <QFileInfo>
#include <QtGui>

#include <QDebug>

NPlaylistWidget::NPlaylistWidget(QWidget *parent) : QListWidget(parent)
{
	connect(this, SIGNAL(itemActivated(QListWidgetItem *)), this, SLOT(on_itemActivated(QListWidgetItem *)));
	setItemDelegate(new NPlaylistItemDelegate);
	m_currentItem = NULL;
}

NPlaylistWidget::~NPlaylistWidget() {}

void NPlaylistWidget::setCurrentItem(NPlaylistItem *item)
{
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

	emit mediaSet(item->data(NPlaylistItem::PathRole).toString());
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

void NPlaylistWidget::setCurrentRow(int row)
{
	if (row > -1)
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
	for(int i = 0; i < count(); ++i)
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

void NPlaylistWidget::setMediaListFromPlaylist(const QString &path)
{
	clear();
	m_currentItem = NULL;
	bool failed = FALSE;

	QString prefix = "#NULLOY:";

	QString line;
	QFile playlist(path);
	if (playlist.exists() && playlist.open(QFile::ReadOnly)) {
		QTextStream in(&playlist);
		while(!in.atEnd()) {
			line = in.readLine();
			if (line.startsWith("#")) {
				if (line.startsWith(prefix)) {
					line.remove(0, prefix.size());
					float time = line.section(',', 0).toFloat();
					if (time == -1)
						failed = TRUE;
				}
			} else {
				NPlaylistItem *item = createItemFromPath(line);
				item->setData(NPlaylistItem::FailedRole, failed);
				addItem(item);

				failed = FALSE;
			}
		}
		playlist.close();
	}
}

void NPlaylistWidget::writePlaylist(const QString &file)
{
	QFile playlist(file);
	if (playlist.open(QFile::WriteOnly | QFile::Truncate)) {
		QTextStream out(&playlist);
		out << "#EXTM3U\n";

		QDir dir(QFileInfo(file).canonicalPath());

		for(int i = 0; i < count(); ++i) {
			float time = 0;
			if (item(i)->data(NPlaylistItem::FailedRole).toBool())
				time = -1;
			if (time != 0)
				out << "#NULLOY:" << time << "\n";
			QString itemPath = item(i)->data(NPlaylistItem::PathRole).toString();
			QString file = dir.relativeFilePath(QFileInfo(itemPath).canonicalFilePath());
			out << "#EXTINF:-1, " << QFileInfo(file).fileName() << "\n";
			out << file << "\n";
		}
		playlist.close();
	}
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

NPlaylistItem* NPlaylistWidget::createItemFromPath(const QString &path)
{
	NPlaylistItem *item = new NPlaylistItem();
	item->setData(NPlaylistItem::PathRole, path);
	item->setText(QFileInfo(path).fileName());
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

QMimeData* NPlaylistWidget::mimeData(const QList<NPlaylistItem *> items) const
{
	QList<QUrl> urls;

	foreach (NPlaylistItem *item, items)
		urls << QUrl::fromLocalFile(item->data(NPlaylistItem::PathRole).toString());

	QMimeData *data = new QMimeData();
	data->setUrls(urls);

	return data;
}

void NPlaylistWidget::keyPressEvent(QKeyEvent *e)
{
	int keyInt = e->key();
	if (keyInt == Qt::Key_Delete && e->modifiers() == Qt::NoModifier) {
		foreach (QListWidgetItem *item, selectedItems()) {
			QListWidgetItem *takenItem = takeItem(row(item));
			delete takenItem;
		}
		update();
	}
}

NPlaylistItem* NPlaylistWidget::item(int row)
{
	return dynamic_cast<NPlaylistItem *>(QListWidget::item(row));
}

/* vim: set ts=4 sw=4: */
