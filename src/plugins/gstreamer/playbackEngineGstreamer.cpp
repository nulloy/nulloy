/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2014 Sergey Vlasov <sergey@vlasov.me>
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

#include "common.h"
#include <QtGlobal>
#include <QTimer>

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

N::PlaybackState NPlaybackEngineGStreamer::fromGstState(GstState state)
{
	switch (state) {
		case GST_STATE_PAUSED:
			return N::PlaybackPaused;
		case GST_STATE_PLAYING:
			return N::PlaybackPlaying;
		default:
			return N::PlaybackStopped;
	}
}

void NPlaybackEngineGStreamer::init()
{
	if (m_init)
		return;

	int argc;
	const char **argv;
	GError *err;
	NCore::cArgs(&argc, &argv);
	gst_init(&argc, (char ***)&argv);
	if (!gst_init_check(&argc, (char ***)&argv, &err)) {
		emit message(QMessageBox::Critical, QFileInfo(m_currentMedia).absoluteFilePath(), err->message);
		emit failed();
	}

	m_playbin = gst_element_factory_make("playbin2", NULL);

#if !defined Q_WS_WIN && !defined Q_WS_MAC
	GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_playbin));
	gst_bus_add_signal_watch(bus);
	g_signal_connect(bus, "message::error", G_CALLBACK(_on_error), this);
	g_signal_connect(bus, "message::eos", G_CALLBACK(_on_eos), this);
	gst_object_unref(bus);
#endif

	m_oldVolume = -1;
	m_oldPosition = -1;
	m_posponedPosition = -1;
	m_oldState = N::PlaybackStopped;
	m_currentMedia = "";
	m_durationNsec = 0;

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
}

void NPlaybackEngineGStreamer::setMedia(const QString &file)
{
#if defined Q_WS_WIN || defined Q_WS_MAC
	qreal vol = m_oldVolume;
#endif

	stop();

	if (file.isEmpty()) {
		m_currentMedia = "";
		emit mediaChanged("");
		return;
	}

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

#if defined Q_WS_WIN || defined Q_WS_MAC
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

	if (m_durationNsec > 0) {
		gst_element_seek(m_playbin, 1.0,
		                 GST_FORMAT_TIME,
		                 GstSeekFlags(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT),
		                 GST_SEEK_TYPE_SET, pos * m_durationNsec,
		                 GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
	} else {
		m_posponedPosition = pos;
	}
}

qreal NPlaybackEngineGStreamer::position()
{
	if (!hasMedia() || m_durationNsec <= 0)
		return -1;

	GstFormat format = GST_FORMAT_TIME;
	gint64 pos;
	gboolean res = gst_element_query_position(m_playbin, &format, &pos);
	if (!res || format != GST_FORMAT_TIME)
		return 0;

	emit tick(pos / 1000000);

	return (qreal)pos / m_durationNsec;
}

qint64 NPlaybackEngineGStreamer::durationMsec()
{
	return m_durationNsec / 1000000;
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
	GstState gstState;
	gst_element_get_state(m_playbin, &gstState, NULL, 0);
	N::PlaybackState state = fromGstState(gstState);
	if (m_oldState != state) {
		emit stateChanged(state);
		m_oldState = state;
	}

	if (state == N::PlaybackPlaying || state == N::PlaybackPaused) {
		// duration may change for some reason
		// TODO use DURATION_CHANGED in gstreamer1.0
		GstFormat format = GST_FORMAT_TIME;
		gboolean res = gst_element_query_duration(m_playbin, &format, &m_durationNsec);
		if (!res || format != GST_FORMAT_TIME)
			m_durationNsec = 0;
	}

	if (m_posponedPosition >= 0 && m_durationNsec > 0) {
		setPosition(m_posponedPosition);
		m_posponedPosition = -1;
		emit positionChanged(m_posponedPosition);
	} else {
		qreal pos = position();
		if (m_oldPosition != pos) {
			m_oldPosition = pos;
			emit positionChanged(pos);
		}
	}

#if defined Q_WS_WIN || defined Q_WS_MAC
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
	if (qAbs(m_oldVolume - vol) > 0.0001) {
		m_oldVolume = vol;
		emit volumeChanged(vol);
	}
}

void NPlaybackEngineGStreamer::_emitFinished()
{
	stop();
	emit finished();
	emit stateChanged(N::PlaybackStopped);
	m_oldState = N::PlaybackStopped;
}

void NPlaybackEngineGStreamer::_emitFailed()
{
	stop();
	emit failed();
	emit stateChanged(N::PlaybackStopped);
	m_oldState = N::PlaybackStopped;
}

void NPlaybackEngineGStreamer::_emitError(QString error)
{
	emit message(QMessageBox::Critical, QFileInfo(m_currentMedia).absoluteFilePath(), error);
}

