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

#ifndef N_PLAYLIST_ITEM_H
#define N_PLAYLIST_ITEM_H

#include <QListWidgetItem>

#include "playlistDataItem.h"

class QFileInfo;

class NPlaylistWidgetItem : public QListWidgetItem
{
private:
    NPlaylistDataItem m_data;

public:
    NPlaylistWidgetItem(QListWidget *parent = 0);
    NPlaylistWidgetItem(const QFileInfo &fileinfo, QListWidget *parent = 0);
    NPlaylistWidgetItem(const NPlaylistDataItem &dataItem, QListWidget *parent = 0);

    QVariant data(int role) const;
    void setData(int role, const QVariant &value);

    NPlaylistDataItem dataItem() const;
};

#include <QStyledItemDelegate>

class NPlaylistWidgetItemDelegate : public QStyledItemDelegate
{
public:
    NPlaylistWidgetItemDelegate(QWidget *parent = 0) : QStyledItemDelegate(parent) {}
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
};

#endif
