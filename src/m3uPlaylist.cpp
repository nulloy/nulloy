/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2013 Sergey Vlasov <sergey@vlasov.me>
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

#include "m3uPlaylist.h"

#include <QStringList>
#include <QFileInfo>
#include <QTextStream>

QList<NM3uItem> NM3uPlaylist::read(const QString &file)
{
	QList<NM3uItem> m3uItems;
	QString nulloyPrefix = "#NULLOY:";
	QString extinfPrefix = "#EXTINF:";

	QString line;
	QFile playlist(file);
	if (!playlist.exists() || !playlist.open(QFile::ReadOnly))
		return m3uItems;

	NM3uItem m3uItem = {"", "", 0, 0};
	QTextStream in(&playlist);
	while (!in.atEnd()) {
		line = in.readLine();
		if (line.startsWith("#")) {
			if (line.startsWith(nulloyPrefix)) {
				line.remove(0, nulloyPrefix.size());
				m3uItem.position = line.split(",").at(0).toFloat();
			} else if (line.startsWith(extinfPrefix)) {
				line.remove(0, extinfPrefix.size());
				QStringList split = line.split(",");
				m3uItem.duration = split.at(0).toInt();
				m3uItem.title = split.at(1);
			}
		} else {
			m3uItem.path = line;
			m3uItems << m3uItem;
			NM3uItem item = {"", "", 0, 0};
			m3uItem = item;
		}
	}

	playlist.close();
	return m3uItems;
}

void NM3uPlaylist::write(const QString &file, QList<NM3uItem> items)
{
	QFile playlist(file);
	if (!playlist.open(QFile::WriteOnly | QFile::Truncate))
		return;

	QTextStream out(&playlist);
	out << "#EXTM3U\n";

	for (int i = 0; i < items.count(); ++i) {
		QString itemPath = QFileInfo(items.at(i).path).canonicalFilePath();

		if (items.at(i).position != 0)
			out << "#NULLOY:" << items.at(i).position << "\n";
		if (items.at(i).position != -1 && !QFileInfo(itemPath).exists()) // keep old item, but mark failed
			out << "#NULLOY:" << -1 << "\n";

		out << "#EXTINF:" << items.at(i).duration << "," << items.at(i).title << "\n";
		out << itemPath << "\n";
	}

	playlist.close();
}

/* vim: set ts=4 sw=4: */
