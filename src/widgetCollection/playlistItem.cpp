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

#include "playlistItem.h"

#include <QPainter>
#include <QDebug>

NPlaylistItem::NPlaylistItem(QListWidget *parent) : QListWidgetItem(parent)
{
	m_failed = FALSE;
	m_duration = -1;
}

QVariant NPlaylistItem::data(int role) const
{
	switch (role) {
		case (FailedRole):
			return m_failed;
		case (PathRole):
			return m_path;
		case (DurationRole):
			return m_duration;
		default:
			return QListWidgetItem::data(role);
	}
}

void NPlaylistItem::setData(int role, const QVariant &value)
{
	switch (role) {
		case (FailedRole):
			m_failed = value.toBool();
			break;
		case (PathRole):
			m_path = value.toString();
			break;
		case (DurationRole):
			m_duration = value.toInt();
			break;
		default:
			QListWidgetItem::setData(role, value);
			break;
	}
}

void NPlaylistItemDelegate::paint(QPainter *painter,
									const QStyleOptionViewItem &option,
									const QModelIndex &index) const
{
	QStyleOptionViewItem opt = option;

	if (index.data(NPlaylistItem::FailedRole).toBool()) {
		QColor dark = option.palette.dark().color();
		opt.palette.setColor(QPalette::HighlightedText, dark);
		opt.palette.setColor(QPalette::Text, dark);
	}

	QItemDelegate::paint(painter, opt, index);
}

/* vim: set ts=4 sw=4: */
