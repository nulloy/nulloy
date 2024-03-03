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

#ifndef N_UTILS_H
#define N_UTILS_H

#include <QObject>

#include "playlistDataItem.h"
#include "playlistModel.h"

class NUtils : public QObject
{
    Q_OBJECT

public:
    explicit NUtils(QObject *parent = nullptr);
    static QList<NPlaylistDataItem> dirListRecursive(const QString &path);
    static NPlaylistModel::DataItem toModelItem(const NPlaylistDataItem &dataItem);
    static QList<NPlaylistModel::DataItem> processPathsRecursive(const QStringList &path);
    Q_INVOKABLE static QString readFile(const QString &path);
    Q_INVOKABLE static QString pathToUri(const QString &path);
};

#endif
