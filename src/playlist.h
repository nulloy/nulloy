/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2011 Sergey Vlasov <sergey@vlasov.me>
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

#ifndef N_PLAY_LIST_H
#define N_PLAY_LIST_H

#include <QtCore>

void playlistWrite(const QString &playlistPath, const QStringList &files);
QStringList playlistParse(const QString &playlistPath);

#endif

/* vim: set ts=4 sw=4: */
