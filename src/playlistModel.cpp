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

#include <QRandomGenerator>

unsigned int NPlaylistModel::m_nextId = 1000; // IDs below 1000 are reserved, 0 means invalid ID

NPlaylistModel::NPlaylistModel(QObject *parent) : QAbstractListModel(parent) {}

int NPlaylistModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_data.size();
}

size_t NPlaylistModel::size() const
{
    return m_data.size();
}

int NPlaylistModel::count() const
{
    return m_data.size();
}

QHash<int, QByteArray> NPlaylistModel::roleNames() const
{
    // role names are used in QML only, so not all roles are named
    QHash<int, QByteArray> roles;
    roles[TextRole] = "text";
    roles[FilePathRole] = "filePath";
    roles[DurationRole] = "duration";
    roles[IsFailedRole] = "isFailed";
    roles[IsPlayingtRole] = "isPlaying";
    roles[IsFocusedRole] = "isFocused";
    roles[IsSelectedRole] = "isSelected";
    roles[IsHoveredRole] = "isHovered";
    roles[TrackIndexRole] = "trackIndex";
    return roles;
}

QVariantMap NPlaylistModel::get(int row) const
{
    const QHash<int, QByteArray> names = roleNames();
    QHashIterator<int, QByteArray> iter(names);
    QVariantMap res;
    while (iter.hasNext()) {
        iter.next();
        QModelIndex idx = index(row, 0);
        QVariant data = idx.data(iter.key());
        res[iter.value()] = data;
    }
    return res;
}

QVariant NPlaylistModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    const DataItem &item = m_data.at(index.row());

    switch (role) {
        case TextRole:
            return QString(item.text).replace("%i", QString::number(index.row() + 1));
        case TextFormatRole:
            return item.textFormat;
        case FilePathRole:
            return item.filePath;
        case IsFailedRole:
            return item.isFailed;
        case IsSelectedRole:
            return item.isSelected;
        case IsPlayingtRole:
            return index == m_playingIndex;
        case IsFocusedRole:
            return index == m_focusedIndex;
        case IsHoveredRole:
            return index == m_hoveredIndex;
        case DurationRole:
            return item.durationSec;
        case PlaybackPositionRole:
            return item.playbackPosition;
        case PlaybackCountRole:
            return item.playbackCount;
        case TrackIndexRole:
            return index.row() + 1;
        case IdRole:
            return item.id;
        default:
            return QVariant();
    }
}

QVariant NPlaylistModel::data(size_t row, Role role) const
{
    return data(createIndex(row, 0), role);
}

QList<size_t> NPlaylistModel::rows(Role role, const QVariant &value) const
{
    QList<size_t> rows;
    const size_t size_ = size();
    for (size_t row = 0; row < size_; ++row) {
        if (data(row, role) == value) {
            rows << row;
        }
    }
    return rows;
}

bool NPlaylistModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() >= rowCount()) {
        return false;
    }

    switch (role) {
        case IsPlayingtRole: {
            const QModelIndex oldIndex = m_playingIndex;
            m_playingIndex = QModelIndex();
            emit dataChanged(oldIndex, oldIndex, {role});
            if (value.toBool()) {
                m_playingIndex = index;
            }
            emit playingRowChanged();
            break;
        }
        case IsFocusedRole: {
            const QModelIndex oldIndex = m_focusedIndex;
            m_focusedIndex = QModelIndex();
            emit dataChanged(oldIndex, oldIndex, {role});
            if (value.toBool()) {
                m_focusedIndex = index;
            }
            emit focusedRowChanged();
            break;
        }
        case IsHoveredRole: {
            const QModelIndex oldIndex = m_hoveredIndex;
            m_hoveredIndex = QModelIndex();
            emit dataChanged(oldIndex, oldIndex, {role});
            if (value.toBool()) {
                m_hoveredIndex = index;
            }
            break;
        }
        default:
            int row = index.row();
            DataItem &item = m_data[row];
            switch (role) {
                case TextRole:
                    item.text = value.toString();
                    break;
                case TextFormatRole:
                    item.textFormat = value.toString();
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
                case DurationRole:
                    item.durationSec = value.toInt();
                    break;
                case PlaybackPositionRole:
                    item.playbackPosition = value.toReal();
                    break;
                case PlaybackCountRole:
                    item.playbackCount = value.toInt();
                    break;
                default: // anything else is read-only
                    return false;
            }
    }

    emit dataChanged(index, index, {role});
    return true;
}

