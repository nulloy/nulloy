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

#include "waveformBuilderGstreamer.h"

#include "common.h"

#if defined Q_OS_WIN || defined Q_OS_MAC
#include <QTimer>
#endif

#include <QDebug>
#include <QFile>

static QMutex _mutex;

static void _handleBuffer(GstPad *pad, GstPadProbeInfo *info, gpointer userData)
{
    QMutexLocker locker(&_mutex);

    int nChannels;
    GstStructure *structure = gst_caps_get_structure(gst_pad_get_current_caps(pad), 0);
    gst_structure_get_int(structure, "channels", &nChannels);

    GstBuffer *buffer = GST_PAD_PROBE_INFO_BUFFER(info);
    GstMapInfo mapInfo;
    gst_buffer_map(buffer, &mapInfo, GST_MAP_READ);
    gint16 *pcmBuffer = (gint16 *)mapInfo.data;
    int nSamples = (mapInfo.size / sizeof(gint16)) / nChannels;

    NWaveformBuilderGstreamer *obj = reinterpret_cast<NWaveformBuilderGstreamer *>(userData);
    obj->handleBuffer(pcmBuffer, nChannels, nSamples);
    gst_buffer_unmap(buffer, &mapInfo);
}

void NWaveformBuilderGstreamer::handleBuffer(gint16 *pcmBuffer, int nChannels, int nSamples)
{
    for (int i = 0; i < nSamples; ++i) {
        qint32 pcmValue = 0;
        for (int j = 0; j < nChannels; ++j) {
            const qint16 *ptr = pcmBuffer + i * nChannels + j;
            pcmValue += *ptr;
        }
        qreal realValue = -((qreal)pcmValue / nChannels) / (1 << 15);
        m_peaks.append(realValue);
    }
}

void NWaveformBuilderGstreamer::init()
{
    if (m_init) {
        return;
    }

    int argc;
    const char **argv;
    GError *err = NULL;
    NCore::cArgs(&argc, &argv);
    gst_init(&argc, (char ***)&argv);
    if (!gst_init_check(&argc, (char ***)&argv, &err)) {
        qCritical() << "WaveformBuilder :: error ::" << QString::fromUtf8(err->message);
        if (err) {
            g_error_free(err);
        }
    }

    m_playbin = NULL;

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));

    reset();

    m_init = true;
}

NWaveformBuilderGstreamer::~NWaveformBuilderGstreamer()
{
    if (!m_init) {
        return;
    }

    stop();
}

void NWaveformBuilderGstreamer::stop()
{
    m_timer->stop();

    if (m_playbin) {
        if (m_peaks.isCompleted()) {
            peaksAppendToCache(m_currentFile);
        }

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

    if (peaksFindFromCache(file)) {
        return;
    }
    if (!QFileInfo(file).exists()) {
        return;
    }
    m_currentFile = file;

    m_playbin = gst_parse_launch("uridecodebin name=w_uridecodebin \
                                  ! audioconvert ! audio/x-raw, format=S16LE \
                                  ! fakesink name=w_sink",
                                 NULL);

    gchar *uri = g_filename_to_uri(QFileInfo(file).absoluteFilePath().toUtf8().constData(), NULL,
                                   NULL);
    GstElement *uridecodebin = gst_bin_get_by_name(GST_BIN(m_playbin), "w_uridecodebin");
    g_object_set(uridecodebin, "uri", uri, NULL);
    gst_object_unref(uridecodebin);

    GstElement *sink = gst_bin_get_by_name(GST_BIN(m_playbin), "w_sink");
    GstPad *pad = gst_element_get_static_pad(sink, "sink");
    gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, (GstPadProbeCallback)_handleBuffer, this,
                      NULL);
    gst_object_unref(sink);
    gst_object_unref(pad);

    reset();
    QThread::start();

    if (!m_timer->isActive()) {
        m_timer->start(100);
    }

    gst_element_set_state(m_playbin, GST_STATE_PLAYING);
}

qreal NWaveformBuilderGstreamer::position() const
{
    if (!m_playbin) {
        return 0;
    }

    if (!isRunning()) {
        return 0;
    }

    gint64 len, pos;
    gst_element_query_duration(m_playbin, GST_FORMAT_TIME, &len);
    gst_element_query_position(m_playbin, GST_FORMAT_TIME, &pos);

    return (qreal)pos / len;
}

void NWaveformBuilderGstreamer::update()
{
    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_playbin));
    GstMessage *msg = gst_bus_pop_filtered(bus, GstMessageType(GST_MESSAGE_EOS | GST_MESSAGE_ERROR));
    if (msg) {
        switch (GST_MESSAGE_TYPE(msg)) {
            case GST_MESSAGE_EOS:
                peaks()->complete();
                qDebug() << "WaveformBuilder ::"
                         << "completed" << peaks()->size();
                stop();
                break;
            case GST_MESSAGE_ERROR: {
                gchar *debug;
                GError *err = NULL;

                gst_message_parse_error(msg, &err, &debug);
                g_free(debug);

                qWarning() << "WaveformBuilder :: error ::" << QString::fromUtf8(err->message);
                if (err) {
                    g_error_free(err);
                }
                break;
            }
            default:
                break;
        }
        gst_message_unref(msg);
    }
    gst_object_unref(bus);
}
