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

#ifndef N_PLAYBACK_ENGINE_VLC_H
#define N_PLAYBACK_ENGINE_VLC_H

#include <vlc/vlc.h>
#include <vlc_aout.h>

#include <QTimer>

#include "playbackEngineInterface.h"
#include "plugin.h"

class NPlaybackEngineVlc : public NPlaybackEngineInterface, public NPlugin
{
    Q_OBJECT
    Q_INTERFACES(NPlaybackEngineInterface NPlugin)

private:
    libvlc_instance_t *m_vlcInstance;
    libvlc_media_player_t *m_mediaPlayer;
    libvlc_event_manager_t *m_eventManager;

    QTimer *m_timer;
    qreal m_oldVolume;
    qreal m_oldPosition;
    N::PlaybackState m_oldState;
    QString m_currentMedia;

public:
    NPlaybackEngineVlc(QObject *parent = NULL) : NPlaybackEngineInterface(parent) {}
    ~NPlaybackEngineVlc();
    void init();
    QString interfaceString() const { return NPlaybackEngineInterface::interfaceString(); }
    N::PluginType type() const { return N::PlaybackEngine; }

    Q_INVOKABLE bool hasMedia() const;
    Q_INVOKABLE QString currentMedia() const;
    Q_INVOKABLE N::PlaybackState state() const { return m_oldState; }

    Q_INVOKABLE qreal volume() const;
    Q_INVOKABLE qreal position() const;
    Q_INVOKABLE qint64 durationMsec() const;

public slots:
    Q_INVOKABLE void setMedia(const QString &file);
    Q_INVOKABLE void setVolume(qreal volume);
    Q_INVOKABLE void setPosition(qreal pos);
    Q_INVOKABLE void jump(qint64 msec);

    Q_INVOKABLE void play();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void pause();

    void _emitFinished();

private slots:
    void checkStatus();

signals:
    void positionChanged(qreal pos);
    void volumeChanged(qreal volume);
    void message(N::MessageIcon icon, const QString &title, const QString &msg);
    void mediaChanged(const QString &file);
    void finished();
    void failed();
    void stateChanged(N::PlaybackState state);
    void tick(qint64 msec);
};

#endif
