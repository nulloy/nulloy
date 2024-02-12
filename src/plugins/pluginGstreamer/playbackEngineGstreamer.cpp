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

#include "playbackEngineGstreamer.h"

#include <QTimer>
#include <QtGlobal>
#include <gst/pbutils/missing-plugins.h>

#include "common.h"

#define NSEC_IN_MSEC 1000000
#define CROSSFADING_MIN_DURATION_MSEC 1000 // 1 second
#define SHORT_TICK_THRESHOLD_MSEC 30000    // 30 seconds
#define SHORT_TICK_MSEC 20
#define LONG_TICK_MSEC 100
#define STATE_CHANGE_DEBOUNCE_MSEC 50
#define GST_BUS_POP_MSEC 50

static void _on_about_to_finish(GstElement *, gpointer userData)
{
    NPlaybackEngineGStreamer *obj = reinterpret_cast<NPlaybackEngineGStreamer *>(userData);
    if ((obj->durationMsec() < CROSSFADING_MIN_DURATION_MSEC) || obj->_nextMediaRequestBlocked()) {
        return;
    }
    obj->_emitNextMediaRequest();
}

N::PlaybackState NPlaybackEngineGStreamer::fromGstState(GstState state) const
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
    if (m_init) {
        return;
    }

    int argc;
    const char **argv;
    GError *err = NULL;
    NCore::cArgs(&argc, &argv);
    gst_init(&argc, (char ***)&argv);
    if (!gst_init_check(&argc, (char ***)&argv, &err)) {
        emit message(N::Critical, tr("Playback error"),
                     err ? QString::fromUtf8(err->message) : tr("Unknown error"));
        if (err) {
            g_error_free(err);
        }
    }
    m_playbin = gst_element_factory_make("playbin", NULL);
    g_signal_connect(m_playbin, "about-to-finish", G_CALLBACK(_on_about_to_finish), this);
    gst_element_add_property_notify_watch(m_playbin, "volume", TRUE);

    // FIXME: https://gitlab.freedesktop.org/gstreamer/gst-plugins-bad/-/issues/1798
    //m_pitchElement = gst_element_factory_make("pitch", NULL);
    m_pitchElement = NULL;
    if (!m_pitchElement) {
        //emit message(N::Critical, "Playback Engine", "Failed to create pitch element");
    } else {
        GstElement *sink = gst_element_factory_make("autoaudiosink", NULL);
        GstElement *bin = gst_bin_new(NULL);
        gst_bin_add_many(GST_BIN(bin), m_pitchElement, sink, NULL);
        gst_element_link_many(m_pitchElement, sink, NULL);

        GstPad *pad = gst_element_get_static_pad(m_pitchElement, "sink");
        gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad));
        gst_object_unref(pad);

        g_object_set(m_playbin, "audio-sink", bin, NULL);
    }

#ifdef _TESTS_
    GstElement *sink = gst_element_factory_make("fakesink", NULL);
    g_object_set(sink, "sync", TRUE, NULL);
    g_object_set(m_playbin, "audio-sink", sink, NULL);

    sink = gst_element_factory_make("fakesink", NULL);
    g_object_set(sink, "sync", TRUE, NULL);
    g_object_set(m_playbin, "video-sink", sink, NULL);
#endif

    m_speed = 1.0;
    m_speedPostponed = false;
    m_pitch = 1.0;
    m_volume = -1.0;
    m_position = 0.0;
    m_gstState = GST_STATE_NULL;
    m_positionPostponed = false;
    m_currentMedia = "";
    m_currentContext = 0;
    m_bkpMedia = "";
    m_bkpContext = 0;
    m_durationNsec = GST_CLOCK_TIME_NONE;
    m_crossfading = false;
    m_nextMediaRequestBlock = false;

    m_checkStatusTimer = new QTimer(this);
    connect(m_checkStatusTimer, SIGNAL(timeout()), this, SLOT(checkStatus()));

    m_emitStateTimer = new QTimer(this);
    m_emitStateTimer->setSingleShot(true);
    m_emitStateTimer->setInterval(STATE_CHANGE_DEBOUNCE_MSEC);
    connect(m_emitStateTimer, &QTimer::timeout,
            [this]() { emit stateChanged(fromGstState(m_gstState)); });

    m_gstBusPopTimer = new QTimer(this);
    m_gstBusPopTimer->setInterval(GST_BUS_POP_MSEC);
    connect(m_gstBusPopTimer, &QTimer::timeout, [this]() {
        GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_playbin));
        GstMessage *msg;
        while ((msg = gst_bus_pop(bus)) != NULL) {
            processGstMessage(msg);
        }
    });

    m_init = true;
}

NPlaybackEngineGStreamer::~NPlaybackEngineGStreamer()
{
    if (!m_init) {
        return;
    }

    stop();
    gst_object_unref(m_playbin);
}

