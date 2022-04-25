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

#include "waveformBuilderVlc.h"

#include <QDebug>
#include <QFile>
#include <QMutex>

#include "common.h"

static QMutex _mutex;

static void _prepareBuffer(void *userData, uint8_t **pcmBuffer, unsigned int size)
{
    QMutexLocker locker(&_mutex);

    NWaveformBuilderVlc *obj = reinterpret_cast<NWaveformBuilderVlc *>(userData);
    obj->prepareBuffer(pcmBuffer, size);
}

static void _handleBuffer(void *userData, uint8_t *pcmBuffer, unsigned int nChannels,
                          unsigned int frequency, unsigned int nSamples, unsigned int bitsPerSample,
                          unsigned int size, int64_t pts)
{
    QMutexLocker locker(&_mutex);

    Q_UNUSED(pts);
    Q_UNUSED(frequency);
    Q_UNUSED(bitsPerSample);
    Q_UNUSED(size);

    NWaveformBuilderVlc *obj = reinterpret_cast<NWaveformBuilderVlc *>(userData);
    obj->handleBuffer(pcmBuffer, nChannels, nSamples);
}

void NWaveformBuilderVlc::prepareBuffer(uint8_t **pcmBuffer, unsigned int size)
{
    if (!m_timer->isActive()) {
        m_timer->start(300);
    }

    if (m_pcmBuffer.size() < (int)size) {
        m_pcmBuffer.resize(size);
    }

    *pcmBuffer = (uint8_t *)(m_pcmBuffer.data());
}

void NWaveformBuilderVlc::handleBuffer(uint8_t *pcmBuffer, unsigned int nChannels,
                                       unsigned int nSamples)
{
    for (unsigned int i = 0; i < nSamples; ++i) {
        qint32 pcmValue = 0;
        for (unsigned int j = 0; j < nChannels; ++j) {
            const qint16 *ptr = reinterpret_cast<const qint16 *>(pcmBuffer) + i * nChannels + j;
            pcmValue += *ptr;
        }
        qreal realValue = -((qreal)pcmValue / nChannels) / (1 << 15);
        m_peaks.append(realValue);
    }
}

void NWaveformBuilderVlc::init()
{
    if (m_init) {
        return;
    }

    char smem_options[512];
    sprintf(smem_options,
            "#transcode{acodec=s16l}:smem{"
            "audio-prerender-callback=%lld,"
            "audio-postrender-callback=%lld,"
            "audio-data=%lld,"
            "no-time-sync}",
            (long long int)(intptr_t)(void *)&_prepareBuffer,
            (long long int)(intptr_t)(void *)&_handleBuffer, (long long int)(intptr_t)(void *)this);

    int argc;
    const char **argv;
    NCore::cArgs(&argc, &argv);

    QVector<const char *> argVector;
    for (int i = 0; i < argc; ++i)
        argVector << argv[i];

    argVector << "-I"
              << "dummy"
              << "--ignore-config"
              << "--no-xlib"
              << "--sout" << smem_options;

    m_vlcInstance = libvlc_new(argVector.size(), &argVector[0]);
    m_mediaPlayer = libvlc_media_player_new(m_vlcInstance);

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));

    reset();

    m_init = true;
}

NWaveformBuilderVlc::~NWaveformBuilderVlc()
{
    if (!m_init) {
        return;
    }

    stop();

    libvlc_media_player_release(m_mediaPlayer);
    libvlc_release(m_vlcInstance);
}

void NWaveformBuilderVlc::stop()
{
    m_timer->stop();
    libvlc_media_player_stop(m_mediaPlayer);

    libvlc_media_t *media = libvlc_media_player_get_media(m_mediaPlayer);
    if (media)
        libvlc_media_release(media);

    if (isRunning()) {
        quit();
        wait();
    }
}

void NWaveformBuilderVlc::start(const QString &file)
{
    stop();

    if (peaksFindFromCache(file)) {
        return;
    }
    if (!QFileInfo(file).exists()) {
        return;
    }
    m_currentFile = file;

    libvlc_media_t *media = libvlc_media_player_get_media(m_mediaPlayer);
    if (media) {
        libvlc_media_player_stop(m_mediaPlayer);
        libvlc_media_release(media);
    }

    libvlc_media_t *mediaDescriptor = libvlc_media_new_path(m_vlcInstance, file.toUtf8());
    libvlc_media_player_set_media(m_mediaPlayer, mediaDescriptor);

    reset();
    QThread::start();
    libvlc_media_player_play(m_mediaPlayer);
}

void NWaveformBuilderVlc::update()
{
    if (!libvlc_media_player_is_playing(m_mediaPlayer)) {
        m_peaks.complete();
#if defined(QT_DEBUG) && !defined(QT_NO_DEBUG)
        qDebug() << "WaveformBuilder ::"
                 << "completed" << m_peaks.size();
#endif
        peaksAppendToCache(m_currentFile);
        stop();
    }
}

qreal NWaveformBuilderVlc::position() const
{
    if (!isRunning()) {
        return 0;
    }

    return libvlc_media_player_get_position(m_mediaPlayer);
}
