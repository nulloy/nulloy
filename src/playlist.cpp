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

#include "playlist.h"

void playlistWrite(const QString &playlistPath, const QStringList &files)
{
	QFile playlist(playlistPath);
	if (playlist.open(QFile::WriteOnly | QFile::Truncate)) {
		QTextStream out(&playlist);
		QDir dir = QFileInfo(playlistPath).absoluteDir();
		foreach (QString file, files) {
			QString path = dir.relativeFilePath(file);
			if (path.startsWith("../"))
				path = QFileInfo(file).absoluteFilePath();
			out << path << "\n";
		}
	playlist.close();
	}
}

QStringList playlistParse(const QString &playlistPath)
{
	QStringList files;
	QFile playlist(playlistPath);
	if (playlist.exists() && playlist.open(QFile::ReadOnly)) {
		QTextStream in(&playlist);
		while(!in.atEnd())
			files << in.readLine();
		playlist.close();
	}
	return files;
}

/* vim: set ts=4 sw=4: */
