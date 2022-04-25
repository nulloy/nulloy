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

#include "playbackEngineVlc.h"

#include <QtGlobal>

#include "common.h"

static void _eventHandler(const libvlc_event_t *event, void *userData)
{
    if (event->type == libvlc_MediaPlayerEndReached) {
        NPlaybackEngineVlc *obj = reinterpret_cast<NPlaybackEngineVlc *>(userData);
        obj->_emitFinished();
    }
}

static N::PlaybackState fromVlcState(libvlc_state_t state)
{
    switch (state) {
        case libvlc_Paused:
            return N::PlaybackPaused;
        case libvlc_Playing:
        case libvlc_Opening:
        case libvlc_Buffering:
            return N::PlaybackPlaying;
        default:
            return N::PlaybackStopped;
    }
}

void NPlaybackEngineVlc::init()
{
    int argc;
    const char **argv;
    NCore::cArgs(&argc, &argv);

    QVector<const char *> argVector;
    for (int i = 0; i < argc; ++i)
        argVector << argv[i];

    argVector << "-I"
              << "dummy"
              << "--ignore-config"
              << "--no-xlib";

    m_vlcInstance = libvlc_new(argVector.size(), &argVector[0]);
    m_mediaPlayer = libvlc_media_player_new(m_vlcInstance);
    m_eventManager = libvlc_media_player_event_manager(m_mediaPlayer);
    libvlc_event_attach(m_eventManager, libvlc_MediaPlayerEndReached, _eventHandler, this);

    m_oldVolume = -1;
    m_oldPosition = -1;
    m_oldState = N::PlaybackStopped;

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(checkStatus()));
    m_timer->start(100);

    m_init = true;
}

NPlaybackEngineVlc::~NPlaybackEngineVlc()
{
    if (!m_init) {
        return;
    }

    stop();
    libvlc_media_player_release(m_mediaPlayer);
    libvlc_release(m_vlcInstance);
}

void NPlaybackEngineVlc::setMedia(const QString &file)
{
    stop();

    if (file.isEmpty()) {
        return;
    }

    if (!QFile(file).exists()) {
        emit message(N::Warning, file, "No such file or directory");
        emit mediaChanged("");
        emit failed();
        return;
    }

    libvlc_media_t *media = libvlc_media_player_get_media(m_mediaPlayer);
    if (media)
        libvlc_media_release(media);

    libvlc_media_t *mediaDescriptor = libvlc_media_new_path(m_vlcInstance, file.toUtf8());
    if (mediaDescriptor)
        libvlc_media_player_set_media(m_mediaPlayer, mediaDescriptor);

    emit mediaChanged(file);
}

void NPlaybackEngineVlc::setVolume(qreal volume)
{
    libvlc_audio_set_volume(m_mediaPlayer, qRound(qBound(0.0, volume, 1.0) * 100 / 2));
}

qreal NPlaybackEngineVlc::volume() const
{
    return libvlc_audio_get_volume(m_mediaPlayer) / (qreal)100 * 2;
}

void NPlaybackEngineVlc::setPosition(qreal pos)
{
    if (!hasMedia() || pos < 0) {
        return;
    }

    libvlc_media_player_set_position(m_mediaPlayer, qBound(0.0, pos, 1.0));
}

qreal NPlaybackEngineVlc::position() const
{
    if (!hasMedia()) {
        return -1;
    }

    return libvlc_media_player_get_position(m_mediaPlayer);
}

void NPlaybackEngineVlc::jump(qint64 msec)
{
    if (!hasMedia() || !libvlc_media_player_is_seekable(m_mediaPlayer))
        return;

    qint64 posMsec = qBound(0LL, libvlc_media_player_get_time(m_mediaPlayer) + msec, durationMsec());
    libvlc_media_player_set_time(m_mediaPlayer, posMsec);
}

qint64 NPlaybackEngineVlc::durationMsec() const
{
    if (!hasMedia()) {
        return -1;
    }

    return libvlc_media_player_get_length(m_mediaPlayer);
}

void NPlaybackEngineVlc::play()
{
    if (!hasMedia()) {
        return;
    }

    if (!libvlc_media_player_is_playing(m_mediaPlayer))
        libvlc_media_player_play(m_mediaPlayer);
    else
        pause();
}

void NPlaybackEngineVlc::pause()
{
    if (!hasMedia()) {
        return;
    }

    libvlc_media_player_set_pause(m_mediaPlayer, true);
}

void NPlaybackEngineVlc::stop()
{
    if (!hasMedia()) {
        return;
    }

    libvlc_media_player_stop(m_mediaPlayer);
}

bool NPlaybackEngineVlc::hasMedia() const
{
    libvlc_media_t *media = libvlc_media_player_get_media(m_mediaPlayer);
    return (media != NULL);
}

QString NPlaybackEngineVlc::currentMedia() const
{
    libvlc_media_t *media = libvlc_media_player_get_media(m_mediaPlayer);
    if (media != NULL)
        return QUrl(QUrl::fromPercentEncoding(libvlc_media_get_mrl(media))).toLocalFile();
    return QString();
}

void NPlaybackEngineVlc::checkStatus()
{
    qreal pos = position();
    if (m_oldPosition != pos) {
        m_oldPosition = pos;
        emit positionChanged(pos);
    }

    qreal vol = volume();
    if (m_oldVolume != vol) {
        m_oldVolume = vol;
        emit volumeChanged(vol);
    }

    libvlc_state_t vlcState = libvlc_media_player_get_state(m_mediaPlayer);
    N::PlaybackState state = fromVlcState(vlcState);
    if (m_oldState != state) {
        emit stateChanged(state);
        m_oldState = state;
    }

    emit tick(libvlc_media_player_get_time(m_mediaPlayer));
}

void NPlaybackEngineVlc::_emitFinished()
{
    emit finished();
}
