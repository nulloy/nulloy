/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2013 Sergey Vlasov <sergey@vlasov.me>
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

#include "waveformBuilderGstreamer.h"

#include <QFile>
#include <QDebug>
#include "core.h"

static QMutex _mutex;

static void _on_eos(GstBus *bus, GstMessage *msg, gpointer userData)
{
	Q_UNUSED(bus);
	Q_UNUSED(msg);

	NWaveformBuilderGstreamer *obj = reinterpret_cast<NWaveformBuilderGstreamer *>(userData);
	obj->peaks()->complete();
#if defined(QT_DEBUG) && !defined(QT_NO_DEBUG)
	qDebug() <<  "WaveformBuilder ::" << "completed" << obj->peaks()->size();
#endif
	obj->stop();
}

#if defined(QT_DEBUG) && !defined(QT_NO_DEBUG)
static void _on_error(GstBus *bus, GstMessage *msg, gpointer userData)
{
	Q_UNUSED(bus);
	Q_UNUSED(userData);

	gchar *debug;
	GError *err;

	gst_message_parse_error(msg, &err, &debug);
	g_free(debug);

	qWarning() << "WaveformBuilder :: error ::" << err->message;
	g_error_free(err);
}
#endif

static void _handleBuffer(GstPad *pad, GstBuffer *buffer, gpointer userData)
{
	QMutexLocker locker(&_mutex);

	Q_UNUSED(pad);

	int nChannels;
	GstStructure* structure = gst_caps_get_structure(GST_BUFFER_CAPS(buffer), 0);
	gst_structure_get_int(structure, "channels", &nChannels);

	gint16 *pcmBuffer = reinterpret_cast<gint16 *>(GST_BUFFER_DATA(buffer));
	int nSamples = (GST_BUFFER_SIZE(buffer) / sizeof(gint16)) / nChannels;

	NWaveformBuilderGstreamer *obj = reinterpret_cast<NWaveformBuilderGstreamer *>(userData);
	obj->handleBuffer(pcmBuffer, nChannels, nSamples);
}

void NWaveformBuilderGstreamer::handleBuffer(gint16 *pcmBuffer, int nChannels, int nSamples)
{
	for (int i = 0; i < nSamples; ++i) {
		qint32 pcmValue = 0;
		for (int j = 0; j < nChannels; ++j) {
			const qint16 *ptr = pcmBuffer + i * nChannels + j;
			pcmValue += *ptr;
		}
		qreal realValue = -((qreal)pcmValue / nChannels) / (1<<15);
		m_peaks.append(realValue);
	}
}

void NWaveformBuilderGstreamer::init()
{
	if (m_init)
		return;

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
	GError *err;
	NCore::cArgs(&argc, &argv);
	gst_init(&argc, (char ***)&argv);
	if (!gst_init_check(&argc, (char ***)&argv, &err))
		qCritical() << "WaveformBuilder :: error ::" << err->message;

	m_playbin = NULL;

#if defined Q_WS_WIN || defined Q_WS_MAC
	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
#endif

	reset();

	m_init = TRUE;
}

NWaveformBuilderGstreamer::~NWaveformBuilderGstreamer()
{
	if (!m_init)
		return;

	stop();
}

void NWaveformBuilderGstreamer::stop()
{
#if defined Q_WS_WIN || defined Q_WS_MAC
	m_timer->stop();
#endif

	if (m_playbin) {
		if (m_peaks.isCompleted())
			peaksAppendToCache(m_currentFile);

		gst_element_set_state(m_playbin, GST_STATE_NULL);
		gst_object_unref(m_playbin);
		m_playbin = NULL;
	}

	if (isRunning()) {
		quit();
		wait();
	}
}

void NWaveformBuilderGstreamer::start(const QString &file)
{
	stop();

	if (peaksFindFromCache(file))
		return;
	if (!QFileInfo(file).exists())
		return;
	m_currentFile = file;

	m_playbin = gst_parse_launch("uridecodebin name=w_uridecodebin \
								! audioconvert ! audio/x-raw-int, width=16, signed=true \
								! fakesink name=w_sink", NULL);

#if !defined Q_WS_WIN && !defined Q_WS_MAC
	GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_playbin));
	gst_bus_add_signal_watch(bus);
#if defined(QT_DEBUG) && !defined(QT_NO_DEBUG)
	g_signal_connect(bus, "message::error", G_CALLBACK(_on_error), this);
#endif
	g_signal_connect(bus, "message::eos", G_CALLBACK(_on_eos), this);
	gst_object_unref(bus);
#endif

	gchar *uri = g_filename_to_uri(QFileInfo(file).absoluteFilePath().toUtf8().constData(), NULL, NULL);
	GstElement *uridecodebin = gst_bin_get_by_name(GST_BIN(m_playbin), "w_uridecodebin");
	g_object_set(uridecodebin, "uri", uri, NULL);
	gst_object_unref(uridecodebin);

	GstElement *sink = gst_bin_get_by_name(GST_BIN(m_playbin), "w_sink");
	GstPad *pad = gst_element_get_pad(sink, "sink");
	gst_pad_add_buffer_probe(pad, G_CALLBACK(_handleBuffer), this);
	gst_object_unref(sink);
	gst_object_unref(pad);

	reset();
	QThread::start();

#if defined Q_WS_WIN || defined Q_WS_MAC
	if (!m_timer->isActive())
		m_timer->start(100);
#endif

	gst_element_set_state(m_playbin, GST_STATE_PLAYING);
}

qreal NWaveformBuilderGstreamer::position()
{
	if (!m_playbin)
		return 0;

	if (!isRunning())
		return 0;

	GstFormat format = GST_FORMAT_TIME;
	gint64 len, pos;
	gst_element_query_duration(m_playbin, &format, &len);
	gst_element_query_position(m_playbin, &format, &pos);

	if (format != GST_FORMAT_TIME)
		return 0;

	return (qreal)pos / len;
}

#if defined Q_WS_WIN || defined Q_WS_MAC
void NWaveformBuilderGstreamer::update()
{
	GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_playbin));
	GstMessage *msg = gst_bus_pop_filtered(bus, GstMessageType(GST_MESSAGE_EOS | GST_MESSAGE_ERROR));
	if (msg) {
		switch (GST_MESSAGE_TYPE(msg)) {
		case GST_MESSAGE_EOS:
			_on_eos(bus, msg, this);
			break;
		case GST_MESSAGE_ERROR:
#if defined(QT_DEBUG) && !defined(QT_NO_DEBUG)
			_on_error(bus, msg, this);
#endif
			break;
		default:
			break;
		}
		gst_message_unref(msg);
	}
	gst_object_unref(bus);
}
#endif

/* vim: set ts=4 sw=4: */
