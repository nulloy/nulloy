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

#include "playlistStorage.h"

#include <QStringList>
#include <QFileInfo>
#include <QTextStream>

QList<NPlaylistDataItem> _processDataItem(const NPlaylistDataItem &dataItem)
{
	QList<NPlaylistDataItem> dataItemsList;
	QString fileName = QFileInfo(dataItem.path).fileName();
	if (fileName.endsWith(".m3u") || fileName.endsWith(".m3u8")) {
		QList<NPlaylistDataItem> _dataItemsList = NPlaylistStorage::readM3u(dataItem.path);
		foreach (NPlaylistDataItem _dataItem, _dataItemsList)
			dataItemsList << _processDataItem(_dataItem);
	} else {
		dataItemsList << dataItem;
	}

	return dataItemsList;
}

QList<NPlaylistDataItem> NPlaylistStorage::readPlaylist(const QString &file)
{
	return _processDataItem(NPlaylistDataItem(file));
}

/*
*  Prefixed data order:
*  #NULLOY:failed,position
*  #EXTINF:durationSeconds,playlistTitle
*/

QList<NPlaylistDataItem> NPlaylistStorage::readM3u(const QString &file)
{
	QList<NPlaylistDataItem> dataItemsList;
	QString nulloyPrefix = "#NULLOY:";
	QString extinfPrefix = "#EXTINF:";

	QString line;
	QFile playlist(file);
	if (!playlist.exists() || !playlist.open(QFile::ReadOnly))
		return dataItemsList;

	NPlaylistDataItem dataItem;
	QTextStream in(&playlist);
	while (!in.atEnd()) {
		line = in.readLine();
		if (line.trimmed().isEmpty())
			continue;
		if (line.startsWith("#")) {
			if (line.startsWith(nulloyPrefix)) {
				line.remove(0, nulloyPrefix.size());

				QStringList split = line.split(",");
				if (split.count() != 2)
					continue;

				dataItem.failed    = split.at(0).toInt();
				dataItem.position  = split.at(1).toFloat();
			} else if (line.startsWith(extinfPrefix)) {
				line.remove(0, extinfPrefix.size());

				QStringList split = line.split(",");
				if (split.count() != 2)
					continue;

				dataItem.duration  = split.at(0).toInt();
				dataItem.title     = split.at(1);
			}
		} else {
			dataItem.path = line;
			dataItemsList << dataItem;
			dataItem = NPlaylistDataItem();
		}
	}

	playlist.close();
	return dataItemsList;
}

void NPlaylistStorage::writeM3u(const QString &file, QList<NPlaylistDataItem> items)
{
	QFile playlist(file);
	if (!playlist.open(QFile::WriteOnly | QFile::Truncate))
		return;

	QTextStream out(&playlist);
	out << "#EXTM3U\n";

	for (int i = 0; i < items.count(); ++i) {
		QString itemPath = QFileInfo(items.at(i).path).canonicalFilePath();

		bool failed = items.at(i).failed || !QFileInfo(itemPath).exists();
		out << "#NULLOY:" << failed << "," << items.at(i).position << "\n";

		out << "#EXTINF:" << items.at(i).duration << "," << items.at(i).title << "\n";
		out << itemPath << "\n";
	}

	playlist.close();
}

