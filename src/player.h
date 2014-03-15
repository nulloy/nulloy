/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2014 Sergey Vlasov <sergey@vlasov.me>
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

#include <QWidget>
#include <QSystemTrayIcon>
#include "global.h"

class NLogDialog;
class NMainWindow;
class NPlaybackEngineInterface;
class NPlaylistWidget;
class NPreferencesDialog;
class NAboutDialog;
class NScriptEngine;
class NSettings;
class NTrackInfoWidget;
class QMenu;
class QNetworkAccessManager;
class QNetworkReply;
class QString;
class QSystemTrayIcon;
class QTimer;

class NPlayer : public QWidget
{
	Q_OBJECT

private:
	NSettings *m_settings;
	NScriptEngine *m_scriptEngine;
	NMainWindow *m_mainWindow;
	NPreferencesDialog *m_preferencesDialog;
	NAboutDialog *m_aboutDialog;
	NPlaybackEngineInterface *m_playbackEngine;
	QMenu *m_contextMenu;
	NPlaylistWidget *m_playlistWidget;
	NTrackInfoWidget *m_trackInfoWidget;
	NLogDialog *m_logDialog;
	QNetworkAccessManager *m_versionDownloader;
	QTimer *m_trayClickTimer;
	bool m_trayIconDoubleClickCheck;

	bool eventFilter(QObject *obj, QEvent *event);
	void writePlaylist(const QString &file, N::M3uExtention ext);

public:
	NPlayer();
	~NPlayer();

	Q_INVOKABLE NMainWindow* mainWindow();
	Q_INVOKABLE NPlaybackEngineInterface* playbackEngine();
	Q_INVOKABLE NSettings* settings();

private slots:
	void loadSettings();
	void saveSettings();

	void on_preferencesDialog_settingsChanged();
	void on_trayIcon_activated(QSystemTrayIcon::ActivationReason reason);
	void on_mainWindow_closed();
	void on_playbackEngine_mediaChanged(const QString &path);
	void on_playbackEngine_stateChanged(N::PlaybackState state);
	void on_alwaysOnTopAction_toggled(bool checked);
	void on_whilePlayingOnTopAction_toggled(bool checked);
	void on_showCoverAction_toggled(bool checked);
	void downloadVersion();
	void on_versionDownloader_finished(QNetworkReply *reply);
	void loadNextActionTriggered();
	void on_trayClickTimer_timeout();
	void trayIconCountClicks(int clicks);

public slots:
	void quit();
	void toggleWindowVisibility();
	void showAboutMessageBox();
	void showOpenFileDialog();
	void showOpenDirDialog();
	void showSavePlaylistDialog();
	void showContextMenu(QPoint pos);
	void message(const QString &str);
	void loadDefaultPlaylist();
	void saveDefaultPlaylist();
};

#endif

