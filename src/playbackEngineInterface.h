/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2011 Sergey Vlasov <sergey@vlasov.me>
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

#include <QtCore>
#include <QMessageBox>

#define PLAYBACK_INTERFACE "Nulloy/PlaybackEngineInterface/0.1"

class NPlaybackEngineInterface : public QObject
{
public:
	enum State {
		Stopped = 0,
		Playing = 1,
		Paused = 2
	};

	NPlaybackEngineInterface(QObject *parent = 0) : QObject(parent) {}
	virtual ~NPlaybackEngineInterface() {}

	Q_INVOKABLE virtual bool hasMedia() = 0;
	Q_INVOKABLE virtual QString currentMedia() = 0;
	Q_INVOKABLE virtual int state() = 0;

	Q_INVOKABLE virtual qreal volume() = 0;
	Q_INVOKABLE virtual qreal position() = 0;

	static QString interface() { return PLAYBACK_INTERFACE; }

public slots:
	Q_INVOKABLE virtual void setMedia(const QString &file) = 0;
	Q_INVOKABLE virtual void setVolume(qreal volume) = 0;
	Q_INVOKABLE virtual void setPosition(qreal pos) = 0;

	Q_INVOKABLE virtual void play() = 0;
	Q_INVOKABLE virtual void stop() = 0;
	Q_INVOKABLE virtual void pause() = 0;

signals:
	virtual void positionChanged(qreal pos) = 0;
	virtual void volumeChanged(qreal vol) = 0;
	virtual void message(QMessageBox::Icon icon, const QString &title, const QString &msg) = 0;
	virtual void mediaChanged(const QString &file) = 0;
	virtual void finished() = 0;
	virtual void failed() = 0;
	virtual void stateChanged(int state) = 0;
};

Q_DECLARE_INTERFACE(NPlaybackEngineInterface, PLAYBACK_INTERFACE)

#endif

/* vim: set ts=4 sw=4: */
