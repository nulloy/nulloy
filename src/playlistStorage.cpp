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

#include "playlistStorage.h"

#include <QFileInfo>
#include <QStringList>
#include <QTextStream>

/*
 *  Prefixed data order:
 *  #NULLOY:failed,playbackCount,playbackPosition,titleFormat
 *  #EXTINF:durationSeconds,playlistTitle
 */

QList<NPlaylistDataItem> NPlaylistStorage::readM3u(const QString &file)
{
    QList<NPlaylistDataItem> dataItemsList;
    QString nulloyPrefix = "#NULLOY:";
    QString extinfPrefix = "#EXTINF:";

    QString line;
    QFile playlist(file);
    if (!playlist.exists() || !playlist.open(QFile::ReadOnly)) {
        return dataItemsList;
    }

    QFileInfo playlistInfo(file);
    NPlaylistDataItem dataItem;
    QTextStream in(&playlist);
    in.setCodec("UTF-8");
    while (!in.atEnd()) {
        line = in.readLine();
        if (line.trimmed().isEmpty()) {
            continue;
        }
        if (line.startsWith("#")) {
            if (line.startsWith(nulloyPrefix)) {
                line.remove(0, nulloyPrefix.size());

                QStringList split = line.split(",");
                if (split.count() < 3) {
                    continue;
                }

                dataItem.failed = split.at(0).toInt();
                dataItem.playbackCount = split.at(1).toInt();
                dataItem.playbackPosition = split.at(2).toFloat();

                if (split.count() == 4) {
                    dataItem.titleFormat = split.at(3);
                }

            } else if (line.startsWith(extinfPrefix)) {
                line.remove(0, extinfPrefix.size());

                QStringList split = line.split(",");
                if (split.count() != 2) {
                    continue;
                }

                dataItem.duration = split.at(0).toInt();
                dataItem.title = split.at(1);
            }
        } else {
            QFileInfo fileInfo(line);
            if (fileInfo.isAbsolute()) {
                dataItem.path = line;
            } else {
                dataItem.path = playlistInfo.absolutePath() + "/" + line;
                fileInfo = QFileInfo(dataItem.path);
            }

            if (!fileInfo.exists()) {
                dataItem.failed = 1;
                dataItem.path = line;
            }

            if (dataItem.title.isEmpty()) {
                dataItem.title = line;
            }

            dataItemsList << dataItem;
            dataItem = NPlaylistDataItem();
        }
    }

    playlist.close();
    return dataItemsList;
}

void NPlaylistStorage::writeM3u(const QString &file, QList<NPlaylistDataItem> items,
                                N::M3uExtention ext)
{
    QFile playlist(file);
    if (!playlist.open(QFile::WriteOnly | QFile::Truncate)) {
        return;
    }

    QString playlistPath = QFileInfo(file).absolutePath();

    QTextStream out(&playlist);
    out.setCodec("UTF-8");
    if (ext >= N::ExtM3u) {
        out << "#EXTM3U\n";
    }

    for (int i = 0; i < items.count(); ++i) {
        bool failed = items.at(i).failed || !QFileInfo(items.at(i).path).exists();
        if (ext == N::NulloyM3u) {
            out << "#NULLOY:" << failed << "," << items.at(i).playbackCount << ","
                << items.at(i).playbackPosition << "," << items.at(i).titleFormat << "\n";
        }

        if (ext >= N::ExtM3u) {
            out << "#EXTINF:" << items.at(i).duration << "," << items.at(i).title << "\n";
        }

        if (QFileInfo(items.at(i).path).exists()) {
            if (playlistPath == QFileInfo(items.at(i).path).absolutePath()) { // same directory
                out << QFileInfo(items.at(i).path).fileName() << "\n";
            } else {
                out << QFileInfo(items.at(i).path).absoluteFilePath() << "\n";
            }
        } else { // keep as is
            out << items.at(i).path << "\n";
        }
    }

    playlist.close();
}
