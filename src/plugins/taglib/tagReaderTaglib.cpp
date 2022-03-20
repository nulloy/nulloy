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
                return QString::number(prop->sampleWidth());
            } else if (auto *prop = dynamic_cast<TagLib::MP4::Properties *>(ap)) {
                return QString::number(prop->bitsPerSample());
            } else if (auto *prop = dynamic_cast<TagLib::RIFF::AIFF::Properties *>(ap)) {
                return QString::number(prop->sampleWidth());
            } else if (auto *prop = dynamic_cast<TagLib::RIFF::WAV::Properties *>(ap)) {
                return QString::number(prop->sampleWidth());
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
            return NTaglib::_tagRef->tag()->properties()["BPM"].toString().toCString(m_isUtf8);
        }
        default: // unsupported, convert to a tag and return
            return QString('%') + ch;
    }
}
