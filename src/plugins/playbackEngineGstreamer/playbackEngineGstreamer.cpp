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

#include "playbackEngineGstreamer.h"
#include <QtGlobal>
#include "core.h"

static void _on_eos(GstBus *bus, GstMessage *msg, gpointer userData)
{
	Q_UNUSED(bus);
	Q_UNUSED(msg);

	NPlaybackEngineGStreamer *obj = reinterpret_cast<NPlaybackEngineGStreamer *>(userData);
	obj->_emitFinished();
}

static void _on_error(GstBus *bus, GstMessage *msg, gpointer userData)
{
	Q_UNUSED(bus);

	gchar *debug;
	GError *err;

	gst_message_parse_error(msg, &err, &debug);
	g_free(debug);

	NPlaybackEngineGStreamer *obj = reinterpret_cast<NPlaybackEngineGStreamer *>(userData);
	obj->_emitError(err->message);
	obj->_emitFailed();

	g_error_free(err);
}

static NPlaybackEngineInterface::State fromGstState(GstState state)
{
	switch (state) {
		case GST_STATE_PAUSED:
			return NPlaybackEngineInterface::Paused;
		case GST_STATE_PLAYING:
			return NPlaybackEngineInterface::Playing;
		default:
			return NPlaybackEngineInterface::Stopped;
	}
}

void NPlaybackEngineGStreamer::init()
{
	if (m_init)
		return;

#if defined WIN32 || defined _WINDOWS || defined Q_WS_WIN
	_putenv("GST_PLUGIN_PATH=Plugins\\GStreamer");
#endif

	int argc;
	const char **argv;
	NCore::cArgs(&argc, &argv);
	gst_init(&argc, (char ***)&argv);

	m_playbin = gst_element_factory_make("playbin2", NULL);

#if !defined WIN32 && !defined _WINDOWS && !defined Q_WS_WIN
	GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_playbin));
	gst_bus_add_signal_watch(bus);
	g_signal_connect(bus, "message::error", G_CALLBACK(_on_error), this);
	g_signal_connect(bus, "message::eos", G_CALLBACK(_on_eos), this);
	gst_object_unref(bus);
#endif

	m_oldVolume = -1;
	m_oldPosition = -1;
	m_savedPosition = -1;
	m_oldState = Stopped;
	m_currentMedia = "";

	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(checkStatus()));
	m_timer->start(100);

	m_init = TRUE;
}

NPlaybackEngineGStreamer::~NPlaybackEngineGStreamer()
{
	if (!m_init)
		return;

	stop();
	gst_object_unref(m_playbin);
	gst_deinit();
}

void NPlaybackEngineGStreamer::setMedia(const QString &file)
{
#if defined WIN32 || defined _WINDOWS || defined Q_WS_WIN
	qreal vol = m_oldVolume;
#endif

	stop();

	if (file.isEmpty())
		return;

	if (!QFile(file).exists()) {
		emit message(QMessageBox::Warning, file, "No such file or directory");
		emit mediaChanged("");
		emit _emitFailed();
		return;
	}

	gchar *uri = g_filename_to_uri(QFileInfo(file).absoluteFilePath().toUtf8().constData(), NULL, NULL);
	if (uri)
		m_currentMedia = file;
	g_object_set(m_playbin, "uri", uri, NULL);

	emit mediaChanged(file);

#if defined WIN32 || defined _WINDOWS || defined Q_WS_WIN
	if (vol != -1)
		setVolume(vol);
#endif
}

void NPlaybackEngineGStreamer::setVolume(qreal volume)
{
	g_object_set(m_playbin, "volume", qBound(0.0, volume, 1.0), NULL);
}

qreal NPlaybackEngineGStreamer::volume()
{
	gdouble volume;
	g_object_get(m_playbin, "volume", &volume, NULL);
	return (qreal)volume;
}

