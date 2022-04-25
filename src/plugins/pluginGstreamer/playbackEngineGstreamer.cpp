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

#include "playbackEngineGstreamer.h"

#include <QTimer>
#include <QtGlobal>

#include "common.h"

#define NSEC_IN_MSEC 1000000

static void _on_about_to_finish(GstElement *playbin, gpointer userData)
{
    NPlaybackEngineGStreamer *obj = reinterpret_cast<NPlaybackEngineGStreamer *>(userData);

    gchar *uri_before;
    g_object_get(playbin, "uri", &uri_before, NULL);

    obj->_crossfadingPrepare();
    obj->_emitAboutToFinish();

    gchar *uri_after =
        g_filename_to_uri(QFileInfo(obj->currentMedia()).absoluteFilePath().toUtf8().constData(),
                          NULL, NULL);

    if (g_strcmp0(uri_before, uri_after) == 0) { // uri hasn't changed
        obj->_crossfadingCancel();
    }

    g_free(uri_before);
    g_free(uri_after);
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
    if (m_init) {
        return;
    }

    int argc;
    const char **argv;
    GError *err = NULL;
    NCore::cArgs(&argc, &argv);
    gst_init(&argc, (char ***)&argv);
    if (!gst_init_check(&argc, (char ***)&argv, &err)) {
        emit message(N::Critical, QFileInfo(m_currentMedia).absoluteFilePath(),
                     err ? QString::fromUtf8(err->message) : "unknown error");
        emit failed();
        if (err) {
            g_error_free(err);
        }
    }

    m_playbin = gst_element_factory_make("playbin", NULL);
    g_signal_connect(m_playbin, "about-to-finish", G_CALLBACK(_on_about_to_finish), this);

#ifdef _TESTS_
    GstElement *sink = gst_element_factory_make("fakesink", NULL);
    g_object_set(sink, "sync", TRUE, NULL);
    g_object_set(m_playbin, "audio-sink", sink, NULL);

    sink = gst_element_factory_make("fakesink", NULL);
    g_object_set(sink, "sync", TRUE, NULL);
    g_object_set(m_playbin, "video-sink", sink, NULL);
#endif

    m_oldVolume = -1;
    m_oldPosition = -1;
    m_posponedPosition = -1;
    m_oldState = N::PlaybackStopped;
    m_currentMedia = "";
    m_durationNsec = 0;
    m_crossfading = false;

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(checkStatus()));

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

void NPlaybackEngineGStreamer::setMedia(const QString &file)
{
    qreal vol = m_oldVolume;

    if (!m_crossfading || file.isEmpty()) {
        stop();
    }

    if (file.isEmpty()) {
        emit mediaChanged(m_currentMedia = "");
        return;
    }

    if (!QFile(file).exists()) {
        fail();
        emit message(N::Warning, file, "No such file or directory");
        return;
    }

    gchar *uri = g_filename_to_uri(QFileInfo(file).absoluteFilePath().toUtf8().constData(), NULL,
                                   NULL);
    if (uri) {
        m_currentMedia = file;
    }
    g_object_set(m_playbin, "uri", uri, NULL);

    emit mediaChanged(m_currentMedia);

    if (vol != -1) {
        setVolume(vol);
    }
}

void NPlaybackEngineGStreamer::setVolume(qreal volume)
{
    g_object_set(m_playbin, "volume", qBound(0.0, volume, 1.0), NULL);
}

qreal NPlaybackEngineGStreamer::volume() const
{
    gdouble volume;
    g_object_get(m_playbin, "volume", &volume, NULL);
    return (qreal)volume;
}

void NPlaybackEngineGStreamer::setPosition(qreal pos)
{
    if (!hasMedia() || pos < 0 || pos > 1) {
        return;
    }

    if (m_durationNsec > 0) {
        gst_element_seek_simple(m_playbin, GST_FORMAT_TIME,
                                GstSeekFlags(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT),
                                pos * m_durationNsec);
    } else {
        m_posponedPosition = pos;
    }
}

void NPlaybackEngineGStreamer::jump(qint64 msec)
{
    if (!hasMedia()) {
        return;
    }

    gint64 posNsec = qBound((gint64)0,
                            (gint64)qRound64(position() * m_durationNsec + msec * NSEC_IN_MSEC),
                            m_durationNsec);
    gst_element_seek_simple(m_playbin, GST_FORMAT_TIME,
                            GstSeekFlags(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT), posNsec);
}

qreal NPlaybackEngineGStreamer::position() const
{
    return m_crossfading ? 0 : m_oldPosition;
}

