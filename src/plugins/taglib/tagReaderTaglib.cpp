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
#include <trueaudioproperties.h>
#include <wavpackproperties.h>
#include <wavproperties.h>

#include <QCoreApplication>
#include <QFileInfo>
#include <QString>
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

QString NTagReaderTaglib::toString(const QString &file, const QString &format,
                                   const QString &encoding)
{
    setSource(file);
    if (!isValid()) {
        return "";
    }
    bool res;
    return parse(format, &res, encoding);
}

QString NTagReaderTaglib::parse(const QString &format, bool *success, const QString &encoding,
                                bool stopOnFail) const
{
    if (format.isEmpty()) {
        return "<Format is empty>";
    }

    *success = true;

    TagLib::Tag *tag = NTaglib::_tagRef->tag();
    TagLib::AudioProperties *ap = NTaglib::_tagRef->audioProperties();

    int seconds_total = ap->length();

    QString res;
    QTextCodec *codec = QTextCodec::codecForName(encoding.toUtf8());
    bool isUtf8 = (encoding == QLatin1String("UTF-8"));
    for (int i = 0; i < format.size(); ++i) {
        if (format.at(i) == '%') {
            ++i;
            QChar ch = format.at(i);
            if (ch == 'a') { // artist
                QString str = codec->toUnicode(tag->artist().toCString(isUtf8));
                if (!(*success = !str.isEmpty())) {
                    str = "<Unknown artist>";
                }
                res += str;
            } else if (ch == 't') { // title
                QString str = codec->toUnicode(tag->title().toCString(isUtf8));
                if (!(*success = !str.isEmpty())) {
                    str = "<Unknown title>";
                }
                res += str;
            } else if (ch == 'A') { // album
                QString str = codec->toUnicode(tag->album().toCString(isUtf8));
                if (!(*success = !str.isEmpty())) {
                    str = "<Unknown album>";
                }
                res += str;
            } else if (ch == 'c') { // comment
                QString str = codec->toUnicode(tag->comment().toCString(isUtf8));
                if (!(*success = !str.isEmpty())) {
                    str = "<Empty comment>";
                }
                res += str;
            } else if (ch == 'g') { // genre
                QString str = TStringToQString(tag->genre());
                if (!(*success = !str.isEmpty())) {
                    str = "<Unknown genre>";
                }
                res += str;
            } else if (ch == 'y') { // year
                QString str = QString::number(tag->year());
                if (str == "0") {
                    str = "<Unknown year>";
                    *success = false;
                }
                res += str;
            } else if (ch == 'n') { // track number
                QString str = QString::number(tag->track());
                if (str == "0") {
                    str = "<Unknown track number>";
                    *success = false;
                }
                res += str;
            } else if (ch == 'b') { // bit depth
                if (auto *prop = dynamic_cast<TagLib::APE::Properties *>(ap)) {
                    res += QString::number(prop->bitsPerSample());
                } else if (auto *prop = dynamic_cast<TagLib::FLAC::Properties *>(ap)) {
                    res += QString::number(prop->sampleWidth());
                } else if (auto *prop = dynamic_cast<TagLib::MP4::Properties *>(ap)) {
                    res += QString::number(prop->bitsPerSample());
                } else if (auto *prop = dynamic_cast<TagLib::RIFF::AIFF::Properties *>(ap)) {
                    res += QString::number(prop->sampleWidth());
                } else if (auto *prop = dynamic_cast<TagLib::RIFF::WAV::Properties *>(ap)) {
                    res += QString::number(prop->sampleWidth());
                } else if (auto *prop = dynamic_cast<TagLib::TrueAudio::Properties *>(ap)) {
                    res += QString::number(prop->bitsPerSample());
                } else if (auto *prop = dynamic_cast<TagLib::WavPack::Properties *>(ap)) {
                    res += QString::number(prop->bitsPerSample());
                } else {
                    res += "<Unknown bit depth>";
                    *success = false;
                }
            } else if (ch == 'd') { // duration as hh:mm:ss
                QString duration;
                if (seconds_total > 0) {
                    int seconds = seconds_total % 60;
                    int minutes = (seconds_total - seconds) / 60;
                    int hours = minutes / 60;
                    minutes = minutes % 60;
                    if (hours > 0) {
                        duration.sprintf("%d:%02d:%02d", hours, minutes, seconds);
                    } else {
                        duration.sprintf("%d:%02d", minutes, seconds);
                    }
                } else {
                    duration = "<Unknown duration>";
                    *success = false;
                }
                res += duration;
            } else if (ch == 'D') { // duration in seconds
                QString duration;
                if (seconds_total == 0) {
                    duration = "<Unknown duration>";
                    *success = false;
                } else {
                    duration = QString::number(seconds_total);
                }
                res += duration;
            } else if (ch == 'B') { // bitrate in Kbps
                QString str = QString::number(ap->bitrate());
                if (str == "0") {
                    str = "<Unknown bitrate>";
                    *success = false;
                }
                res += str;
            } else if (ch == 's') { // sample rate in kHz
                QString str = QString::number(ap->sampleRate() / (float)1000);
                if (str == "0") {
                    str = "<Unknown sample rate>";
                    *success = false;
                }
                res += str;
            } else if (ch == 'H') { // number of channels
                QString str = QString::number(ap->channels());
                if (str == "0") {
                    str = "<Unknown channels number>";
                    *success = false;
                }
                res += str;
            } else if (ch == 'f') { // file name without extension
                res += QFileInfo(NTaglib::_filePath).baseName();
            } else if (ch == 'F') { // file name
                res += QFileInfo(NTaglib::_filePath).fileName();
            } else if (ch == 'p') { // file name including absolute path
                res += QFileInfo(NTaglib::_filePath).absoluteFilePath();
            } else if (ch == 'P') { // directory path without file name
                res += QFileInfo(NTaglib::_filePath).canonicalPath();
            } else if (ch == 'e') { // file name extension
                res += QFileInfo(NTaglib::_filePath).suffix();
            } else if (ch == 'E') { // file name extension, uppercased
                res += QFileInfo(NTaglib::_filePath).suffix().toUpper();
            } else if (ch == 'v') { // Nulloy version number
                res += QCoreApplication::applicationVersion();
            } else {
                res += ch;
            }
        } else if (format.at(i) == '{') {
            ++i;
            int matchedAt = format.indexOf('}', i);
            if (matchedAt == -1) {
                res += "<condition error: unmatched '{'>";
                return res;
            }

            QString condition = format.mid(i, matchedAt - i);

            if (condition.indexOf('{') != -1) {
                res += "<condition error: extra '{'>";
                return res;
            }

            QStringList values = condition.split('|');
            if (values.count() < 2) {
                res += "<condition error: missing '|'>";
                return res;
            } else if (values.count() > 2) {
                res += "<condition error: extra '|'>";
                return res;
            }

            bool cond_res;
            QString cond_true = parse(values.at(0), &cond_res, encoding, true);
            if (cond_res) {
                res += cond_true;
            } else {
                res += parse(values.at(1), &cond_res, encoding);
            }
            i = matchedAt;
        } else {
            res += format.at(i);
        }
        if (!*success && stopOnFail) {
            return "";
        }
    }

    return res;
}

bool NTagReaderTaglib::isValid() const
{
    return (NTaglib::_tagRef && NTaglib::_tagRef->file() && NTaglib::_tagRef->file()->isValid());
}
