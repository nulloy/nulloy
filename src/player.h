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

#ifndef N_PLAYER_H
#define N_PLAYER_H

#include <QSystemTrayIcon>
#include <QWidget>

#include "global.h"

class NLogDialog;
class NMainWindow;
class NPlaybackEngineInterface;
class NPlaylistWidget;
class NWaveformSlider;
class NCoverWidget;
class NCoverReaderInterface;
class NVolumeSlider;
class NPreferencesDialog;
class NAboutDialog;
class NScriptEngine;
class NSettings;
class NTrackInfoReader;
class NTrackInfoWidget;
class QMenu;
class NActionManager;
class QString;
class QTimer;
class NTagReaderInterface;

#ifndef _N_NO_UPDATE_CHECK_
class QNetworkAccessManager;
class QNetworkReply;
#endif

class NPlayer : public QWidget
{
    Q_OBJECT

private:
    NSettings *m_settings;
    NActionManager *m_actionManager;
    NScriptEngine *m_scriptEngine;
    NMainWindow *m_mainWindow;
    NCoverWidget *m_coverWidget;
    NCoverReaderInterface *m_coverReader;
    NWaveformSlider *m_waveformSlider;
    NPreferencesDialog *m_preferencesDialog;
    NAboutDialog *m_aboutDialog;
    NVolumeSlider *m_volumeSlider;
    NTrackInfoReader *m_trackInfoReader;
    NPlaybackEngineInterface *m_playbackEngine;
    NPlaylistWidget *m_playlistWidget;
    NTrackInfoWidget *m_trackInfoWidget;
    NLogDialog *m_logDialog;
    QSystemTrayIcon *m_systemTray;
    QTimer *m_trayClickTimer;
    QTimer *m_settingsSaveTimer;
    QTimer *m_writeDefaultPlaylistTimer;
    bool m_trayIconDoubleClickCheck;

    bool eventFilter(QObject *obj, QEvent *event);
    void writePlaylist(const QString &file, N::M3uExtention ext);

    void connectSignals();
    void loadCoverArt(const QString &file);

    void loadDefaultPlaylist();
    void loadSettings();
    void saveSettings();
    void savePlaybackState();

public:
    NPlayer();
    ~NPlayer();
    NMainWindow *mainWindow();
    NPlaybackEngineInterface *playbackEngine();
    NPlaylistWidget *playlistWidget();
    NTagReaderInterface *tagReader();
    NCoverWidget *coverWidget();
    NSettings *settings();

private slots:
    void on_preferencesDialog_settingsChanged();
    void on_playbackEngine_mediaChanged(const QString &path, int);
    void on_playbackEngine_mediaFailed(const QString &, int);
    void on_playbackEngine_stateChanged(N::PlaybackState state);
    void on_playlist_addMoreRequested();

    void on_mainWindow_closed();
    void on_mainWindow_scrolled(int delta);
    void on_trayIcon_activated(QSystemTrayIcon::ActivationReason reason);
    void on_trayClickTimer_timeout();
    void trayIconCountClicks(int clicks);

#ifndef _N_NO_UPDATE_CHECK_
private:
    QNetworkAccessManager *m_versionDownloader;

private slots:
    void downloadVersion();
    void on_versionDownloader_finished(QNetworkReply *reply);
#endif

public slots:
    void quit();
    void playPause();
    void toggleWindowVisibility();
    void showAboutDialog();
    void showPreferencesDialog();
    void showTagEditor(const QString &path);
    void showOpenFileDialog();
    void showOpenDirDialog();
    void showSavePlaylistDialog();
    void showToolTip(const QString &text);
    void showContextMenu(const QPoint &pos);
    void showPlaylistContextMenu(const QPoint &pos);
    void readMessage(const QString &str);
    bool revealInFileManager(const QString &file, QString *error) const;
};

#endif
