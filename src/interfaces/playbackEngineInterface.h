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

#ifndef N_PLAYBACK_ENGINE_INTERFACE_H
#define N_PLAYBACK_ENGINE_INTERFACE_H

#include "global.h"

#define PLAYBACK_INTERFACE "Nulloy/NPlaybackEngineInterface/0.9"

class NPlaybackEngineInterface : public QObject
{
public:
    NPlaybackEngineInterface(QObject *parent = 0) : QObject(parent) {}
    virtual ~NPlaybackEngineInterface() {}

    Q_INVOKABLE virtual bool hasMedia() const = 0;
    Q_INVOKABLE virtual QString currentMedia() const = 0;
    Q_INVOKABLE virtual N::PlaybackState state() const = 0;

    Q_INVOKABLE virtual qreal volume() const = 0;
    Q_INVOKABLE virtual qreal position() const = 0;
    Q_INVOKABLE virtual qint64 durationMsec() const = 0;
    Q_INVOKABLE virtual qreal speed() const { return 1.0; }
    Q_INVOKABLE virtual qreal pitch() const { return 1.0; }

    static QString interfaceString() { return PLAYBACK_INTERFACE; }

public slots:
    Q_INVOKABLE virtual void setMedia(const QString &file, int context) = 0;
    // if responded, will emit mediaChanged() instead of mediaFinished():
    Q_INVOKABLE virtual void nextMediaRespond(const QString &file, int context)
    {
        Q_UNUSED(file);
        Q_UNUSED(context);
    };
    Q_INVOKABLE virtual void setVolume(qreal volume) = 0;
    Q_INVOKABLE virtual void setPosition(qreal pos) = 0;
    Q_INVOKABLE virtual void jump(qint64 msec) { Q_UNUSED(msec); }
    Q_INVOKABLE virtual void setSpeed(qreal speed) { Q_UNUSED(speed); }
    Q_INVOKABLE virtual void setPitch(qreal pitch) { Q_UNUSED(pitch); }

    Q_INVOKABLE virtual void play() = 0;
    Q_INVOKABLE virtual void stop() = 0;
    Q_INVOKABLE virtual void pause() = 0;

signals:
    virtual void positionChanged(qreal pos) = 0;
    virtual void volumeChanged(qreal vol) = 0;
    virtual void message(N::MessageIcon icon, const QString &title, const QString &msg) = 0;
    // emitted if playback started, otherwise will emit mediaFailed:
    virtual void mediaChanged(const QString &file, int context) = 0;
    virtual void nextMediaRequested() {}
    // emitted if there is no next media supplied(not responded to nextMediaRequested, or if gapless playback not supported):
    virtual void mediaFinished(const QString &file, int context) = 0;
    // emitted instead of mediaChanged:
    virtual void mediaFailed(const QString &file, int context) = 0;
    virtual void stateChanged(N::PlaybackState state) = 0;
    virtual void tick(qint64 msec) = 0;
};

Q_DECLARE_INTERFACE(NPlaybackEngineInterface, PLAYBACK_INTERFACE)

#endif
