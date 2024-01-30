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

#include "tagReaderGstreamer.h"

#include <gst/pbutils/pbutils.h>

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

void NTagReaderGstreamer::setEncoding(const QString &encoding)
{
    m_codec = QTextCodec::codecForName(encoding.toUtf8());
    m_isUtf8 = (encoding == QLatin1String("UTF-8"));
}

QString NTagReaderGstreamer::gCharToUnicode(const char *str) const
{
    if (!m_isUtf8) {
        return m_codec->toUnicode(QString::fromUtf8(str).toLatin1());
    } else {
        return m_codec->toUnicode(str);
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

QString NTagReaderGstreamer::getTag(QChar ch) const
{
    if (!m_isValid) {
        return "";
    }

    gchar *gstr = NULL;
    QString res;
    switch (ch.unicode()) {
        case 'a': { // artist
            if (gst_tag_list_get_string(m_taglist, GST_TAG_ARTIST, &gstr)) {
                res = gCharToUnicode(gstr);
            }
            break;
        }
        case 't': { // title
            if (gst_tag_list_get_string(m_taglist, GST_TAG_TITLE, &gstr)) {
                res = gCharToUnicode(gstr);
            }
            break;
        }
        case 'A': { // album
            if (gst_tag_list_get_string(m_taglist, GST_TAG_ALBUM, &gstr)) {
                res = gCharToUnicode(gstr);
            }
            break;
        }
        case 'c': { // comment
            if (gst_tag_list_get_string(m_taglist, GST_TAG_COMMENT, &gstr)) {
                res = gCharToUnicode(gstr);
            }
            break;
        }
        case 'g': { // genre
            if (gst_tag_list_get_string(m_taglist, GST_TAG_GENRE, &gstr)) {
                res = QString::fromUtf8(gstr);
            }
            break;
        }
        case 'y': { // year
            GDate *date = NULL;
            if (gst_tag_list_get_date(m_taglist, GST_TAG_DATE, &date)) {
                GDateYear year = g_date_get_year(date);
                if (year != G_DATE_BAD_YEAR) {
                    res = QString::number(year);
                }
            }
            break;
        }
        case 'n': { // track number
            unsigned int track = 0;
            if (gst_tag_list_get_uint(m_taglist, GST_TAG_TRACK_NUMBER, &track)) {
                res = QString::number(track);
            }
            break;
        }
        case 'b': { // bit depth
            if (!m_codecName.contains("MP3")) {
                res = QString::number(m_bitDepth);
            }
            break;
        }
        case 'D': { // duration in seconds
            int seconds_total = GST_TIME_AS_SECONDS(m_nanosecs);
            if (seconds_total > 0) {
                res = QString::number(seconds_total);
            }
            break;
        }
        case 'B': { // bitrate in Kbps
            unsigned int bitrate = 0;
            if (gst_tag_list_get_uint(m_taglist, GST_TAG_BITRATE, &bitrate)) {
                res = QString::number(bitrate / 1000);
            }
            break;
        }
        case 's': { // sample rate in kHz
            res = QString::number(m_sampleRate);
            break;
        }
        case 'H': { // number of channels
            break;  // unsupported
        }
    }
    g_free(gstr);
    return res;
}
