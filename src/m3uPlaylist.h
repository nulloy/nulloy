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
#ifndef N_M3U_PLAYLIST_H
#define N_M3U_PLAYLIST_H

#include <QString>

typedef struct
{
	QString title;
	QString path;
	int duration;
	float position; // -1 == failed
} NM3uItem;

namespace NM3uPlaylist
{
	QList<NM3uItem> read(const QString &file);
	void write(const QString &file, QList<NM3uItem> items);
};

#endif

/* vim: set ts=4 sw=4: */
