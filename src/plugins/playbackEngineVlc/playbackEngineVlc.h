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

#ifndef N_PLAYBACK_ENGINE_VLC_H
#define N_PLAYBACK_ENGINE_VLC_H

#include "pluginInterface.h"
#include "playbackEngineInterface.h"
#include <QTimer>
#include <vlc/vlc.h>
#include <vlc_aout.h>

class NPlaybackEngineVlc : public NPlaybackEngineInterface, public NPluginInterface
{
	Q_OBJECT
	Q_INTERFACES(NPlaybackEngineInterface NPluginInterface)

private:
	libvlc_instance_t *m_vlcInstance;
	libvlc_media_player_t *m_mediaPlayer;
	libvlc_event_manager_t *m_eventManager;

	QTimer *m_timer;
	qreal m_oldVolume;
	qreal m_oldPosition;
	bool m_oldPlayState;
	QString m_currentMedia;

public:
	NPlaybackEngineVlc(QObject *parent = NULL) : NPlaybackEngineInterface(parent) {}
	~NPlaybackEngineVlc();
	void init();
	QString identifier() { return "Nulloy/Playback/VLC/0.1.1"; }
	QString interface() { return PLAYBACK_INTERFACE; }

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

	void _emitFinished();

private slots:
	void checkStatus();

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
