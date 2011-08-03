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

#ifndef N_PLAYBACK_ENGINE_PHONON_H
#define N_PLAYBACK_ENGINE_PHONON_H

#include "pluginInterface.h"
#include "playbackEngineInterface.h"
#include <QTimer>
#include <phonon/audiooutput.h>
#include <phonon/mediaobject.h>

class NPlaybackEnginePhonon : public NPlaybackEngineInterface, public NPluginInterface
{
	Q_OBJECT
	Q_INTERFACES(NPlaybackEngineInterface NPluginInterface)

private:
	Phonon::MediaObject *m_mediaObject;
	Phonon::AudioOutput *m_audioOutput;
	qreal m_savedPosition;

public:
	NPlaybackEnginePhonon(QObject *parent = NULL) : NPlaybackEngineInterface(parent) {}
	~NPlaybackEnginePhonon();
	void init();
	QString identifier() { return "Nulloy/Playback/Phonon/0.1.1"; }
	QString interface() { return NPlaybackEngineInterface::interface(); }

	Q_INVOKABLE bool hasMedia();
	Q_INVOKABLE QString currentMedia();

	Q_INVOKABLE qreal volume();
	Q_INVOKABLE qreal position();

public slots:
	Q_INVOKABLE void setMedia(const QString &file);
	Q_INVOKABLE void setVolume(qreal volume);
	Q_INVOKABLE void setPosition(qreal pos);

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
	void message(QMessageBox::Icon icon, const QString &title, const QString &msg);
	void mediaChanged(const QString &file);
	void finished();
	void failed();
	void playStateChanged(bool);
};

#endif

/* vim: set ts=4 sw=4: */
