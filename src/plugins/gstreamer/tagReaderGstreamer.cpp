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
#include <QFileInfo>

void NTagReaderGstreamer::init()
{
	if (m_init)
		return;

	m_isValid = FALSE;
	m_taglist = NULL;
	m_playbin = NULL;

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
	if (m_taglist) {
		gst_tag_list_free(m_taglist);
		m_taglist = NULL;
	}

	m_path = file;

	m_playbin = gst_element_factory_make("playbin", NULL);

	gchar *uri = g_filename_to_uri(QFileInfo(file).absoluteFilePath().toUtf8().constData(), NULL, NULL);

	g_object_set(m_playbin, "uri", uri, NULL);
	gst_element_set_state(m_playbin, GST_STATE_PAUSED);

	GstMessage *msg;
	while (TRUE) {
		msg = gst_bus_timed_pop_filtered(GST_ELEMENT_BUS(m_playbin), GST_CLOCK_TIME_NONE,
										(GstMessageType)(GST_MESSAGE_ASYNC_DONE | GST_MESSAGE_TAG | GST_MESSAGE_ERROR));

		if (GST_MESSAGE_TYPE(msg) != GST_MESSAGE_TAG)
			break;

		GstTagList *tags = NULL;
		gst_message_parse_tag(msg, &tags);

		if (gst_is_tag_list(tags)) {
			m_taglist = gst_tag_list_merge(m_taglist, tags, GST_TAG_MERGE_KEEP);
			gst_tag_list_free(tags);
		}
		gst_message_unref(msg);
	};

	if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
		gchar *debug;
		GError *err;

		gst_message_parse_error(msg, &err, &debug);
		g_free(debug);
		qWarning() << "NTagReaderGstreamer :: parse error ::" << err->message;
	}

	if (gst_is_tag_list(m_taglist))
		m_isValid = TRUE;

	if (m_isValid) {
		GstFormat format = GST_FORMAT_TIME;
		m_nanosecs = 0;
		gst_element_query_duration(m_playbin, &format, &m_nanosecs);
	}

	gst_message_unref(msg);
	gst_element_set_state(m_playbin, GST_STATE_NULL);
	gst_object_unref(m_playbin);
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
		if (format.at(i) == '%') {
			++i;
			QChar ch = format.at(i);
			if (ch == 'a') {
				gchar *gstr;
				bool exists = gst_tag_list_get_string(m_taglist, "artist", &gstr);
				QString str(gstr);
				if (str.isEmpty() || !exists)
					str = "<Unknown artist>";
				res += str;
			} else if (ch == 't') {
				gchar *gstr;
				bool exists = gst_tag_list_get_string(m_taglist, "title", &gstr);
				QString str(gstr);
				if (str.isEmpty() || !exists)
					str = QFileInfo(m_path).baseName();
				res += str;
			} else if (ch == 'A') {
				gchar *gstr;
				bool exists = gst_tag_list_get_string(m_taglist, "album", &gstr);
				QString str(gstr);
				if (str.isEmpty() || !exists)
					str = "<Unknown album>";
				res += str;
			} else if (ch == 'c') {
				gchar *gstr;
				bool exists = gst_tag_list_get_string(m_taglist, "comment", &gstr);
				QString str(gstr);
				if (str.isEmpty() || !exists)
					str = "<Empty comment>";
				res += str;
			} else if (ch == 'g') {
				gchar *gstr;
				bool exists = gst_tag_list_get_string(m_taglist, "genre", &gstr);
				QString str(gstr);
				if (str.isEmpty() || !exists)
					str = "<Unknown genre>";
				res += str;
			} else if (ch == 'y') {
				GDate *date = NULL;
				QString str = "0";
				bool exists = gst_tag_list_get_date(m_taglist, "date", &date);
				if (exists) {
					GDateYear year = g_date_get_year(date);
					if (year != G_DATE_BAD_YEAR)
						str = QString::number(year);
				}
				if (str == "0")
					str = "<Unknown year>";
				res += str;
			} else if (ch == 'n') {
				unsigned int track = 0;
				bool exists = gst_tag_list_get_uint(m_taglist, "track-number", &track);
				if (!track || !exists)
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
				bool exists = gst_tag_list_get_uint(m_taglist, "bitrate", &bitrate);
				if (!bitrate || !exists)
					res += "<Unknown bitrate>";
				else
					res += QString::number(bitrate / 1000);
			} else if (ch == 's') {
				res += "<Usupported tag: samplerate>";
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
