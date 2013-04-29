/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2012 Sergey Vlasov <sergey@vlasov.me>
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

#include "core.h"

#include <QCoreApplication>
#include <gst/pbutils/pbutils.h>
#include <QFileInfo>

void NTagReaderGstreamer::init()
{
	if (m_init)
		return;

	m_isValid = FALSE;
	m_taglist = NULL;

#ifdef Q_WS_WIN
	_putenv("GST_PLUGIN_PATH=Plugins\\GStreamer");
#endif

#ifdef Q_WS_MAC
	QDir executable_path(QCoreApplication::applicationDirPath());
	if (executable_path.dirName() == "MacOS") {
		executable_path.cd("GStreamer/plugins");
		if (executable_path.exists())
			putenv(QString("GST_PLUGIN_PATH=" + executable_path.absolutePath() +
							":" + getenv("GST_PLUGIN_PATH")).toAscii().data());
	}
#endif

	int argc;
	const char **argv;
	GError *init_err;
	NCore::cArgs(&argc, &argv);
	if (!gst_init_check(&argc, (char ***)&argv, &init_err)) {
		g_error("NTagReaderGstreamer :: error: %s", init_err->message);
		return;
	}

	m_init = TRUE;
}

void NTagReaderGstreamer::setSource(const QString &file)
{
	if (m_taglist)
		gst_tag_list_free(m_taglist);

	m_isValid = FALSE;

	m_path = file;
	gchar *uri = g_filename_to_uri(QFileInfo(file).absoluteFilePath().toUtf8().constData(), NULL, NULL);

	GError *err = NULL;
	GstDiscoverer *discoverer = gst_discoverer_new(GST_SECOND * 60, &err);
	if (discoverer == NULL) {
		qWarning() << "NTagReaderGstreamer :: GstDiscoverer error ::" << err->message;
		g_error_free(err);
		return;
	}

	GstDiscovererInfo *info = gst_discoverer_discover_uri(discoverer, uri, &err);
	GList *audioInfo = gst_discoverer_info_get_audio_streams(info);
	if (!audioInfo) {
		qWarning() << "NTagReaderGstreamer :: GstDiscoverer error ::" << "not an audio file";
		return;
	}

	m_sampleRate = gst_discoverer_audio_info_get_sample_rate((GstDiscovererAudioInfo *)audioInfo->data) / (float)1000;
	gst_discoverer_stream_info_list_free(audioInfo);

	m_nanosecs = gst_discoverer_info_get_duration(info);

	const GstTagList *tagList = gst_discoverer_info_get_tags(info);
	m_taglist = gst_tag_list_copy(tagList);
	if (gst_is_tag_list(m_taglist) && !gst_tag_list_is_empty(m_taglist))
		m_isValid = TRUE;
}

NTagReaderGstreamer::~NTagReaderGstreamer()
{
	if (!m_init)
		return;
	if (m_taglist)
		gst_tag_list_free(m_taglist);
}

QString NTagReaderGstreamer::toString(const QString &format)
{
	if (format.isEmpty())
		return "";

	if (!m_isValid)
		return "NTagReaderGstreamer::InvalidFile";

	int seconds_total = GST_TIME_AS_SECONDS(m_nanosecs);

	QString res;
	for (int i = 0; i < format.size(); ++i) {
		gchar *gstr = NULL;
		if (format.at(i) == '%') {
			++i;
			QChar ch = format.at(i);
			if (ch == 'a') {
				if (!gst_tag_list_get_string(m_taglist, "artist", &gstr))
					res += "<Unknown artist>";
				else
					res += gstr;
			} else if (ch == 't') {
				if (!gst_tag_list_get_string(m_taglist, "title", &gstr))
					res += QFileInfo(m_path).baseName();
				else
					res += gstr;
			} else if (ch == 'A') {
				if (!gst_tag_list_get_string(m_taglist, "album", &gstr))
					res += "<Unknown album>";
				else
					res += gstr;
			} else if (ch == 'c') {
				if (!gst_tag_list_get_string(m_taglist, "comment", &gstr))
					res += "<Empty comment>";
				else
					res += gstr;
			} else if (ch == 'g') {
				if (!gst_tag_list_get_string(m_taglist, "genre", &gstr))
					res += "<Unknown genre>";
				else
					res += gstr;
			} else if (ch == 'y') {
				GDate *date = NULL;
				QString str = "0";
				if (gst_tag_list_get_date(m_taglist, "date", &date)) {
					GDateYear year = g_date_get_year(date);
					if (year != G_DATE_BAD_YEAR)
						str = QString::number(year);
				}
				if (str == "0")
					str = "<Unknown year>";
				res += str;
			} else if (ch == 'n') {
				unsigned int track = 0;
				if (!gst_tag_list_get_uint(m_taglist, "track-number", &track))
					res += "<Unknown track number>";
				else
					res += QString::number(track);
			} else if (ch == 'd') {
				QString duration;
				if (seconds_total > 0) {
					int seconds = seconds_total % 60;
					int minutes = (seconds_total - seconds) / 60;
					int hours = minutes / 60;
					minutes = minutes % 60;
					if (hours > 0)
						duration.sprintf("%d:%02d:%02d", hours, minutes, seconds);
					else
						duration.sprintf("%d:%02d", minutes, seconds);
				} else {
					duration = "<Unknown duration>";
				}
				res += duration;
			} else if (ch == 'D') {
				QString duration;
				if (seconds_total == 0)
					duration = "<Unknown duration>";
				else
					duration = QString::number(seconds_total);
				res += duration;
			} else if (ch == 'B') {
				unsigned int bitrate = 0;
				if (!gst_tag_list_get_uint(m_taglist, "bitrate", &bitrate))
					res += "<Unknown bitrate>";
				else
					res += QString::number(bitrate / 1000);
			} else if (ch == 's') {
				res += QString::number(m_sampleRate);
			} else if (ch == 'C') {
				res += "<Usupported tag: channels number>";
			} else if (ch == 'f') {
				res += QFileInfo(m_path).baseName();
			} else if (ch == 'F') {
				res += QFileInfo(m_path).fileName();
			} else if (ch == 'p') {
				res += QFileInfo(m_path).absoluteFilePath();
			} else if (ch == 'v') {
				res += QCoreApplication::applicationVersion();
			} else {
				res += ch;
			}
		} else {
			res += format.at(i);
		}
	}

	return res;
}

bool NTagReaderGstreamer::isValid()
{
	return m_isValid;
}


/* vim: set ts=4 sw=4: */
