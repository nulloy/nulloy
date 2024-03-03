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

#include "playlistModel.h"

NPlaylistModel::NPlaylistModel(QObject *parent) : QAbstractListModel(parent) {}

int NPlaylistModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_data.size();
}

QHash<int, QByteArray> NPlaylistModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[DisplayRole] = "text";
    roles[FilePathRole] = "filePath";
    roles[DurationRole] = "duration";
    roles[IsFailedRole] = "isFailed";
    roles[IsCurrentRole] = "isCurrent";
    roles[IsFocusedRole] = "isFocused";
    roles[IsSelectedRole] = "isSelected";
    roles[IsHoveredRole] = "isHovered";
    roles[RowRole] = "row";
    return roles;
}

QVariant NPlaylistModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_data.size()) {
        return QVariant();
    }

    const DataItem &item = m_data.at(index.row());

    switch (role) {
        case DisplayRole:
            return item.text;
        case TrackInfoFormatIdRole:
            return item.trackInfoFormatId;
        case FilePathRole:
            return item.filePath;
        case IsFailedRole:
            return item.isFailed;
        case IsSelectedRole:
            return item.isSelected;
        case IsCurrentRole:
            return index.row() == m_currentRow;
        case IsFocusedRole:
            return index.row() == m_focusedRow;
        case IsHoveredRole:
            return index.row() == m_hoveredRow;
        case DurationRole:
            return item.durationSec;
        case LastPositionRole:
            return item.lastPosition;
        case PlaybackCountRole:
            return item.playbackCount;
        case RowRole:
            return index.row() + 1;
        case IdRole:
            return item.id;
        default:
            return QVariant();
    }
}

QVariant NPlaylistModel::data(int row, int role) const
{
    return data(createIndex(row, 0), role);
}

bool NPlaylistModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= m_data.size()) {
        return false;
    }

    int row = index.row();

    DataItem &item = m_data[row];

    switch (role) {
        case DisplayRole:
            item.text = value.toString();
            break;
        case TrackInfoFormatIdRole:
            item.trackInfoFormatId = value.toInt();
            break;
        case FilePathRole:
            item.filePath = value.toString();
            break;
        case IsFailedRole:
            item.isFailed = value.toBool();
            break;
        case IsSelectedRole:
            item.isSelected = value.toBool();
            break;
        case IsCurrentRole: {
            QModelIndex oldIndex = createIndex(m_currentRow, 0);
            m_currentRow = -1;
            emit dataChanged(oldIndex, oldIndex, {role});
            if (value.toBool()) {
                m_currentRow = row;
            }
            break;
        }
        case IsFocusedRole: {
            QModelIndex oldIndex = createIndex(m_focusedRow, 0);
            m_focusedRow = -1;
            emit dataChanged(oldIndex, oldIndex, {role});
            if (value.toBool()) {
                m_focusedRow = row;
            }
            break;
        }
        case IsHoveredRole: {
            QModelIndex oldIndex = createIndex(m_hoveredRow, 0);
            m_hoveredRow = -1;
            emit dataChanged(oldIndex, oldIndex, {role});
            if (value.toBool()) {
                m_hoveredRow = row;
            }
            break;
        }
        case DurationRole:
            item.durationSec = value.toInt();
            break;
        case LastPositionRole:
            item.lastPosition = value.toReal();
            break;
        case PlaybackCountRole:
            item.playbackCount = value.toInt();
            break;
        default:
            return false;
    }

    emit dataChanged(index, index, {role});

    return true;
}

bool NPlaylistModel::setData(int row, const QVariant &value, int role)
{
    return setData(createIndex(row, 0), value, role);
}

bool NPlaylistModel::setDataRange(int startRow, int endRow, const QVariant &value, int role)
{
    if (startRow < 0 || endRow >= rowCount()) {
        return false;
    }

    for (int i = startRow; i <= endRow; ++i) {
        if (!setData(i, value, role)) {
            return false;
        }
    }

    return true;
}

bool NPlaylistModel::setDataAll(const QVariant &value, int role)
{
    return setDataRange(0, rowCount() - 1, value, role);
}

Qt::ItemFlags NPlaylistModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}

bool NPlaylistModel::insertData(const DataItem &data, int row)
{
    if (row < 0 || row > m_data.size()) {
        return false;
    }

    beginInsertRows(QModelIndex(), row, row);
    m_data.insert(row, data);
    endInsertRows();

    return true;
}

bool NPlaylistModel::appendData(const DataItem &data)
{
    return insertData(data, rowCount());
}

void NPlaylistModel::clearData()
{
    if (rowCount() > 0) {
        beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
        m_data.clear();
        endRemoveRows();
    }
}

int NPlaylistModel::currentRow() const
{
    return m_currentRow;
}

void NPlaylistModel::setCurrentRow(int row)
{
    setData(row, true, IsCurrentRole);
}

int NPlaylistModel::focusedRow() const
{
    return m_focusedRow;
}

void NPlaylistModel::setFocusedRow(int row)
{
    setData(row, true, IsFocusedRole);
}
