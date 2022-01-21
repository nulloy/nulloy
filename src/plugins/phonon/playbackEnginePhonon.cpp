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

#include "playbackEnginePhonon.h"

#include <QtGlobal>

static N::PlaybackState fromPhononState(Phonon::State state)
{
    switch (state) {
        case Phonon::PlayingState:
        case Phonon::BufferingState:
            return N::PlaybackPlaying;
        case Phonon::PausedState:
            return N::PlaybackPaused;
        default:
            return N::PlaybackStopped;
    }
}

void NPlaybackEnginePhonon::init()
{
    m_savedPosition = -1;

    m_audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    connect(m_audioOutput, SIGNAL(volumeChanged(qreal)), this, SLOT(on_volumeChanged(qreal)));

    m_mediaObject = new Phonon::MediaObject(this);
    connect(m_mediaObject, SIGNAL(tick(qint64)), this, SLOT(on_tick(qint64)));
    connect(m_mediaObject, SIGNAL(finished()), this, SIGNAL(finished()));
    connect(m_mediaObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this,
            SLOT(on_stateChanged(Phonon::State)));
    m_mediaObject->setTickInterval(100);

    Phonon::createPath(m_mediaObject, m_audioOutput);

    m_init = true;
}

NPlaybackEnginePhonon::~NPlaybackEnginePhonon() {}

void NPlaybackEnginePhonon::setMedia(const QString &file)
{
    stop();
    m_mediaObject->clearQueue();

    if (file.isEmpty()) {
        return;
    }

    if (!QFile(file).exists()) {
        emit message(N::Warning, file, "No such file or directory");
        emit mediaChanged("");
        emit failed();
        return;
    }

    m_mediaObject->setCurrentSource(Phonon::MediaSource(QUrl::fromLocalFile(file)));

    emit mediaChanged(file);
}

void NPlaybackEnginePhonon::setVolume(qreal volume)
{
    m_audioOutput->setVolume(qBound(0.0, volume, 1.0));
}

qreal NPlaybackEnginePhonon::volume() const
{
    return m_audioOutput->volume();
}

void NPlaybackEnginePhonon::setPosition(qreal pos)
{
    if (!hasMedia() || pos < 0) {
        return;
    }

    if (m_mediaObject->isSeekable())
        m_mediaObject->seek(qRound(pos * m_mediaObject->totalTime()));
    else
        m_savedPosition = pos;
}

qreal NPlaybackEnginePhonon::position() const
{
    if (!hasMedia()) {
        return -1;
    }

    return (qreal)m_mediaObject->currentTime() / m_mediaObject->totalTime();
}

void NPlaybackEnginePhonon::play()
{
    if (!hasMedia()) {
        return;
    }

    if (m_mediaObject->state() != Phonon::PlayingState)
        m_mediaObject->play();
    else
        m_mediaObject->pause();
}

void NPlaybackEnginePhonon::pause()
{
    if (!hasMedia()) {
        return;
    }

    m_mediaObject->pause();
}

void NPlaybackEnginePhonon::stop()
{
    if (!hasMedia()) {
        return;
    }

    m_mediaObject->stop();
}

bool NPlaybackEnginePhonon::hasMedia() const
{
    Phonon::MediaSource source = m_mediaObject->currentSource();

    if (source.type() == Phonon::MediaSource::Invalid ||
        source.type() == Phonon::MediaSource::Empty) {
        return false;
    } else {
        return true;
    }
}

QString NPlaybackEnginePhonon::currentMedia() const
{
    return m_mediaObject->currentSource().fileName();
}

void NPlaybackEnginePhonon::on_tick(qint64 ms)
{
    if (m_savedPosition >= 0 && m_mediaObject->isSeekable()) {
        setPosition(m_savedPosition);
        m_savedPosition = -1;
        emit positionChanged(m_savedPosition);
        return;
    }

    emit positionChanged((qreal)ms / m_mediaObject->totalTime());
    emit tick(m_mediaObject->currentTime());
}

void NPlaybackEnginePhonon::on_volumeChanged(qreal volume)
{
    emit volumeChanged(volume);
}

void NPlaybackEnginePhonon::on_stateChanged(Phonon::State newState)
{
    emit stateChanged(fromPhononState(newState));
}

N::PlaybackState NPlaybackEnginePhonon::state() const
{
    return fromPhononState(m_mediaObject->state());
}

qint64 NPlaybackEnginePhonon::durationMsec() const
{
    return m_mediaObject->totalTime();
}

void NPlaybackEnginePhonon::jump(qint64 msec)
{
    if (!hasMedia() || !m_mediaObject->isSeekable())
        return;

    qint64 posMsec = qBound(0LL, m_mediaObject->currentTime() + msec,
                            durationMsec() - 1000); // 1000 msec gap to avoid phonon freeze
    m_mediaObject->seek(posMsec);
}
