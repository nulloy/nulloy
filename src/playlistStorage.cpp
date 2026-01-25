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

#include "playlistStorage.h"

#include <QFileInfo>
#include <QStringList>
#include <QTextStream>

/*
 *  Prefixed data order:
 *  #NULLOY:failed,playbackCount,playbackPosition,titleFormat
 *  #EXTINF:durationSeconds,playlistTitle
 */

QList<NPlaylistModel::DataItem> NPlaylistStorage::readM3u(const QString &file)
{
    QList<NPlaylistModel::DataItem> dataItemsList;
    QString nulloyPrefix = "#NULLOY:";
    QString extinfPrefix = "#EXTINF:";

    QString line;
    QFile playlist(file);
    if (!playlist.exists() || !playlist.open(QFile::ReadOnly)) {
        return dataItemsList;
    }

    QFileInfo playlistInfo(file);
    NPlaylistModel::DataItem dataItem;
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

                dataItem.isFailed = split.at(0).toInt();
                dataItem.playbackCount = split.at(1).toInt();
                dataItem.playbackPosition = split.at(2).toFloat();

                if (split.count() == 4) {
                    dataItem.textFormat = split.at(3);
                }

            } else if (line.startsWith(extinfPrefix)) {
                line.remove(0, extinfPrefix.size());

                QStringList split = line.split(",");
                if (split.count() != 2) {
                    continue;
                }

                dataItem.durationSec = split.at(0).toInt();
                dataItem.text = split.at(1);
            }
        } else {
            QFileInfo fileInfo(line);
            if (fileInfo.isAbsolute()) {
                dataItem.filePath = line;
            } else {
                dataItem.filePath = playlistInfo.absolutePath() + "/" + line;
                fileInfo = QFileInfo(dataItem.filePath);
            }

            if (!fileInfo.exists()) {
                dataItem.isFailed = true;
                dataItem.filePath = line;
            }

            if (dataItem.text.isEmpty()) {
                dataItem.text = line;
            }

            dataItemsList << dataItem;
            dataItem = NPlaylistModel::DataItem();
        }
    }

    playlist.close();
    return dataItemsList;
}

void NPlaylistStorage::writeM3u(const QString &file, QList<NPlaylistModel::DataItem> items,
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
        bool failed = items.at(i).isFailed || !QFileInfo(items.at(i).filePath).exists();
        if (ext == N::NulloyM3u) {
            out << "#NULLOY:" << failed << "," << items.at(i).playbackCount << ","
                << items.at(i).playbackPosition << "," << items.at(i).textFormat << "\n";
        }

        if (ext >= N::ExtM3u) {
            out << "#EXTINF:" << items.at(i).durationSec << "," << items.at(i).text << "\n";
        }

        if (QFileInfo(items.at(i).filePath).exists()) {
            if (playlistPath == QFileInfo(items.at(i).filePath).absolutePath()) { // same directory
                out << QFileInfo(items.at(i).filePath).fileName() << "\n";
            } else {
                out << QFileInfo(items.at(i).filePath).absoluteFilePath() << "\n";
            }
        } else { // keep as is
            out << items.at(i).filePath << "\n";
        }
    }

    playlist.close();
}
