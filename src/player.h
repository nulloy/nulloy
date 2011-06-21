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

#ifndef N_PLAYER_H
#define N_PLAYER_H

#include "playbackEngineInterface.h"
#include "waveformSlider.h"
#include "preferencesDialog.h"
#include "playlistWidget.h"
#include "mainWindow.h"
#include "logDialog.h"
#include <QSystemTrayIcon>
#include <QMenu>
#include <QtScript>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class NPlayer : public QWidget
{
	Q_OBJECT

private:
	QScriptEngine *m_scriptEngine;
	NMainWindow *m_mainWindow;
	NPreferencesDialog *m_preferencesDialog;
	NPlaybackEngineInterface *m_playbackEngine;
	QSystemTrayIcon *m_trayIcon;
	QMenu *m_contextMenu;
	NPlaylistWidget *m_playlistWidget;
	NLogDialog *m_logDialog;
	QString m_localPlaylist;
	QNetworkAccessManager *m_networkManager;

public:
	NPlayer();
	~NPlayer();
	QString about();

private slots:
	void restorePlaylist();
	void savePlaylist();
	void loadSettings();
	void saveSettings();
	void preferencesDialogSettingsChanged();
	void on_trayIcon_activated(QSystemTrayIcon::ActivationReason reason);
	void mainWindowClosed();
	void on_playbackEngine_mediaChanged(const QString &path);
	void on_alwaysOnTopAction_toggled(bool checked);
	void versionCheckOnline();
	void on_networkManager_finished(QNetworkReply *reply);

public slots:
	void quit();
	void showPreferencesDialog();
	void showAboutMessageBox();
	void showFileDialog();
	void showContextMenu(QPoint pos);
	void message(const QString &str);
};

#endif

/* vim: set ts=4 sw=4: */