bool NPlaybackEngineGStreamer::gstSetFile(const QString &file, int context, bool prepareNext)
{
    if (file.isEmpty()) {
        stop();
        m_currentMedia = "";
        m_currentContext = 0;
        emit mediaChanged(m_currentMedia, m_currentContext);
        return false;
    }

    if (!QFile(file).exists()) {
        emit message(N::Warning, file, tr("No such file or directory"));
        fail();
        return false;
    }

    GError *err = NULL;
    gchar *uri = g_filename_to_uri(QFileInfo(file).absoluteFilePath().toUtf8().constData(), NULL,
                                   &err);
    if (uri) {
        m_currentMedia = file;
        m_currentContext = context;
        if (!prepareNext) {
            gst_element_set_state(m_playbin, GST_STATE_NULL);
        }
        g_object_set(m_playbin, "uri", uri, NULL);
        g_free(uri);
    } else {
        emit message(N::Critical, file, err ? QString::fromUtf8(err->message) : tr("Invalid path"));
        if (err) {
            g_error_free(err);
        }
        fail();
        return false;
    }
    return true;
}

void NPlaybackEngineGStreamer::setMedia(const QString &file, int context)
{
    m_crossfading = false;
    m_position = 0.0;
    m_nextMediaRequestBlock = true;

    if (!gstSetFile(file, context, false)) {
        return;
    }
}

void NPlaybackEngineGStreamer::nextMediaRespond(const QString &file, int context)
{
    if (!m_crossfading) {
        return;
    }
    m_bkpMedia = m_currentMedia;
    m_bkpContext = m_currentContext;

    gstSetFile(file, context, true);
}

qreal NPlaybackEngineGStreamer::speed() const
{
    return m_speed;
}

void NPlaybackEngineGStreamer::setSpeed(qreal speed)
{
    m_speed = speed;
    m_speedPostponed = true;
}

qreal NPlaybackEngineGStreamer::pitch() const
{
    return m_pitch;
}

void NPlaybackEngineGStreamer::setPitch(qreal pitch)
{
    if (!m_pitchElement) {
        return;
    }
    m_pitch = pitch;
    g_object_set(m_pitchElement, "pitch", m_pitch, NULL);
}

void NPlaybackEngineGStreamer::setVolume(qreal volume)
{
    m_volume = qBound(0.0, volume, 1.0);
    g_object_set(m_playbin, "volume", m_volume, NULL);
}

qreal NPlaybackEngineGStreamer::volume() const
{
    return m_volume;
}

void NPlaybackEngineGStreamer::setPosition(qreal pos)
{
    if (!hasMedia() || pos < 0.0 || pos > 1.0) {
        return;
    }

    if (m_crossfading) {
        // abort cross-fading:
        if (!gstSetFile(m_bkpMedia, m_bkpContext, false)) {
            fail();
            return;
        }
    }
    m_position = pos;
    m_positionPostponed = true;
}

void NPlaybackEngineGStreamer::jump(qint64 msec)
{
    if (!hasMedia()) {
        return;
    }

    if (m_crossfading) {
        // abort cross-fading:
        if (!gstSetFile(m_bkpMedia, m_bkpContext, false)) {
            fail();
            return;
        }
    }
    m_position += ((qreal)msec * NSEC_IN_MSEC) / m_durationNsec;
    m_positionPostponed = true;
}

qreal NPlaybackEngineGStreamer::position() const
{
    return m_position;
}

qint64 NPlaybackEngineGStreamer::durationMsec() const
{
    return m_durationNsec / NSEC_IN_MSEC;
}

void NPlaybackEngineGStreamer::play()
{
    if (!hasMedia()) {
        return;
    }

    m_gstBusPopTimer->start();
    m_checkStatusTimer->start(LONG_TICK_MSEC);
    gst_element_set_state(m_playbin, GST_STATE_PLAYING);
}

void NPlaybackEngineGStreamer::pause()
{
    if (!hasMedia()) {
        return;
    }

    gst_element_set_state(m_playbin, GST_STATE_PAUSED);

    m_checkStatusTimer->stop();
    m_gstBusPopTimer->stop();

    m_gstState = GST_STATE_PAUSED;
    emit stateChanged(fromGstState(m_gstState));

    checkStatus();
}

void NPlaybackEngineGStreamer::stop()
{
    m_crossfading = false;
    m_nextMediaRequestBlock = true;
    gst_element_set_state(m_playbin, GST_STATE_NULL);
    m_durationNsec = 0;
    m_position = 0.0;

    emit stateChanged(N::PlaybackStopped);
    emit positionChanged(m_position);

    m_checkStatusTimer->stop();
    m_gstBusPopTimer->stop();
}

bool NPlaybackEngineGStreamer::hasMedia() const
{
    return !m_currentMedia.isEmpty();
}

QString NPlaybackEngineGStreamer::currentMedia() const
{
    return m_currentMedia;
}

N::PlaybackState NPlaybackEngineGStreamer::state() const
{
    return fromGstState(m_gstState);
}

