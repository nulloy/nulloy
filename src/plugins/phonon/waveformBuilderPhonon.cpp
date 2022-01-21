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

#include "waveformBuilderPhonon.h"

#include <QDebug>
#include <QFile>

void NWaveformBuilderPhonon::handleData(
    const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16>> &data)
{
    for (int i = 0; i < m_audioDataOutput->dataSize(); ++i) {
        qint32 pcmValue = 0;
        for (int j = 0; j < data.size(); ++j)
            pcmValue += data[(Phonon::AudioDataOutput::Channel)j][i];
        qreal realValue = -((qreal)pcmValue / data.size()) / (1 << 15);
        m_peaks.append(realValue);
    }
}

void NWaveformBuilderPhonon::init()
{
    if (m_init) {
        return;
    }

    m_audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    m_audioOutput->setVolume(0);

    m_mediaObject = new Phonon::MediaObject(this);

    m_audioDataOutput = new Phonon::AudioDataOutput(this);

    Phonon::createPath(m_mediaObject, m_audioDataOutput);
    Phonon::createPath(m_audioDataOutput, m_audioOutput);

    connect(m_audioDataOutput,
            SIGNAL(dataReady(const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16>> &)),
            this,
            SLOT(handleData(const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16>> &)));

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));

    reset();

    m_init = true;
}

NWaveformBuilderPhonon::~NWaveformBuilderPhonon()
{
    if (!m_init) {
        return;
    }

    m_timer->stop();

    m_mediaObject->stop();
    m_mediaObject->clearQueue();

    quit();
    wait();
}

void NWaveformBuilderPhonon::stop()
{
    m_timer->stop();
    m_mediaObject->stop();
    m_mediaObject->clearQueue();

    if (isRunning()) {
        quit();
        wait();
    }
}

void NWaveformBuilderPhonon::start(const QString &file)
{
    stop();

    if (peaksFindFromCache(file)) {
        return;
    }
    if (!QFileInfo(file).exists()) {
        return;
    }
    m_currentFile = file;

    m_mediaObject->setCurrentSource(Phonon::MediaSource(file));

    reset();
    QThread::start();
    m_mediaObject->play();
    m_timer->start(300);
}

void NWaveformBuilderPhonon::update()
{
    if (m_mediaObject->state() != Phonon::PlayingState) {
        m_peaks.complete();
#if defined(QT_DEBUG) && !defined(QT_NO_DEBUG)
        qDebug() << "WaveformBuilder ::"
                 << "completed" << m_peaks.size();
#endif
        peaksAppendToCache(m_currentFile);
        stop();
    }
}

qreal NWaveformBuilderPhonon::position() const
{
    if (!isRunning()) {
        return 0;
    }

    return (qreal)m_mediaObject->currentTime() / m_mediaObject->totalTime();
}
