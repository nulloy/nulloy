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

#ifndef N_PLAYLIST_MODEL_H
#define N_PLAYLIST_MODEL_H

#include <QAbstractListModel>
#include <QHash>
#include <QList>
#include <QVariant>

class NPlaylistModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

    static unsigned int m_nextId;

public:
    struct DataItem
    {
        QString text{};               // TextRole
        QString textFormat{};         // TextFormatRole
        QString filePath{};           // FilePathRole
        int durationSec{-1};          // DurationRole, -1 if not estimated yet
        bool isSelected{};            // IsSelectedRole
        bool isFailed{};              // IsFailedRole
        qreal playbackPosition{};     // PlaybackPositionRole
        unsigned int playbackCount{}; // PlaybackCountRole
        unsigned int id{m_nextId++};  // IdRole
    };

    enum Role
    {
        TextRole = Qt::UserRole + 1,
        TextFormatRole,
        FilePathRole,
        DurationRole,
        TrackIndexRole, // read-only
        IsSelectedRole,
        IsFailedRole,
        IsPlayingtRole,
        IsFocusedRole,
        IsHoveredRole,
        PlaybackPositionRole,
        PlaybackCountRole,
        IdRole, // read-only
    };

    NPlaylistModel(QObject *parent = nullptr);

    // QAbstractListModel overrides:
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // to partially match QML ListView API:
    Q_INVOKABLE QVariantMap get(int row) const;
    int count() const;

    // low-level operations:
    QVariant data(size_t row, Role role) const;
    QList<size_t> rows(Role role, const QVariant &value) const;
    bool setData(ssize_t row, Role role, const QVariant &value);
    bool setData(size_t startRow, size_t endRow, Role role, const QVariant &value);
    bool setData(Role role, const QVariant &value);
    bool insert(size_t beforeRow, const QList<DataItem> &items);
    bool append(const QList<DataItem> &items);
    void move(const QList<size_t> &rows, size_t beforeRow);
    void remove(const QList<size_t> &rows);
    Q_INVOKABLE void clear();

    // convenience methods:
    size_t size() const;
    Q_INVOKABLE QList<size_t> selectedRows() const;
    Q_INVOKABLE int playingRow() const;
    Q_INVOKABLE int focusedRow() const;
    void setFocusedRow(size_t row);
    void setFocusedRowRelative(int offset);
    ssize_t getRowById(unsigned int id) const;

    void calculateDuration();
    void shuffle();

private:
    QList<DataItem> m_data;
    QHash<unsigned int, size_t> m_idToRowMap; // <id, row>
    QPersistentModelIndex m_playingIndex;
    QPersistentModelIndex m_focusedIndex;
    QPersistentModelIndex m_hoveredIndex;

    bool move(size_t startRow, size_t count, size_t beforeRow);

signals:
    void durationChanged(size_t seconds);
    void countChanged();
    void playingRowChanged();
    void focusedRowChanged();
    void cleared();
};

#endif