void NPlaybackEngineGStreamer::processGstMessage(GstMessage *msg)
{
    //qDebug() << "message type:" << GST_MESSAGE_TYPE_NAME(msg);
    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS: {
            stop();
            emit mediaFinished(m_currentMedia, m_currentContext);
            break;
        }
        case GST_MESSAGE_ERROR: {
            gchar *debug;
            GError *err = NULL;
            gst_message_parse_error(msg, &err, &debug);
            g_free(debug);

            emit message(N::Critical, QFileInfo(m_currentMedia).absoluteFilePath(),
                         err ? QString::fromUtf8(err->message) : tr("Unknown error"));
            fail();
            if (err) {
                g_error_free(err);
            }
            break;
        }
        case GST_MESSAGE_ELEMENT: {
            if (gst_is_missing_plugin_message(msg)) {
                QString str = tr("Missing GStreamer plugin:<br/>");
                gchar *detail = gst_missing_plugin_message_get_installer_detail(msg);
                if (detail) {
                    QStringList fields = QString::fromUtf8(detail).split('|').mid(3);
                    str += QString::fromUtf8(detail).split('|').mid(3).join("<br/>");
                    g_free(detail);
                } else {
                    str += tr("Unknown plugin");
                }
                emit message(N::Critical, QFileInfo(m_currentMedia).absoluteFilePath(), str);
                fail();
            }
            break;
        }
        case GST_MESSAGE_DURATION_CHANGED: {
            m_durationNsec = GST_CLOCK_TIME_NONE;
            break;
        }
        case GST_MESSAGE_STREAM_START: {
            m_crossfading = false;
            m_nextMediaRequestBlock = false;
            if (m_speed != 1.0) {
                m_speedPostponed = true;
            }
            m_durationNsec = GST_CLOCK_TIME_NONE;
            emit mediaChanged(m_currentMedia, m_currentContext);
            break;
        }
        case GST_MESSAGE_PROPERTY_NOTIFY: {
            const gchar *name;
            const GValue *value;
            gst_message_parse_property_notify(msg, NULL, &name, &value);
            if (QString(name) == "volume") {
                gdouble gstVolume = g_value_get_double(value);
                if (gstVolume != m_volume) {
                    m_volume = gstVolume;
                    emit volumeChanged(m_volume);
                }
            }
            break;
        }
        case GST_MESSAGE_STATE_CHANGED: {
            if (GST_MESSAGE_SRC(msg) == GST_OBJECT(m_playbin)) {
                GstState newState, oldState, pendingState;
                gst_message_parse_state_changed(msg, &oldState, &newState, &pendingState);
                //qDebug() << "state changed old:" << gst_element_state_get_name(oldState)
                //         << " new:" << gst_element_state_get_name(newState)
                //         << " pending:" << gst_element_state_get_name(pendingState);
                if (newState != m_gstState) {
                    m_gstState = newState;
                    m_emitStateTimer->start();
                }
            }
            break;
        }
        default:
            break;
    }
}

void NPlaybackEngineGStreamer::checkStatus()
{
    if (!GST_CLOCK_TIME_IS_VALID(m_durationNsec)) {
        gst_element_query_duration(m_playbin, GST_FORMAT_TIME, &m_durationNsec);
    }

    if (GST_CLOCK_TIME_IS_VALID(m_durationNsec)) {
        gint64 gstPos = 0;
        if (gst_element_query_position(m_playbin, GST_FORMAT_TIME, &gstPos)) {
            if (!m_positionPostponed) {
                m_position = (qreal)gstPos / m_durationNsec;
                emit positionChanged(m_position);
            }
            emit tick(gstPos / NSEC_IN_MSEC * m_speed);
        }

        if (m_positionPostponed || m_speedPostponed) {
            if (m_positionPostponed) {
                gstPos = m_position * m_durationNsec;
            }
            gst_element_seek(m_playbin, m_speed, GST_FORMAT_TIME,
                             GstSeekFlags(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT |
                                          GST_SEEK_FLAG_SNAP_NEAREST),
                             GST_SEEK_TYPE_SET, gstPos, GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
            m_positionPostponed = false;
            m_speedPostponed = false;
        }

        if (m_durationNsec / NSEC_IN_MSEC <= SHORT_TICK_THRESHOLD_MSEC) {
            m_checkStatusTimer->setInterval(SHORT_TICK_MSEC);
        }
    }
}

void NPlaybackEngineGStreamer::fail()
{
    stop();

    emit mediaFailed(m_currentMedia, m_currentContext);

    m_currentMedia = "";
    m_currentContext = 0;
}

void NPlaybackEngineGStreamer::_emitNextMediaRequest()
{
    m_crossfading = true;
    emit nextMediaRequested();
}

bool NPlaybackEngineGStreamer::_nextMediaRequestBlocked()
{
    return m_nextMediaRequestBlock;
}