qint64 NPlaybackEngineGStreamer::durationMsec() const
{
    return m_durationNsec / NSEC_IN_MSEC;
}

void NPlaybackEngineGStreamer::play()
{
    if (!hasMedia() || m_crossfading) {
        return;
    }

    GstState gstState;
    gst_element_get_state(m_playbin, &gstState, NULL, 0);
    if (gstState != GST_STATE_PLAYING) {
        gst_element_set_state(m_playbin, GST_STATE_PLAYING);
        m_timer->start(100);
    } else {
        pause();
    }
}

void NPlaybackEngineGStreamer::pause()
{
    if (!hasMedia()) {
        return;
    }

    gst_element_set_state(m_playbin, GST_STATE_PAUSED);

    m_timer->stop();
    checkStatus();
}

void NPlaybackEngineGStreamer::stop()
{
    if (!hasMedia()) {
        return;
    }

    m_crossfading = false;
    gst_element_set_state(m_playbin, GST_STATE_NULL);
    m_durationNsec = 0;
}

bool NPlaybackEngineGStreamer::hasMedia() const
{
    return !m_currentMedia.isEmpty();
}

QString NPlaybackEngineGStreamer::currentMedia() const
{
    return m_currentMedia;
}

void NPlaybackEngineGStreamer::checkStatus()
{
    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_playbin));
    GstMessage *msg;
    while ((msg = gst_bus_pop_filtered(bus, GstMessageType(GST_MESSAGE_EOS | GST_MESSAGE_ERROR))) !=
           NULL) {
        switch (GST_MESSAGE_TYPE(msg)) {
            case GST_MESSAGE_EOS: {
                stop();
                emit finished();
                emit stateChanged(m_oldState = N::PlaybackStopped);
                break;
            }
            case GST_MESSAGE_ERROR: {
                gchar *debug;
                GError *err = NULL;
                gst_message_parse_error(msg, &err, &debug);
                g_free(debug);

                emit message(N::Critical, QFileInfo(m_currentMedia).absoluteFilePath(),
                             err ? QString::fromUtf8(err->message) : "unknown error");
                fail();

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

    GstState gstState;
    if (gst_element_get_state(m_playbin, &gstState, NULL, 0) != GST_STATE_CHANGE_SUCCESS) {
        return;
    }

    N::PlaybackState state = fromGstState(gstState);
    if (m_oldState != state) {
        emit stateChanged(m_oldState = state);
    }

    if (state == N::PlaybackPlaying || state == N::PlaybackPaused) {
        // duration may change for some reason
        // TODO use DURATION_CHANGED in gstreamer1.0
        gboolean res = gst_element_query_duration(m_playbin, GST_FORMAT_TIME, &m_durationNsec);
        if (!res) {
            m_durationNsec = 0;
        }
    }

    if (m_posponedPosition >= 0 && m_durationNsec > 0) {
        setPosition(m_posponedPosition);
        m_posponedPosition = -1;
        emit positionChanged(m_posponedPosition);
    } else {
        qreal pos;
        gint64 gstPos = 0;

        if (!hasMedia() || m_durationNsec <= 0) {
            pos = -1;
        } else {
            gboolean res = gst_element_query_position(m_playbin, GST_FORMAT_TIME, &gstPos);
            if (!res) {
                gstPos = 0;
            }
            pos = (qreal)gstPos / m_durationNsec;
        }

        if (m_oldPosition != pos) {
            if (m_oldPosition > pos) {
                m_crossfading = false;
            }
            m_oldPosition = pos;
            emit positionChanged(m_crossfading ? 0 : m_oldPosition);
        }

        emit tick(m_crossfading ? 0 : gstPos / NSEC_IN_MSEC);
    }

    qreal vol = volume();
    if (qAbs(m_oldVolume - vol) > 0.0001) {
        m_oldVolume = vol;
        emit volumeChanged(vol);
    }

    if (state == N::PlaybackStopped) {
        m_timer->stop();
    }
}

void NPlaybackEngineGStreamer::fail()
{
    if (!m_crossfading) { // avoid thread deadlock
        stop();
    } else {
        m_crossfading = false;
    }
    emit mediaChanged(m_currentMedia = "");
    emit failed();
    emit stateChanged(m_oldState = N::PlaybackStopped);
}

void NPlaybackEngineGStreamer::_emitAboutToFinish()
{
    emit aboutToFinish();
}

void NPlaybackEngineGStreamer::_crossfadingPrepare()
{
    m_crossfading = true;
}

void NPlaybackEngineGStreamer::_crossfadingCancel()
{
    m_crossfading = false;
}