bool NPlaylistModel::setData(ssize_t row, Role role, const QVariant &value)
{
    const QModelIndex index = (row == -1) ? QModelIndex() : createIndex(row, 0);
    return setData(index, value, role);
}

bool NPlaylistModel::setData(size_t startRow, size_t endRow, Role role, const QVariant &value)
{
    if (size() == 0) {
        return false;
    }

    if (endRow >= size() || startRow > endRow || endRow >= size()) {
        return false;
    }

    for (size_t row = startRow; row <= endRow; ++row) {
        if (!setData(row, role, value)) {
            return false;
        }
    }

    return true;
}

bool NPlaylistModel::setData(Role role, const QVariant &value)
{
    if (size() == 0) {
        return false;
    }

    return setData(0, size() - 1, role, value);
}

Qt::ItemFlags NPlaylistModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return QAbstractListModel::flags(index);
}

bool NPlaylistModel::insert(size_t beforeRow, const QList<DataItem> &items)
{
    if (beforeRow > size()) {
        return false;
    }

    beginInsertRows(QModelIndex(), beforeRow, beforeRow + items.size() - 1);
    for (const DataItem &item : items) {
        m_data.insert(beforeRow, item);
        m_idToRowMap.insert(item.id, beforeRow);
        ++beforeRow;
    }
    endInsertRows();

    const QModelIndex firstAffectedIndex = createIndex(beforeRow, 0);
    emit dataChanged(firstAffectedIndex, createIndex(size() - 1, 0), {TextRole, TrackIndexRole});
    emit countChanged();

    return true;
}

bool NPlaylistModel::append(const QList<DataItem> &items)
{
    return insert(size(), items);
}

bool NPlaylistModel::move(size_t startRow, size_t count, size_t beforeRow)
{
    if (startRow + count - 1 >= size() || beforeRow > size() || count == 0) {
        return false;
    }

    if (!beginMoveRows(QModelIndex(), startRow, startRow + count - 1, QModelIndex(), beforeRow)) {
        return false;
    }

    int fromRow = startRow;
    if (beforeRow < startRow) {
        fromRow += count - 1;
    } else {
        --beforeRow;
    }

    while (count--) {
        m_data.move(fromRow, beforeRow);
    }

    endMoveRows();

    const QModelIndex firstAffectedIndex = createIndex(qMin(startRow, beforeRow), 0);
    emit dataChanged(firstAffectedIndex, createIndex(size() - 1, 0), {TextRole, TrackIndexRole});

    return true;
}

void NPlaylistModel::move(const QList<size_t> &rows, size_t beforeRow)
{
    QList<QPersistentModelIndex> indexes;
    for (size_t row : rows) {
        indexes << createIndex(row, 0);
    }

    std::sort(indexes.begin(), indexes.end());
    const size_t firstAffectedRow = qMin(beforeRow, static_cast<size_t>(indexes.first().row()));
    if (beforeRow <= indexes.first().row()) {
        std::reverse(indexes.begin(), indexes.end());
    }

    int startRow = -1;
    size_t count = 0;
    for (const auto &index : indexes) {
        if (startRow == -1) {
            startRow = index.row();
            count = 1;
        } else if (startRow + count == index.row()) {
            ++count;
        } else {
            move(startRow, count, beforeRow);
            startRow = index.row();
            count = 1;
        }
    }

    if (startRow != -1) {
        move(startRow, count, beforeRow);
    }

    const size_t size_ = size();
    for (size_t row = firstAffectedRow; row < size_; ++row) {
        m_idToRowMap[m_data[row].id] = row;
    }
}

