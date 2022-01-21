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

#ifndef N_PLAYBACK_ENGINE_PHONON_H
#define N_PLAYBACK_ENGINE_PHONON_H

#include <phonon/audiooutput.h>
#include <phonon/mediaobject.h>

#include <QTimer>

#include "playbackEngineInterface.h"
#include "plugin.h"

class NPlaybackEnginePhonon : public NPlaybackEngineInterface, public NPlugin
{
    Q_OBJECT
    Q_INTERFACES(NPlaybackEngineInterface NPlugin)

private:
    Phonon::MediaObject *m_mediaObject;
    Phonon::AudioOutput *m_audioOutput;
    qreal m_savedPosition;

public:
    NPlaybackEnginePhonon(QObject *parent = NULL) : NPlaybackEngineInterface(parent) {}
    ~NPlaybackEnginePhonon();
    void init();
    QString interfaceString() const { return NPlaybackEngineInterface::interfaceString(); }
    N::PluginType type() const { return N::PlaybackEngine; }

    Q_INVOKABLE bool hasMedia() const;
    Q_INVOKABLE QString currentMedia() const;
    Q_INVOKABLE N::PlaybackState state() const;

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

private slots:
    void on_tick(qint64 ms);
    void on_volumeChanged(qreal volume);
    void on_stateChanged(Phonon::State newState);

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
