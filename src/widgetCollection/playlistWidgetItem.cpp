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

void NPlaylistWidgetItem::setText(const QString &text)
{
    m_data.title = text;
    QListWidgetItem::setText(text);
}

QVariant NPlaylistWidgetItem::data(int role) const
{
    switch (role) {
        case (N::PlayingRole):
            return m_data.playing;
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
        case (N::TrackIndexRole):
            return m_data.trackIndex;
        case (Qt::DisplayRole):
            return QVariant(
                QListWidgetItem::data(Qt::DisplayRole)
                    .toString()
                    .replace("%i", QString::number(data(N::TrackIndexRole).toInt() + 1)));
        case (Qt::FontRole): {
            QFont font = qvariant_cast<QFont>(QListWidgetItem::data(Qt::FontRole));
            font.setBold(m_data.playing);
            return font;
        }
        case (N::IdRole):
            return m_data.id;

        default:
            return QListWidgetItem::data(role);
    }
}

void NPlaylistWidgetItem::setData(int role, const QVariant &value)
{
    switch (role) {
        case (N::PlayingRole):
            m_data.playing = value.toBool();
            break;
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
        case (N::TrackIndexRole):
            m_data.trackIndex = value.toInt();
            break;
        case (N::IdRole):
            m_data.id = value.toInt();
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

    if (index.data(N::FailedRole).toBool()) { // FailedRole has higher priority than PlayingRole
        QColor color = playlistWidget->failedTextColor();
        if (color.isValid()) {
            opt.palette.setColor(QPalette::HighlightedText, color);
            opt.palette.setColor(QPalette::Text, color);
        }
    } else if (index.data(N::PlayingRole).toBool()) {
        QColor color = playlistWidget->playingTextColor();
        if (color.isValid()) {
            opt.palette.setColor(QPalette::HighlightedText, color);
            opt.palette.setColor(QPalette::Text, color);
        }
    }

    // bold font set via Qt::FontRole

    QStyledItemDelegate::paint(painter, opt, index);
}