void NPlaylistModel::remove(const QList<size_t> &rows)
{
    if (size() == 0) {
        return;
    }

    QList<QPersistentModelIndex> indexes;
    for (size_t row : rows) {
        indexes << createIndex(row, 0);
    }

    std::sort(indexes.begin(), indexes.end());
    const size_t firstAffectedRow = indexes.first().row();
    std::reverse(indexes.begin(), indexes.end());

    // TODO: optimize, delete adjacent rows in groups:
    for (const auto &index : indexes) {
        const size_t row = index.row();
        beginRemoveRows(QModelIndex(), row, row);
        m_idToRowMap.remove(m_data[row].id);
        m_data.removeAt(row);
        endRemoveRows();
    }

    const size_t size_ = size();
    for (size_t row = firstAffectedRow; row < size_; ++row) {
        m_idToRowMap[m_data[row].id] = row;
    }

    emit dataChanged(indexes.first(), createIndex(size_ - 1, 0), {TextRole, TrackIndexRole});
    emit countChanged();
}

void NPlaylistModel::clear()
{
    const size_t oldSize = size();

    beginRemoveRows(QModelIndex(), 0, size() - 1);
    m_data.clear();
    m_idToRowMap.clear();
    endRemoveRows();

    emit cleared();
    emit dataChanged(QModelIndex(), createIndex(oldSize - 1, 0), {TextRole, TrackIndexRole});
    emit countChanged();
}

int NPlaylistModel::playingRow() const
{
    if (!m_playingIndex.isValid()) {
        return -1;
    }
    return m_playingIndex.row();
}

int NPlaylistModel::focusedRow() const
{
    if (!m_focusedIndex.isValid()) {
        return -1;
    }
    return m_focusedIndex.row();
}

void NPlaylistModel::setFocusedRow(size_t row)
{
    if (row > size()) {
        return;
    }
    setData(row, NPlaylistModel::IsFocusedRole, true);
}

void NPlaylistModel::setFocusedRowRelative(int offset)
{
    if (size() == 0) {
        return;
    }

    int oldFocusedRow = focusedRow();
    if (oldFocusedRow == -1) {
        oldFocusedRow = 0;
    }

    const size_t newFocusedRow = qBound(0, oldFocusedRow + offset, static_cast<int>(size()) - 1);
    setFocusedRow(newFocusedRow);
}

QList<size_t> NPlaylistModel::selectedRows() const
{
    return rows(NPlaylistModel::IsSelectedRole, true);
}

ssize_t NPlaylistModel::getRowById(unsigned int id) const
{
    if (m_idToRowMap.contains(id)) {
        return m_idToRowMap.value(id);
    }
    return -1;
}

void NPlaylistModel::calculateDuration()
{
    size_t secondsTotal = 0;
    const size_t size_ = size();
    for (size_t i = 0; i < size_; ++i) {
        int seconds = data(i, NPlaylistModel::DurationRole).toInt();
        if (seconds > 0) {
            secondsTotal += static_cast<size_t>(seconds);
        }
    }

    emit durationChanged(secondsTotal);
}

void NPlaylistModel::shuffle()
{
    const size_t size_ = size();
    if (size_ < 2) {
        return;
    }

    const int oldPlayingId = data(m_playingIndex, NPlaylistModel::IdRole).toInt();
    QList<DataItem> items = m_data;

    clear();

    QRandomGenerator *rng = QRandomGenerator::global();
    for (size_t i = 0; i < size_; ++i) {
        size_t j = i;
        while (i == j) {
            j = rng->bounded(static_cast<quint32>(size_));
        }
        items.swapItemsAt(i, j);
    }
    append(items);

    if (oldPlayingId == 0) {
        return;
    }

    const int playingRow = getRowById(oldPlayingId);
    setData(playingRow, NPlaylistModel::IsPlayingtRole, true);
    setData(NPlaylistModel::IsSelectedRole, false);
    setData(playingRow, NPlaylistModel::IsSelectedRole, true);
    setData(playingRow, NPlaylistModel::IsFocusedRole, true);
}
