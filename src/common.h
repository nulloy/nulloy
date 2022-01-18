/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2018 Sergey Vlasov <sergey@vlasov.me>
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

#ifndef N_COMMON_H
#define N_COMMON_H

#include <QStringList>

#include "playlistDataItem.h"

namespace NCore
{
    void cArgs(int *argc, const char ***argv);
    QString applicationBinaryName();
    QString applicationBasenameName();
    QString defaultPlaylistPath();
    QString settingsPath();
    QString rcDir();
    QList<NPlaylistDataItem> dirListRecursive(const QString &path);
} // namespace NCore

#endif
