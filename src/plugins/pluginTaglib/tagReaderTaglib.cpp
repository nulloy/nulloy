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

#include "tagReaderTaglib.h"

#include <aiffproperties.h>
#include <apeproperties.h>
#include <flacproperties.h>
#include <mp4properties.h>
#include <tpropertymap.h>
#include <trueaudioproperties.h>
#include <wavpackproperties.h>
#include <wavproperties.h>

#include <QFileInfo>
#include <QTextCodec>

#include "tagLibFileRef.h"

TagLib::FileRef *NTaglib::_tagRef;
QString NTaglib::_filePath;

void NTagReaderTaglib::init()
{
    if (m_init) {
        return;
    }

    m_init = true;
    NTaglib::_tagRef = NULL;
}

void NTagReaderTaglib::setSource(const QString &file)
{
    if (NTaglib::_filePath == file) {
        return;
    }

    m_isValid = false;

    if (NTaglib::_tagRef) {
        delete NTaglib::_tagRef;
        NTaglib::_tagRef = NULL;
    }

    NTaglib::_filePath = "";

    if (!QFileInfo(file).exists()) {
        return;
    }

    NTaglib::_filePath = file;

#ifdef WIN32
    NTaglib::_tagRef = new TagLib::FileRef(reinterpret_cast<const wchar_t *>(file.constData()));
#else
    NTaglib::_tagRef = new TagLib::FileRef(file.toUtf8().data());
#endif

    m_isValid = NTaglib::_tagRef->file() && NTaglib::_tagRef->file()->isValid();
}

void NTagReaderTaglib::setEncoding(const QString &encoding)
{
    m_codec = QTextCodec::codecForName(encoding.toUtf8());
    m_isUtf8 = (encoding == QLatin1String("UTF-8"));
}

NTagReaderTaglib::~NTagReaderTaglib()
{
    if (!m_init) {
        return;
    }

    if (NTaglib::_tagRef) {
        delete NTaglib::_tagRef;
        NTaglib::_tagRef = NULL;
    }
}

QString NTagReaderTaglib::getTag(QChar ch) const
{
    if (!m_isValid) {
        return "";
    }

    switch (ch.unicode()) {
        case 'a': // artist
            return m_codec->toUnicode(NTaglib::_tagRef->tag()->artist().toCString(m_isUtf8));
        case 't': // title
            return m_codec->toUnicode(NTaglib::_tagRef->tag()->title().toCString(m_isUtf8));
        case 'A': // album
            return m_codec->toUnicode(NTaglib::_tagRef->tag()->album().toCString(m_isUtf8));
        case 'c': // comment
            return m_codec->toUnicode(NTaglib::_tagRef->tag()->comment().toCString(m_isUtf8));
        case 'g': // genre
            return TStringToQString(NTaglib::_tagRef->tag()->genre());
        case 'y': { // year
            unsigned int res = NTaglib::_tagRef->tag()->year();
            if (res == 0) {
                return "";
            }
            return QString::number(res);
        }
        case 'n': { // track number
            unsigned int res = NTaglib::_tagRef->tag()->track();
            if (res == 0) {
                return "";
            }
            return QString::number(res);
        }
        case 'b': { // bit depth
            TagLib::AudioProperties *ap = NTaglib::_tagRef->audioProperties();
            if (auto *prop = dynamic_cast<TagLib::APE::Properties *>(ap)) {
                return QString::number(prop->bitsPerSample());
            } else if (auto *prop = dynamic_cast<TagLib::FLAC::Properties *>(ap)) {
                return QString::number(prop->bitsPerSample());
            } else if (auto *prop = dynamic_cast<TagLib::MP4::Properties *>(ap)) {
                return QString::number(prop->bitsPerSample());
            } else if (auto *prop = dynamic_cast<TagLib::RIFF::AIFF::Properties *>(ap)) {
                return QString::number(prop->bitsPerSample());
            } else if (auto *prop = dynamic_cast<TagLib::RIFF::WAV::Properties *>(ap)) {
                return QString::number(prop->bitsPerSample());
            } else if (auto *prop = dynamic_cast<TagLib::TrueAudio::Properties *>(ap)) {
                return QString::number(prop->bitsPerSample());
            } else if (auto *prop = dynamic_cast<TagLib::WavPack::Properties *>(ap)) {
                return QString::number(prop->bitsPerSample());
            } else {
                return "";
            }
        }
        case 'D': { // duration in seconds
            int seconds = NTaglib::_tagRef->audioProperties()->length();
            if (seconds == 0) {
                return "";
            }
            return QString::number(seconds);
        }
        case 'B': { // bitrate in Kbps
            int res = NTaglib::_tagRef->audioProperties()->bitrate();
            if (res == 0) {
                return "";
            }
            return QString::number(res);
        }
        case 's': { // sample rate in kHz
            int res = NTaglib::_tagRef->audioProperties()->sampleRate();
            if (res == 0) {
                return "";
            }
            return QString::number(res / (float)1000);
        }
        case 'H': { // number of channels
            int res = NTaglib::_tagRef->audioProperties()->channels();
            if (res == 0) {
                return "";
            }
            return QString::number(res);
        }
        case 'M': { // beats per minute
            return NTaglib::_tagRef->file()->properties()["BPM"].toString().toCString(m_isUtf8);
        }
        default: // unsupported, convert to a tag and return
            return QString('%') + ch;
    }
}

