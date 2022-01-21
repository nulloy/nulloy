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

#ifndef N_PLAYLIST_STORAGE_H
#define N_PLAYLIST_STORAGE_H

#include <QList>
#include <QString>

#include "global.h"
#include "playlistDataItem.h"

namespace NPlaylistStorage
{
    QList<NPlaylistDataItem> readM3u(const QString &file);
    void writeM3u(const QString &file, QList<NPlaylistDataItem> items, N::M3uExtention ext);
}; // namespace NPlaylistStorage

#endif
