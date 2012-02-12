/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2011 Sergey Vlasov <sergey@vlasov.me>
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

#include "tagReader.h"

#include "core.h"
#include <gst/gst.h>

#include <QCoreApplication>
#include <QFileInfo>

#include <QDebug>

class NTagReaderPrivate
{
public:
	NTagReaderPrivate() {}
	~NTagReaderPrivate() {}

	QString m_path;
	GstElement *m_playbin;
	GstTagList *m_taglist;
	gint64 m_nanosecs;
	bool m_isValid;
};

NTagReader::NTagReader(const QString &file) : d_ptr(new NTagReaderPrivate())
{
	Q_D(NTagReader);

	d->m_path = file;
	d->m_isValid = FALSE;

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
		g_error("NTagReader::error: %s", init_err->message);
		return;
	}

	d->m_playbin = gst_element_factory_make("playbin", NULL);

	gchar *uri = g_filename_to_uri(QFileInfo(file).absoluteFilePath().toUtf8().constData(), NULL, NULL);

	g_object_set(d->m_playbin, "uri", uri, NULL);
	gst_element_set_state(d->m_playbin, GST_STATE_PAUSED);

	d->m_taglist = NULL;

	GstMessage *msg;
	while (TRUE) {
		msg = gst_bus_timed_pop_filtered(GST_ELEMENT_BUS(d->m_playbin), GST_CLOCK_TIME_NONE,
										(GstMessageType)(GST_MESSAGE_ASYNC_DONE | GST_MESSAGE_TAG | GST_MESSAGE_ERROR));

		if (GST_MESSAGE_TYPE(msg) != GST_MESSAGE_TAG)
			break;

		GstTagList *tags = NULL;
		gst_message_parse_tag(msg, &tags);

		if (gst_is_tag_list(tags)) {
			d->m_taglist = gst_tag_list_merge(d->m_taglist, tags, GST_TAG_MERGE_KEEP);
			gst_tag_list_free(tags);
		}
		gst_message_unref(msg);
	};

	if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
		gchar *debug;
		GError *err;

		gst_message_parse_error(msg, &err, &debug);
		g_free(debug);
		qWarning() << "NTagReader :: parse error ::" << err->message;
	}

	if (gst_is_tag_list(d->m_taglist))
		d->m_isValid = TRUE;

	if (d->m_isValid) {
		GstFormat format = GST_FORMAT_TIME;
		d->m_nanosecs = 0;
		gst_element_query_duration(d->m_playbin, &format, &d->m_nanosecs);
	}

	gst_message_unref(msg);
	gst_element_set_state(d->m_playbin, GST_STATE_NULL);
	gst_object_unref(d->m_playbin);
}

NTagReader::~NTagReader()
{
	Q_D(NTagReader);
	gst_tag_list_free(d->m_taglist);
}

QString NTagReader::toString(const QString &format)
{
	Q_D(NTagReader);

	if (format.isEmpty())
		return "";

	if (!d->m_isValid)
		return "NTagReader::InvalidFile";

	int seconds_total = GST_TIME_AS_SECONDS(d->m_nanosecs);

	QString res;
	for (int i = 0; i < format.size(); ++i) {
		if (format.at(i) == '%') {
			++i;
			QChar ch = format.at(i);
			if (ch == 'a') {
				gchar *gstr;
				bool exists = gst_tag_list_get_string(d->m_taglist, "artist", &gstr);
				QString str(gstr);
				if (str.isEmpty() || !exists)
					str = "<Unknown artist>";
				res += str;
			} else if (ch == 't') {
				gchar *gstr;
				bool exists = gst_tag_list_get_string(d->m_taglist, "title", &gstr);
				QString str(gstr);
				if (str.isEmpty() || !exists)
					str = QFileInfo(d->m_path).baseName();
				res += str;
			} else if (ch == 'A') {
				gchar *gstr;
				bool exists = gst_tag_list_get_string(d->m_taglist, "album", &gstr);
				QString str(gstr);
				if (str.isEmpty() || !exists)
					str = "<Unknown album>";
				res += str;
			} else if (ch == 'c') {
				gchar *gstr;
				bool exists = gst_tag_list_get_string(d->m_taglist, "comment", &gstr);
				QString str(gstr);
				if (str.isEmpty() || !exists)
					str = "<Empty comment>";
				res += str;
			} else if (ch == 'g') {
				gchar *gstr;
				bool exists = gst_tag_list_get_string(d->m_taglist, "genre", &gstr);
				QString str(gstr);
				if (str.isEmpty() || !exists)
					str = "<Unknown genre>";
				res += str;
			} else if (ch == 'y') {
				GDate *date = NULL;
				QString str = "0";
				bool exists = gst_tag_list_get_date(d->m_taglist, "date", &date);
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
				bool exists = gst_tag_list_get_uint(d->m_taglist, "track-number", &track);
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
				bool exists = gst_tag_list_get_uint(d->m_taglist, "bitrate", &bitrate);
				if (!bitrate || !exists)
					res += "<Unknown bitrate>";
				else
					res += QString::number(bitrate / 1000);
			} else if (ch == 's') {
				res += "<Usupported tag: samplerate>";
			} else if (ch == 'C') {
				res += "<Usupported tag: channels number>";
			} else if (ch == 'f') {
				res += QFileInfo(d->m_path).baseName();
			} else if (ch == 'F') {
				res += QFileInfo(d->m_path).fileName();
			} else if (ch == 'p') {
				res += QFileInfo(d->m_path).absoluteFilePath();
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

bool NTagReader::isValid()
{
	Q_D(NTagReader);
	return d->m_isValid;
}


/* vim: set ts=4 sw=4: */
