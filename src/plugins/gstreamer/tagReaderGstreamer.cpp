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

#include "tagReaderGstreamer.h"

#include <gst/pbutils/pbutils.h>

#include <QCoreApplication>
#include <QFileInfo>
#include <QTextCodec>

#include "common.h"

void NTagReaderGstreamer::init()
{
    if (m_init) {
        return;
    }

    m_isValid = false;
    m_taglist = NULL;

    int argc;
    const char **argv;
    GError *err = NULL;
    NCore::cArgs(&argc, &argv);
    if (!gst_init_check(&argc, (char ***)&argv, &err)) {
        qWarning() << "NTagReaderGstreamer :: gst_init_check error ::"
                   << (err ? QString::fromUtf8(err->message) : "unknown error");
        if (err) {
            g_error_free(err);
        }
        return;
    }

    m_init = true;
}

void NTagReaderGstreamer::setSource(const QString &file)
{
    if (m_taglist) {
        gst_tag_list_free(m_taglist);
        m_taglist = NULL;
    }

    m_isValid = false;
    m_path = "";

    QFileInfo fileInfo(file);
    if (!fileInfo.exists()) {
        return;
    }

    m_path = file;
    gchar *uri = g_filename_to_uri(fileInfo.absoluteFilePath().toUtf8().constData(), NULL, NULL);

    GError *err = NULL;
    GstDiscoverer *discoverer = gst_discoverer_new(GST_SECOND * 60, &err);
    if (discoverer == NULL) {
        qWarning() << "NTagReaderGstreamer :: GstDiscoverer error ::"
                   << (err ? QString::fromUtf8(err->message) : "unknown error");
        if (err) {
            g_error_free(err);
        }
        return;
    }

    GstDiscovererInfo *info = gst_discoverer_discover_uri(discoverer, uri, &err);
    GList *audioInfo = gst_discoverer_info_get_audio_streams(info);
    if (!audioInfo) {
        qWarning() << "NTagReaderGstreamer :: GstDiscoverer error ::"
                   << "not an audio file";
        return;
    }

    m_sampleRate = gst_discoverer_audio_info_get_sample_rate(
                       (GstDiscovererAudioInfo *)audioInfo->data) /
                   (float)1000;
    m_bitDepth = gst_discoverer_audio_info_get_depth((GstDiscovererAudioInfo *)audioInfo->data);
    gst_discoverer_stream_info_list_free(audioInfo);

    m_nanosecs = gst_discoverer_info_get_duration(info);

    const GstTagList *tagList = gst_discoverer_info_get_tags(info);
    m_taglist = gst_tag_list_copy(tagList);
    if (GST_IS_TAG_LIST(m_taglist) && !gst_tag_list_is_empty(m_taglist)) {
        gchar *gstr = NULL;
        if (gst_tag_list_get_string(m_taglist, GST_TAG_AUDIO_CODEC, &gstr)) {
            m_codecName = QString::fromUtf8(gstr);
        }
        m_isValid = true;
    }
}

NTagReaderGstreamer::~NTagReaderGstreamer()
{
    if (!m_init) {
        return;
    }

    if (m_taglist) {
        gst_tag_list_free(m_taglist);
    }
}

QString NTagReaderGstreamer::toString(const QString &file, const QString &format,
                                      const QString &encoding)
{
    setSource(file);
    if (!m_isValid) {
        return "";
    }

    bool res;
    return parse(format, &res, encoding);
}