void NPlaybackEngineGStreamer::setPosition(qreal pos)
{
	if (!hasMedia() || pos < 0)
		return;

	GstFormat format = GST_FORMAT_TIME;
	gint64 len;
	if (gst_element_query_duration(m_playbin, &format, &len)) {
		gst_element_seek(m_playbin, 1.0,
						GST_FORMAT_TIME,
						GstSeekFlags(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT),
						GST_SEEK_TYPE_SET, pos * len,
						GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
	} else {
		m_savedPosition = pos;
	}
}

qreal NPlaybackEngineGStreamer::position()
{
	if (!hasMedia())
		return -1;

	GstFormat format = GST_FORMAT_TIME;
	gint64 len, pos;

	gboolean res;

	res = gst_element_query_duration(m_playbin, &format, &len);
	if (!res)
		return 0;

	res = gst_element_query_position(m_playbin, &format, &pos);
	if (!res)
		return 0;

	if (format != GST_FORMAT_TIME)
		return 0;

	return (qreal)pos / len;
}

void NPlaybackEngineGStreamer::play()
{
	if (!hasMedia())
		return;

	GstState state;
	gst_element_get_state(m_playbin, &state, NULL, 0);
	if (state != GST_STATE_PLAYING)
		gst_element_set_state(m_playbin, GST_STATE_PLAYING);
	else
		gst_element_set_state(m_playbin, GST_STATE_PAUSED);
}

void NPlaybackEngineGStreamer::pause()
{
	if (!hasMedia())
		return;

	gst_element_set_state(m_playbin, GST_STATE_PAUSED);
}

void NPlaybackEngineGStreamer::stop()
{
	if (!hasMedia())
		return;

	gst_element_set_state(m_playbin, GST_STATE_NULL);
}

bool NPlaybackEngineGStreamer::hasMedia()
{
	return !m_currentMedia.isEmpty();
}

QString NPlaybackEngineGStreamer::currentMedia()
{
	return m_currentMedia;
}

void NPlaybackEngineGStreamer::checkStatus()
{
	if (m_savedPosition >= 0) {
		GstFormat format = GST_FORMAT_TIME;
		gint64 len;
		if (gst_element_query_duration(m_playbin, &format, &len)) {
			setPosition(m_savedPosition);
			m_savedPosition = -1;
			emit positionChanged(m_savedPosition);
		}
	} else {
		qreal pos = position();
		if (m_oldPosition != pos) {
			m_oldPosition = pos;
			emit positionChanged(pos);
		}
	}

#if defined WIN32 || defined _WINDOWS || defined Q_WS_WIN
	GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_playbin));
	GstMessage *msg = gst_bus_pop_filtered(bus, GstMessageType(GST_MESSAGE_EOS | GST_MESSAGE_ERROR));
	if (msg) {
		switch (GST_MESSAGE_TYPE(msg)) {
		case GST_MESSAGE_EOS:
			_on_eos(bus, msg, this);
			break;
		case GST_MESSAGE_ERROR:
			_on_error(bus, msg, this);
			break;
		default:
			break;
		}
		gst_message_unref(msg);
	}
	gst_object_unref(bus);
#endif

	qreal vol = volume();
	if (m_oldVolume != vol) {
		m_oldVolume = vol;
		emit volumeChanged(vol);
	}

	GstState gstState;
	gst_element_get_state(m_playbin, &gstState, NULL, 0);
	State state = fromGstState(gstState);
	if (m_oldState != state) {
		emit stateChanged(state);
		m_oldState = state;
	}
}

void NPlaybackEngineGStreamer::_emitFinished()
{
	stop();
	emit finished();
	emit stateChanged(Stopped);
}

void NPlaybackEngineGStreamer::_emitFailed()
{
	stop();
	emit failed();
	emit stateChanged(Stopped);
}

void NPlaybackEngineGStreamer::_emitError(QString error)
{
	emit message(QMessageBox::Critical, QFileInfo(m_currentMedia).absoluteFilePath(), error);
}

#if !defined _N_GSTREAMER_PLUGINS_BUILTIN_ && !defined _N_NO_PLUGINS_
Q_EXPORT_PLUGIN2(playback_gstreamer, NPlaybackEngineGStreamer)
#endif

/* vim: set ts=4 sw=4: */
