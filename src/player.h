/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2013 Sergey Vlasov <sergey@vlasov.me>
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

#include "settings.h"
#include "playbackEngineInterface.h"
#include "waveformSlider.h"
#include "preferencesDialog.h"
#include "playlistWidget.h"
#include "trackInfoWidget.h"
#include "mainWindow.h"
#include "logDialog.h"

#include <QtScript>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class NPlayer : public QWidget
{
	Q_OBJECT

private:
	NSettings *m_settings;
	QScriptEngine *m_scriptEngine;
	NMainWindow *m_mainWindow;
	NPreferencesDialog *m_preferencesDialog;
	NPlaybackEngineInterface *m_playbackEngine;
	QMenu *m_contextMenu;
	NPlaylistWidget *m_playlistWidget;
	NTrackInfoWidget *m_trackInfoWidget;
	NLogDialog *m_logDialog;
	QString m_localPlaylist;
	QNetworkAccessManager *m_networkManager;
	QTimer *m_trayIconDoubleClickTimer;
	bool m_trayIconDoubleClickCheck;
	QWidget *m_waveformSlider;

	bool eventFilter(QObject *obj, QEvent *event);

public:
	NPlayer();
	~NPlayer();

	Q_INVOKABLE NMainWindow* mainWindow();
	Q_INVOKABLE NPlaybackEngineInterface* playbackEngine();
	Q_INVOKABLE NSettings* settings();

private slots:
	void loadSettings();
	void saveSettings();

	void savePlaylist();

	void preferencesDialogSettingsChanged();
	void on_trayIcon_activated(QSystemTrayIcon::ActivationReason reason);
	void mainWindowClosed();
	void on_playbackEngine_mediaChanged(const QString &path);
	void on_playbackEngine_stateChanged(int state);
	void on_alwaysOnTopAction_toggled(bool checked);
	void on_whilePlayingOnTopAction_toggled(bool checked);
	void versionOnlineFetch();
	void on_networkManager_finished(QNetworkReply *reply);
	void loadNextActionTriggered();
	void trayIconDoubleClick_timeout();
	void trackIcon_clicked(int clicks);
	void waveformSliderToolTip(int x, int y);

public slots:
	void quit();
	void showAboutMessageBox();
	void showOpenFileDialog();
	void showSavePlaylistDialog();
	void showContextMenu(QPoint pos);
	void message(const QString &str);
	void restorePlaylist();
};

#endif

/* vim: set ts=4 sw=4: */