N::Tag NTagReaderTaglib::tagFromKey(const QString &key) const
{
    if (key == "ALBUM") {
        return N::AlbumTag;
    } else if (key == "ARTIST") {
        return N::ArtistTag;
    } else if (key == "BPM") {
        return N::BpmTag;
    } else if (key == "COMMENT") {
        return N::CommentTag;
    } else if (key == "COMPOSER") {
        return N::ComposerTag;
    } else if (key == "COPYRIGHT") {
        return N::CopyrightTag;
    } else if (key == "ENCODEDBY") {
        return N::EncodedByTag;
    } else if (key == "GENRE") {
        return N::GenreTag;
    } else if (key == "PUBLISHER") {
        return N::PublisherTag;
    } else if (key == "TITLE") {
        return N::TitleTag;
    } else if (key == "TRACKNUMBER") {
        return N::TrackNumberTag;
    } else if (key == "URL") {
        return N::UrlTag;
    } else if (key == "DATE") {
        return N::DateTag;
    } else {
        return N::UnknownTag;
    }
}

QString NTagReaderTaglib::tagToKey(N::Tag tag) const
{
    switch (tag) {
        case N::AlbumTag:
            return "ALBUM";
        case N::ArtistTag:
            return "ARTIST";
        case N::BpmTag:
            return "BPM";
        case N::CommentTag:
            return "COMMENT";
        case N::ComposerTag:
            return "COMPOSER";
        case N::CopyrightTag:
            return "COPYRIGHT";
        case N::EncodedByTag:
            return "ENCODEDBY";
        case N::GenreTag:
            return "GENRE";
        case N::PublisherTag:
            return "PUBLISHER";
        case N::TitleTag:
            return "TITLE";
        case N::TrackNumberTag:
            return "TRACKNUMBER";
        case N::UrlTag:
            return "URL";
        case N::DateTag:
            return "DATE";
    }
}

QMap<QString, QStringList>
NTagReaderTaglib::TMapToQMap(const TagLib::Map<TagLib::String, TagLib::StringList> &tmap) const
{
    QMap<QString, QStringList> qmap;
    for (auto iter = tmap.begin(); iter != tmap.end(); ++iter) {
        QStringList values;
        TagLib::StringList tlist = iter->second;
        for (auto iter = tlist.begin(); iter != tlist.end(); ++iter) {
            values << m_codec->toUnicode((*iter).toCString(m_isUtf8));
        }
        qmap[TStringToQString(iter->first)] = values;
    }
    return qmap;
}

TagLib::Map<TagLib::String, TagLib::StringList>
NTagReaderTaglib::QMapToTMap(const QMap<QString, QStringList> &qmap) const
{
    TagLib::Map<TagLib::String, TagLib::StringList> tmap;
    foreach (QString key, qmap.keys()) {
        TagLib::StringList tlist;
        QStringList values = qmap[key];
        foreach (QString value, values) {
            tlist.append(QStringToTString(value));
        }
        tmap.insert(QStringToTString(key), tlist);
    }
    return tmap;
}

QMap<QString, QStringList> NTagReaderTaglib::getTags() const
{
    if (!m_isValid) { // workaround to relay the error
        QMap<QString, QStringList> tags;
        tags["Error"] = QStringList() << "Invalid";
        return tags;
    }
    return TMapToQMap(NTaglib::_tagRef->file()->properties());
}

QMap<QString, QStringList> NTagReaderTaglib::setTags(const QMap<QString, QStringList> &tags)
{
    QMap<QString, QStringList> unsaved = TMapToQMap(
        NTaglib::_tagRef->file()->setProperties(QMapToTMap(tags)));
    if (unsaved.isEmpty()) {
        bool success = NTaglib::_tagRef->file()->save();
        if (!success) { // workaround to relay the error
            unsaved["Error"] = QStringList() << "Write";
        }
    }
    return unsaved;
}
