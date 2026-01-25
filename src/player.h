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

#include <QObject>
#include <QSystemTrayIcon>

#include "global.h"

class NLogDialogHandler;
class NPlaybackEngineInterface;
class NPlaylistController;
class NImage;
class NCoverReaderInterface;
class QQmlApplicationEngine;
class NSettings;
class NTrackInfoReader;
class NTrackInfoModel;
class QMenu;
class NActionManager;
class QString;
class QTimer;
class NTagReaderInterface;
class NUtils;
class NWaveformBuilderInterface;
class NMainWindowController;

class NPlayer : public QObject
{
    Q_OBJECT

private:
    NSettings *m_settings{nullptr};
    NActionManager *m_actionManager{nullptr};
    QQmlApplicationEngine *m_qmlEngine{nullptr};
    QWindow *m_mainWindow{nullptr};
    NMainWindowController *m_mainWindowController{nullptr};
    NImage *m_coverImage{nullptr};
    NCoverReaderInterface *m_coverReader{nullptr};
    NTrackInfoReader *m_trackInfoReader{nullptr};
    NPlaybackEngineInterface *m_playbackEngine{nullptr};
    NPlaylistController *m_playlistController{nullptr};
    NTrackInfoModel *m_trackInfoModel{nullptr};
    NLogDialogHandler *m_logDialogHandler{nullptr};
    QSystemTrayIcon *m_systemTray{nullptr};
    QTimer *m_trayClickTimer{nullptr};
    QTimer *m_settingsSaveTimer{nullptr};
    QTimer *m_writeDefaultPlaylistTimer{nullptr};
    bool m_trayIconDoubleClickCheck{nullptr};
    NUtils *m_utils{nullptr};
    NWaveformBuilderInterface *m_waveBuilder{nullptr};

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
    NMainWindowController *mainWindowController();
    NPlaybackEngineInterface *playbackEngine();
    NPlaylistController *playlistController();
    NTagReaderInterface *tagReader();
    NImage *coverImage();
    NSettings *settings();
    Q_INVOKABLE QString volumeTooltipText(qreal value) const;

private slots:
    void on_playbackEngine_mediaChanged(const QString &path, int);
    void on_playbackEngine_mediaFailed(const QString &, int);
    void on_playbackEngine_stateChanged(N::PlaybackState state);
    void on_playlist_addMoreRequested();

    void on_mainWindow_closed();
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
