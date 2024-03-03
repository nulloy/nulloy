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
#include <QList>
#include <QVariant>

class NPlaylistModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow)
    Q_PROPERTY(int focusedRow READ focusedRow WRITE setFocusedRow)

public:
    struct DataItem
    {
        QString text;
        unsigned int trackInfoFormatId;
        QString filePath;
        bool isFailed;
        bool isSelected;
        unsigned int durationSec;
        qreal lastPosition;
        unsigned int playbackCount;
        unsigned int id;
    };

    enum Role
    {
        DisplayRole = Qt::UserRole + 1,
        TrackInfoFormatIdRole,
        FilePathRole,
        DurationRole,
        LastPositionRole,
        PlaybackCountRole,
        IsFailedRole,
        IsCurrentRole,
        IsFocusedRole,
        IsHoveredRole,
        IsSelectedRole,
        RowRole,
        IdRole,
    };

    NPlaylistModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant data(int row, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    bool setData(int row, const QVariant &value, int role);
    bool setDataRange(int startRow, int endRow, const QVariant &value, int role);
    bool setDataAll(const QVariant &value, int role);
    bool insertData(const DataItem &data, int row);
    bool appendData(const DataItem &data);
    void clearData();

    int currentRow() const;
    void setCurrentRow(int row);

    int focusedRow() const;
    void setFocusedRow(int row);

private:
    QList<DataItem> m_data;
    int m_currentRow{-1};
    int m_focusedRow{-1};
    int m_hoveredRow{-1};
};

#endif
