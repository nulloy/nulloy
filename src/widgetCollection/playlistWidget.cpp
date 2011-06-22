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

#include "dirProcessor.h"
#include <QFileInfo>
#include <QtGui>

#include <QDebug>

NPlaylistWidget::NPlaylistWidget(QWidget *parent) : QListWidget(parent)
{
	connect(this, SIGNAL(itemActivated(QListWidgetItem *)), this, SLOT(on_itemActivated(QListWidgetItem *)));

	m_currentItem = NULL;
}

NPlaylistWidget::~NPlaylistWidget() {}

void NPlaylistWidget::setCurrentItem(QListWidgetItem *item)
{
	QBrush b = item->foreground();
	b.setColor(QPalette::Text);
	item->setForeground(b);

	QFont f = item->font();

	if (m_currentItem) {
		f.setBold(FALSE);
		m_currentItem->setFont(f);
	}

	f.setBold(TRUE);
	item->setFont(f);

	scrollToItem(item);
	m_currentItem = item;

	emit mediaSet(item->data(Qt::UserRole).toString());
}

void NPlaylistWidget::setCurrentFailed()
{
	QBrush b = m_currentItem->foreground();
	b.setColor(QPalette().color(QPalette::Dark));
	m_currentItem->setForeground(b);
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

void NPlaylistWidget::activateItem(QListWidgetItem *item)
{
	emit itemActivated(item);
}

void NPlaylistWidget::on_itemActivated(QListWidgetItem *item)
{
	if (m_currentItem != item)
		setCurrentItem(item);
	emit currentActivated();
}

QStringList NPlaylistWidget::mediaList()
{
	QStringList list;
	for(int i = 0; i < count(); ++i)
		list << item(i)->data(Qt::UserRole).toString();

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

void NPlaylistWidget::activateNext()
{
	int row = currentRow();
	if (row < count() - 1)
		activateItem(item(row + 1));
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
	activateItem(m_currentItem);
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

QListWidgetItem* NPlaylistWidget::createItemFromPath(const QString &path)
{
	QListWidgetItem *item = new QListWidgetItem();
	item->setData(Qt::UserRole, path);
	item->setText(QFileInfo(path).fileName());
	return item;
}

bool NPlaylistWidget::dropMimeData(int index, const QMimeData *data, Qt::DropAction action)
{
	Q_UNUSED(action);
	foreach (QUrl url, data->urls()) {
		foreach (QString file, dirListRecursive(url.toLocalFile())) {
			QListWidgetItem *item = createItemFromPath(file);
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

QMimeData* NPlaylistWidget::mimeData(const QList<QListWidgetItem *> items) const
{
	QList<QUrl> urls;

	foreach (QListWidgetItem *item, items)
		urls << QUrl::fromLocalFile(item->data(Qt::UserRole).toString());

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

/* vim: set ts=4 sw=4: */
