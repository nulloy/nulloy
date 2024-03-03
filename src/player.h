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
class NPlaylistController;
class NWaveformSlider;
class NCoverWidget;
class NImage;
class NCoverReaderInterface;
class NVolumeSlider;
class NScriptEngine;
class QQmlApplicationEngine;
class NSettings;
class NTrackInfoReader;
class NTrackInfoWidget;
class NTrackInfoModel;
class QMenu;
class NActionManager;
class QString;
class QTimer;
class NTagReaderInterface;
class NUtils;

class NPlayer : public QWidget
{
    Q_OBJECT

private:
    NSettings *m_settings;
    NActionManager *m_actionManager;
    NScriptEngine *m_scriptEngine;
    QQmlApplicationEngine *m_qmlEngine;
    NMainWindow *m_mainWindow;
    QObject *m_qmlMainWindow;
    NCoverWidget *m_coverWidget;
    NImage *m_coverImage;
    NCoverReaderInterface *m_coverReader;
    NWaveformSlider *m_waveformSlider;
    NVolumeSlider *m_volumeSlider;
    NTrackInfoReader *m_trackInfoReader;
    NPlaybackEngineInterface *m_playbackEngine;
    NPlaylistWidget *m_playlistWidget;
    NPlaylistController *m_playlistController;
    NTrackInfoWidget *m_trackInfoWidget;
    NTrackInfoModel *m_trackInfoModel;
    NLogDialog *m_logDialog;
    QSystemTrayIcon *m_systemTray;
    QTimer *m_trayClickTimer;
    QTimer *m_settingsSaveTimer;
    QTimer *m_writeDefaultPlaylistTimer;
    bool m_trayIconDoubleClickCheck;
    NUtils *m_utils;

    bool eventFilter(QObject *obj, QEvent *event);
    void writePlaylist(const QString &file, N::M3uExtention ext);

    void connectSignals();
    void loadCoverArt(const QString &file);

    void loadDefaultPlaylist();
    void loadSettings();
    void applySettings();
    void saveSettings();
    void savePlaybackState();

public:
    NPlayer();
    ~NPlayer();
    NMainWindow *mainWindow();
    QObject *qmlMainWindow();
    NPlaybackEngineInterface *playbackEngine();
    NPlaylistWidget *playlistWidget();
    NPlaylistController *playlistController();
    NTagReaderInterface *tagReader();
    NCoverWidget *coverWidget();
    NSettings *settings();
    Q_INVOKABLE QString volumeTooltipText(qreal value) const;

private slots:
    void on_playbackEngine_mediaChanged(const QString &path, int);
    void on_playbackEngine_mediaFailed(const QString &, int);
    void on_playbackEngine_stateChanged(N::PlaybackState state);
    void on_playlist_addMoreRequested();

    void on_mainWindow_closed();
    void on_mainWindow_scrolled(int delta);
    void on_trayIcon_activated(QSystemTrayIcon::ActivationReason reason);
    void on_trayClickTimer_timeout();
    void trayIconCountClicks(int clicks);

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
