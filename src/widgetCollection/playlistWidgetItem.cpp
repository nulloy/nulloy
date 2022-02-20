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

#include "playlistWidgetItem.h"

#include <QFileInfo>
#include <QPainter>

#include "global.h"
#include "playlistWidget.h"

NPlaylistWidgetItem::NPlaylistWidgetItem(QListWidget *parent) : QListWidgetItem(parent) {}

NPlaylistWidgetItem::NPlaylistWidgetItem(const QFileInfo &fileinfo, QListWidget *parent)
    : QListWidgetItem(parent)
{
    m_data.path = fileinfo.filePath();
}

NPlaylistWidgetItem::NPlaylistWidgetItem(const NPlaylistDataItem &dataItem, QListWidget *parent)
    : QListWidgetItem(parent)
{
    m_data = dataItem;
    setText(m_data.title);
}

QVariant NPlaylistWidgetItem::data(int role) const
{
    switch (role) {
        case (N::FailedRole):
            return m_data.failed;
        case (N::PathRole):
            return m_data.path;
        case (N::DurationRole):
            return m_data.duration;
        case (N::CountRole):
            return m_data.playbackCount;
        case (N::PositionRole):
            return m_data.playbackPosition;
        case (N::TitleFormatRole):
            return m_data.titleFormat;
        default:
            return QListWidgetItem::data(role);
    }
}

void NPlaylistWidgetItem::setData(int role, const QVariant &value)
{
    switch (role) {
        case (N::FailedRole):
            m_data.failed = value.toBool();
            break;
        case (N::PathRole):
            m_data.path = value.toString();
            break;
        case (N::DurationRole):
            m_data.duration = value.toInt();
            break;
        case (N::CountRole):
            m_data.playbackCount = value.toInt();
            break;
        case (N::PositionRole):
            m_data.playbackPosition = value.toFloat();
            break;
        case (N::TitleFormatRole):
            m_data.titleFormat = value.toString();
            break;
        default:
            QListWidgetItem::setData(role, value);
            break;
    }
}

NPlaylistDataItem NPlaylistWidgetItem::dataItem() const
{
    return m_data;
}

void NPlaylistWidgetItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    const NPlaylistWidget *playlistWidget = qobject_cast<const NPlaylistWidget *>(opt.widget);

    if (index == playlistWidget->currentIndex()) { // if currently playing item
        QColor color = playlistWidget->currentTextColor();
        if (color.isValid()) {
            opt.palette.setColor(QPalette::HighlightedText, color);
            opt.palette.setColor(QPalette::Text, color);
        }
    } else if (index.data(N::FailedRole).toBool()) { // else if a failed one
        QColor color = playlistWidget->failedTextColor();
        if (color.isValid()) {
            opt.palette.setColor(QPalette::HighlightedText, color);
            opt.palette.setColor(QPalette::Text, color);
        }
    }

    QStyledItemDelegate::paint(painter, opt, index);
}