QString NTagReaderGstreamer::parse(const QString &format, bool *success, const QString &encoding,
                                   bool stopOnFail) const
{
    if (format.isEmpty()) {
        return "<Format is empty>";
    }

    *success = true;

    int seconds_total = GST_TIME_AS_SECONDS(m_nanosecs);

    QString res;
    QTextCodec *codec = QTextCodec::codecForName(encoding.toUtf8());
    for (int i = 0; i < format.size(); ++i) {
        if (format.at(i) == '%') {
            gchar *gstr = NULL;
            ++i;
            QChar ch = format.at(i);
            if (ch == 'a') { // artist
                if (!(*success = gst_tag_list_get_string(m_taglist, GST_TAG_ARTIST, &gstr))) {
                    res += "<Unknown artist>";
                } else {
                    res += codec->toUnicode(QString::fromUtf8(gstr).toLatin1());
                }
            } else if (ch == 't') { // title
                if (!(*success = gst_tag_list_get_string(m_taglist, GST_TAG_TITLE, &gstr))) {
                    res += "<Unknown title>";
                } else {
                    res += codec->toUnicode(QString::fromUtf8(gstr).toLatin1());
                }
            } else if (ch == 'A') { // album
                if (!(*success = gst_tag_list_get_string(m_taglist, GST_TAG_ALBUM, &gstr))) {
                    res += "<Unknown album>";
                } else {
                    res += codec->toUnicode(QString::fromUtf8(gstr).toLatin1());
                }
            } else if (ch == 'c') { // comment
                if (!(*success = gst_tag_list_get_string(m_taglist, GST_TAG_COMMENT, &gstr))) {
                    res += "<Empty comment>";
                } else {
                    res += codec->toUnicode(QString::fromUtf8(gstr).toLatin1());
                }
            } else if (ch == 'g') { // genre
                if (!(*success = gst_tag_list_get_string(m_taglist, GST_TAG_GENRE, &gstr))) {
                    res += "<Unknown genre>";
                } else {
                    res += QString::fromUtf8(gstr);
                }
            } else if (ch == 'y') { // year
                GDate *date = NULL;
                QString str = "0";
                if (gst_tag_list_get_date(m_taglist, GST_TAG_DATE, &date)) {
                    GDateYear year = g_date_get_year(date);
                    if (year != G_DATE_BAD_YEAR) {
                        str = QString::number(year);
                    }
                }
                if (str == "0") {
                    str = "<Unknown year>";
                    *success = false;
                }
                res += str;
            } else if (ch == 'n') { // track number
                unsigned int track = 0;
                QString str;
                if ((*success = gst_tag_list_get_uint(m_taglist, GST_TAG_TRACK_NUMBER, &track))) {
                    str = QString::number(track);
                } else {
                    str = "<Unknown track number>";
                }
                res += str;
            } else if (ch == 'b') { // bit depth
                if (m_codecName.contains("MP3")) {
                    res += "<Unknown bit depth>";
                    *success = false;
                } else {
                    res += QString::number(m_bitDepth);
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
                    *success = false;
                    duration = "<Unknown duration>";
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
                unsigned int bitrate = 0;
                QString str;
                if ((*success = gst_tag_list_get_uint(m_taglist, GST_TAG_BITRATE, &bitrate))) {
                    str = QString::number(bitrate / 1000);
                } else {
                    str = "<Unknown bitrate>";
                }
                res += str;
            } else if (ch == 's') { // sample rate in kHz
                res += QString::number(m_sampleRate);
            } else if (ch == 'H') { // number of channels
                res += "<Usupported tag: channels number>";
                *success = false;
            } else if (ch == 'f') { // file name without extension
                res += QFileInfo(m_path).baseName();
            } else if (ch == 'F') { // file name
                res += QFileInfo(m_path).fileName();
            } else if (ch == 'p') { // file name including absolute path
                res += QFileInfo(m_path).absoluteFilePath();
            } else if (ch == 'P') { // directory path without file name
                res += QFileInfo(m_path).canonicalPath();
            } else if (ch == 'e') { // file name extension
                res += QFileInfo(m_path).suffix();
            } else if (ch == 'E') { // file name extension, uppercased
                res += QFileInfo(m_path).suffix().toUpper();
            } else if (ch == 'v') { // Nulloy version number
                res += QCoreApplication::applicationVersion();
            } else {
                res += ch;
            }
            g_free(gstr);
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
