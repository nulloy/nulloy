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

#include "trackInfoReader.h"
#include "pluginLoader.h"
#include "settings.h"

QString NTrackInfoReader::formatTime(int durationSec)
{
    int seconds = durationSec % 60;
    int minutes = (durationSec - seconds) / 60;
    int hours = minutes / 60;
    minutes = minutes % 60;
    if (hours > 0) {
        return QString::asprintf("%d:%02d:%02d", hours, minutes, seconds);
    } else {
        return QString::asprintf("%d:%02d", minutes, seconds);
    }
}

NTrackInfoReader::NTrackInfoReader(NTagReaderInterface *tagReader, QObject *parent)
    : QObject(parent)
{
    m_durationSec = -1;
    m_reader = tagReader;
    Q_ASSERT(m_reader);
}

void NTrackInfoReader::setSource(const QString &file)
{
    m_fileInfo = QFileInfo(file);
    m_reader->setSource(file);
    m_reader->setEncoding(NSettings::instance()->value("EncodingTrackInfo").toString());

    QString seconds = m_reader->getTag('D');
    if (seconds.isEmpty()) {
        m_durationSec = -1;
    } else {
        m_durationSec = seconds.toInt();
    }
    m_positionSec = -1;
}

void NTrackInfoReader::updatePlaybackPosition(int seconds)
{
    m_positionSec = seconds;
}

QString NTrackInfoReader::toString(const QString &format) const
{
    int cur = 0;
    bool ok = true;
    return parseFormat(format, cur, false, ok);
}

QString NTrackInfoReader::getInfo(QChar ch) const
{
    switch (ch.unicode()) {
        case 'f': // file name without extension
            return m_fileInfo.baseName();
        case 'F': // file name
            return m_fileInfo.fileName();
        case 'p': // file name including absolute path
            return m_fileInfo.absoluteFilePath();
        case 'P': // directory path without file name
            return m_fileInfo.canonicalPath();
        case 'N': // directory name
            return m_fileInfo.absoluteDir().dirName();
        case 'e': // file name extension
            return m_fileInfo.suffix();
        case 'E': // file name extension, uppercased
            return m_fileInfo.suffix().toUpper();
        case 'D': // duration in seconds
            if (m_durationSec < 0) {
                return "";
            }
            return QString::number(m_durationSec);
        case 'd': { // duration as hh:mm:ss
            if (m_durationSec < 0) {
                return "";
            }
            return formatTime(m_durationSec);
        }
        case 'T': { // elapsed playback time as hh:mm:ss
            if (m_positionSec < 0) {
                return "";
            }
            return formatTime(m_positionSec);
        }
        case 'r': { // remaining playback time as hh:mm:ss
            if (m_durationSec < 0 || m_positionSec < 0) {
                return "";
            }
            return formatTime(m_durationSec - m_positionSec);
        }
        case 'v':
            return QCoreApplication::applicationVersion();
        default:
            return m_reader->getTag(ch);
    }
}

QString NTrackInfoReader::parseFormat(const QString &format, int &cur, bool skip, bool &ok) const
{
    QString res;
    int len = format.size();
    while (cur < len) {
        QChar ch = format.at(cur);
        if (skip && ch != '{') {
            while (ch != '|' && ch != '}' && cur < len) {
                ++cur;
                ch = format.at(cur).unicode();
                if (format.at(cur - 1) == '\\') { // escaped
                    continue;
                }
            }
        }
        switch (ch.unicode()) {
            case '\\': { // escaped
                ++cur;
                res += format.at(cur);
                break;
            }
            case '%': {
                ++cur;
                ch = format.at(cur);
                QString str = getInfo(ch);
                if (str.isEmpty()) { // failed
                    res = "";
                    skip = true; // skip the rest of this alternative
                    ok = false;
                } else {
                    res += str;
                    // allow skipping following alternatives, if any:
                    ok = true;
                }
                break;
            }
            case '|': { // alternative
                if (ok) {
                    // previous alternative was successful, will skip all the following
                    skip = true;
                } else {
                    // allow evaluating this alternative:
                    ok = true;
                    skip = false;
                }
                break;
            }
            case '{': { // condition starts
                ++cur;
                res += parseFormat(format, cur, skip, ok);
                break;
            }
            case '}': { // condition ends
                return res;
            }
            default:
                res += QString(ch);
        }
        ++cur;
    }
    return res;
}
