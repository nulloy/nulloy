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

#ifndef N_PLAYBACK_ENGINE_GSTREAMER_H
#define N_PLAYBACK_ENGINE_GSTREAMER_H

#include "pluginInterface.h"
#include "playbackEngineInterface.h"
#include <QObject>
#include <QTimer>
#include <gst/gst.h>

class NPlaybackEngineGStreamer : public NPlaybackEngineInterface, public NPluginInterface
{
	Q_OBJECT
	Q_INTERFACES(NPlaybackEngineInterface NPluginInterface)

private:
	GstElement *m_playbin;

	QTimer *m_timer;
	qreal m_oldVolume;
	qreal m_oldPosition;
	bool m_oldPlayState;
	qreal m_savedPosition;
	QString m_currentMedia;

public:
	NPlaybackEngineGStreamer(QObject *parent = NULL) : NPlaybackEngineInterface(parent) {}
	~NPlaybackEngineGStreamer();
	void init();
	QString identifier() { return "Nulloy/Playback/GStreamer/0.2"; }
	QString interface() { return PLAYBACK_INTERFACE; }

	Q_INVOKABLE bool hasMedia();
	Q_INVOKABLE QString currentMedia();

	Q_INVOKABLE qreal volume();
	Q_INVOKABLE qreal position();

public slots:
	void setMedia(const QString &file);
	void setVolume(qreal volume);
	void setPosition(qreal pos);
	void emitFinished();
	void emitError(QString error);

	void play();
	void stop();
	void pause();

private slots:
	void checkStatus();

signals:
	void positionChanged(qreal pos);
	void volumeChanged(qreal volume);
	void message(QMessageBox::Icon icon, const QString &title, const QString &msg);
	void mediaChanged(const QString &file);
	void finished();
	void playStateChanged(bool);
};

#endif

/* vim: set ts=4 sw=4: */
