/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2025 Sergey Vlasov <sergey@vlasov.me>
**
**  player program can be distributed under the terms of the GNU
**  General Public License version 3.0 as published by the Free
**  Software Foundation and appearing in the file LICENSE.GPL3
**  included in the packaging of player file.  Please review the
**  following information to ensure the GNU General Public License
**  version 3.0 requirements will be met:
**
**  http://www.gnu.org/licenses/gpl-3.0.html
**
*********************************************************************/

#include "actionManager.h"

#include "action.h"
#include "coverWidget.h"
#include "mainWindow.h"
#include "playbackEngineInterface.h"
#include "player.h"
#include "playlistController.h"
#include "playlistWidget.h"
#include "settings.h"
#include "tagReaderInterface.h"
#include "trash.h"

#include <QApplication>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStyle>

NActionManager::NActionManager(NPlayer *player) : QObject(player)
{
    m_player = player;

    m_contextMenu = new QMenu(player->mainWindow());
    player->mainWindow()->setContextMenuPolicy(Qt::CustomContextMenu);
    QMenu *windowSubMenu = new QMenu(tr("Window"), player->mainWindow());
    QMenu *playlistSubMenu = new QMenu(tr("Playlist"), player->mainWindow());

    QMenuBar *menuBar = new QMenuBar(player->mainWindow());
    QMenu *fileMenu = menuBar->addMenu(tr("File"));
    QMenu *controlsMenu = menuBar->addMenu(tr("Controls"));
    QMenu *controlsPlaylistSubMenu = controlsMenu->addMenu(tr("Playlist"));
    controlsMenu->addMenu(controlsPlaylistSubMenu);
    QMenu *windowMenu = menuBar->addMenu(tr("Window"));

    m_trayIconMenu = new QMenu(player);
    m_playlistContextMenu = new QMenu(player->mainWindow());

    QStyle *style = QApplication::style();
    QList<QIcon> winIcons;
#ifdef Q_OS_WIN
    winIcons = NWinIcon::getIcons(QProcessEnvironment::systemEnvironment().value("SystemRoot") +
                                  "/system32/imageres.dll");
#endif

    {
        NAction *showHideAction = new NAction(
            QIcon::fromTheme("preferences-system-windows",
                             QIcon(":/trolltech/styles/commonstyle/images/dockdock-16.png")),
            tr("Show / Hide"), player);
        showHideAction->setObjectName("ShowHideAction");
        showHideAction->setStatusTip(tr("Toggle window visibility"));
        showHideAction->setCustomizable(true);
        connect(showHideAction, &NAction::triggered, player, &NPlayer::toggleWindowVisibility);
        m_trayIconMenu->addAction(showHideAction);
    }

    m_trayIconMenu->addSeparator();

    {
        NAction *playAction = new NAction(QIcon::fromTheme("media-playback-start",
                                                           style->standardIcon(
                                                               QStyle::SP_MediaPlay)),
                                          tr("Play"), player);
        playAction->setObjectName("PlayAction");
        playAction->setStatusTip(tr("Start playback"));
        playAction->setCustomizable(true);
        connect(playAction, &NAction::triggered, player->playbackEngine(),
                &NPlaybackEngineInterface::play);
        m_trayIconMenu->addAction(playAction);
        controlsMenu->addAction(playAction);
    }

    {
        NAction *pauseAction = new NAction(QIcon::fromTheme("media-playback-pause",
                                                            style->standardIcon(
                                                                QStyle::SP_MediaPause)),
                                           tr("Pause"), player);
        pauseAction->setObjectName("PauseAction");
        pauseAction->setStatusTip(tr("Pause playback"));
        pauseAction->setCustomizable(true);
        connect(pauseAction, &NAction::triggered, player->playbackEngine(),
                &NPlaybackEngineInterface::pause);
        m_trayIconMenu->addAction(pauseAction);
        controlsMenu->addAction(pauseAction);
    }

    {
        NAction *playPauseAction = new NAction(QIcon::fromTheme("media-playback-start",
                                                                style->standardIcon(
                                                                    QStyle::SP_MediaPlay)),
                                               tr("Play / Pause"), player);
        playPauseAction->setObjectName("PlayPauseAction");
        playPauseAction->setStatusTip(tr("Toggle playback"));
        playPauseAction->setCustomizable(true);
        connect(playPauseAction, &NAction::triggered, player, &NPlayer::playPause);
        m_trayIconMenu->addAction(playPauseAction);
        controlsMenu->addAction(playPauseAction);
    }

    {
        NAction *stopAction = new NAction(QIcon::fromTheme("media-playback-stop",
                                                           style->standardIcon(
                                                               QStyle::SP_MediaStop)),
                                          tr("Stop"), player);
        stopAction->setObjectName("StopAction");
        stopAction->setStatusTip(tr("Stop playback"));
        stopAction->setCustomizable(true);
        connect(stopAction, &NAction::triggered, player->playbackEngine(),
                &NPlaybackEngineInterface::stop);
        m_trayIconMenu->addAction(stopAction);
        controlsMenu->addAction(stopAction);
    }

    {
        NAction *prevAction = new NAction(QIcon::fromTheme("media-playback-backward",
                                                           style->standardIcon(
                                                               QStyle::SP_MediaSkipBackward)),
                                          tr("Previous"), player);
        prevAction->setObjectName("PrevAction");
        prevAction->setStatusTip(tr("Play previous track in playlist"));
        prevAction->setCustomizable(true);
        //connect(prevAction, &NAction::triggered, player->playlistWidget(),
        //        &NPlaylistWidget::playPrevItem);
        connect(prevAction, &NAction::triggered, player->playlistController(),
                &NPlaylistController::playPrevRow);
        m_trayIconMenu->addAction(prevAction);
        controlsMenu->addAction(prevAction);
    }

    {
        NAction *nextAction = new NAction(QIcon::fromTheme("media-playback-forward",
                                                           style->standardIcon(
                                                               QStyle::SP_MediaSkipForward)),
                                          tr("Next"), player);
        nextAction->setObjectName("NextAction");
        nextAction->setStatusTip(tr("Play next track in playlist"));
        nextAction->setCustomizable(true);
        //connect(nextAction, &NAction::triggered, player->playlistWidget(),
        //        &NPlaylistWidget::playNextItem);
        connect(nextAction, &NAction::triggered, player->playlistController(),
                &NPlaylistController::playNextRow);
        m_trayIconMenu->addAction(nextAction);
        controlsMenu->addAction(nextAction);
    }

    m_trayIconMenu->addSeparator();
    controlsMenu->addSeparator();

    {
        NAction *addFilesAction = new NAction(QIcon::fromTheme("add", winIcons.value(171)),
                                              tr("Add Files..."), player);
        addFilesAction->setShortcut(QKeySequence("Ctrl+O"));
        connect(addFilesAction, &NAction::triggered, player, &NPlayer::showOpenFileDialog);
        m_contextMenu->addAction(addFilesAction);
        fileMenu->addAction(addFilesAction);
    }

    {
        NAction *addDirAction = new NAction(QIcon::fromTheme("folder-add", winIcons.value(3)),
                                            tr("Add Directory..."), player);
        addDirAction->setShortcut(QKeySequence("Ctrl+Shift+O"));
        connect(addDirAction, &NAction::triggered, player, &NPlayer::showOpenDirDialog);
        m_contextMenu->addAction(addDirAction);
        fileMenu->addAction(addDirAction);
    }

    {
        NAction *savePlaylistAction = new NAction(QIcon::fromTheme("document-save",
                                                                   winIcons.value(175)),
                                                  tr("Save Playlist..."), player);
        savePlaylistAction->setShortcut(QKeySequence("Ctrl+S"));
        connect(savePlaylistAction, &NAction::triggered, player, &NPlayer::showSavePlaylistDialog);
        m_contextMenu->addAction(savePlaylistAction);
        fileMenu->addAction(savePlaylistAction);
    }

    m_contextMenu->addMenu(windowSubMenu);
    m_contextMenu->addMenu(playlistSubMenu);

    {
        NAction *preferencesAction = new NAction(QIcon::fromTheme("configure", winIcons.value(109)),
                                                 tr("Preferences..."), player);
        preferencesAction->setShortcut(QKeySequence("Ctrl+P"));
        connect(preferencesAction, &NAction::triggered, player, &NPlayer::showPreferencesDialog);
        m_trayIconMenu->addAction(preferencesAction);
        m_contextMenu->addAction(preferencesAction);
        fileMenu->addAction(preferencesAction);
    }

    m_contextMenu->addSeparator();

    if (player->coverWidget()) {
        NAction *showCoverAction = new NAction(tr("Show Cover Art"), player);
        showCoverAction->setCheckable(true);
        showCoverAction->setObjectName("ShowCoverAction");
        connect(showCoverAction, &NAction::toggled, [player](bool checked) {
            player->settings()->setValue("ShowCoverArt", checked);
            player->coverWidget()->setVisible(checked);
        });
        showCoverAction->setChecked(player->settings()->value("ShowCoverArt").toBool());
        windowSubMenu->addAction(showCoverAction);
        windowMenu->addAction(showCoverAction);
    }

    {
        NAction *showPlaybackControlsAction = new NAction(tr("Show Playback Controls"), player);
        showPlaybackControlsAction->setCheckable(true);
        showPlaybackControlsAction->setObjectName("ShowPlaybackControls");
        connect(showPlaybackControlsAction, &NAction::toggled, player->mainWindow(),
                &NMainWindow::showPlaybackControls);
        showPlaybackControlsAction->setChecked(
            player->settings()->value("ShowPlaybackControls").toBool());
        windowSubMenu->addAction(showPlaybackControlsAction);
        windowMenu->addAction(showPlaybackControlsAction);
    }

    {
        NAction *aboutAction = new NAction(QIcon::fromTheme("help", winIcons.value(76)),
                                           tr("About"), player);
        connect(aboutAction, &NAction::triggered, player, &NPlayer::showAboutDialog);
        m_contextMenu->addAction(aboutAction);
        fileMenu->addAction(aboutAction);
    }

    m_contextMenu->addSeparator();

    {
        NAction *exitAction = new NAction(QIcon::fromTheme("exit", winIcons.value(259)), tr("Exit"),
                                          player);
        exitAction->setShortcut(QKeySequence("Ctrl+Q"));
        connect(exitAction, &NAction::triggered, QCoreApplication::instance(),
                &QCoreApplication::quit);
        m_contextMenu->addAction(exitAction);
        m_trayIconMenu->addAction(exitAction);
        fileMenu->addAction(exitAction);
    }

    {
        NAction *playingOnTopAction = new NAction(tr("On Top During Playback"), player);
        playingOnTopAction->setCheckable(true);
        playingOnTopAction->setObjectName("PlayingOnTopAction");
        connect(playingOnTopAction, &NAction::toggled, [player](bool checked) {
            player->settings()->setValue("WhilePlayingOnTop", checked);

            bool alwaysOnTop = player->settings()->value("AlwaysOnTop").toBool();
            if (!alwaysOnTop) {
                player->mainWindow()->setOnTop(checked && player->playbackEngine()->state() ==
                                                              N::PlaybackPlaying);
            }
        });
        playingOnTopAction->setChecked(player->settings()->value("WhilePlayingOnTop").toBool());
        windowSubMenu->addAction(playingOnTopAction);
        windowMenu->addAction(playingOnTopAction);
    }

    {
        NAction *alwaysOnTopAction = new NAction(tr("Always On Top"), player);
        alwaysOnTopAction->setCheckable(true);
        alwaysOnTopAction->setObjectName("AlwaysOnTopAction");
        connect(alwaysOnTopAction, &NAction::toggled, [player](bool checked) {
            player->settings()->setValue("AlwaysOnTop", checked);

            bool whilePlaying = player->settings()->value("WhilePlayingOnTop").toBool();
            if (!whilePlaying || player->playbackEngine()->state() != N::PlaybackPlaying) {
                player->mainWindow()->setOnTop(checked);
            }
        });
        alwaysOnTopAction->setChecked(player->settings()->value("AlwaysOnTop").toBool());
        windowSubMenu->addAction(alwaysOnTopAction);
        windowMenu->addAction(alwaysOnTopAction);
    }

    {
        NAction *fullScreenAction = new NAction(tr("Fullscreen Mode"), player);
        fullScreenAction->setStatusTip(tr("Hide all controls except waveform"));
        fullScreenAction->setObjectName("FullScreenAction");
        fullScreenAction->setCustomizable(true);
        connect(fullScreenAction, &NAction::triggered, player->mainWindow(),
                &NMainWindow::toggleFullScreen);
        windowSubMenu->addAction(fullScreenAction);
        windowMenu->addAction(fullScreenAction);
    }

    {
        NAction *shufflePlaylistAction = new NAction(tr("Shuffle"), player);
        shufflePlaylistAction->setObjectName("ShufflePlaylistAction");
        shufflePlaylistAction->setStatusTip(tr("Shuffle items in playlist"));
        shufflePlaylistAction->setCustomizable(true);
        //connect(shufflePlaylistAction, &NAction::triggered, player->playlistWidget(),
        //        &NPlaylistWidget::shufflePlaylist);
        connect(shufflePlaylistAction, &NAction::triggered, player->playlistController(),
                &NPlaylistController::shuffleRows);
        playlistSubMenu->addAction(shufflePlaylistAction);
        controlsPlaylistSubMenu->addAction(shufflePlaylistAction);
    }

    {
        NAction *repeatPlaylistAction = new NAction(tr("Repeat"), player);
        repeatPlaylistAction->setCheckable(true);
        repeatPlaylistAction->setObjectName("RepeatPlaylistAction");
        repeatPlaylistAction->setStatusTip(tr("Toggle current item repeat"));
        repeatPlaylistAction->setCustomizable(true);
        connect(repeatPlaylistAction, &NAction::triggered, player->playlistWidget(),
                &NPlaylistWidget::setRepeatMode);
        connect(player->playlistWidget(), &NPlaylistWidget::repeatModeChanged, repeatPlaylistAction,
                &NAction::setChecked);
        repeatPlaylistAction->setChecked(player->settings()->value("Repeat").toBool());
        playlistSubMenu->addAction(repeatPlaylistAction);
        controlsPlaylistSubMenu->addAction(repeatPlaylistAction);
    }

    {
        NAction *loopPlaylistAction = new NAction(tr("Loop playlist"), player);
        loopPlaylistAction->setCheckable(true);
        loopPlaylistAction->setObjectName("LoopPlaylistAction");
        connect(loopPlaylistAction, &NAction::triggered,
                [player](bool checked) { player->settings()->setValue("LoopPlaylist", checked); });
        loopPlaylistAction->setChecked(player->settings()->value("LoopPlaylist").toBool());
        playlistSubMenu->addAction(loopPlaylistAction);
        controlsPlaylistSubMenu->addAction(loopPlaylistAction);
    }

    {
        NAction *scrollToItemPlaylistAction = new NAction(tr("Scroll to playing item"), player);
        scrollToItemPlaylistAction->setCheckable(true);
        scrollToItemPlaylistAction->setStatusTip(
            tr("Automatically scroll playlist to currently playing item"));
        scrollToItemPlaylistAction->setObjectName("ScrollToItemPlaylistAction");
        connect(scrollToItemPlaylistAction, &NAction::triggered,
                [player](bool checked) { player->settings()->setValue("ScrollToItem", checked); });
        scrollToItemPlaylistAction->setChecked(player->settings()->value("ScrollToItem").toBool());
        playlistSubMenu->addAction(scrollToItemPlaylistAction);
        controlsPlaylistSubMenu->addAction(scrollToItemPlaylistAction);
    }

    {
        NAction *nextFileEnableAction = new NAction(tr("Load next file in directory when finished"),
                                                    player);
        nextFileEnableAction->setCheckable(true);
        nextFileEnableAction->setObjectName("NextFileEnableAction");
        connect(nextFileEnableAction, &NAction::triggered,
                [player](bool checked) { player->settings()->setValue("LoadNext", checked); });
        nextFileEnableAction->setChecked(player->settings()->value("LoadNext").toBool());
        playlistSubMenu->addAction(nextFileEnableAction);
        controlsPlaylistSubMenu->addAction(nextFileEnableAction);

        NAction *nextFileByNameAscdAction =
            new NAction(QString::fromUtf8("    ├  %1 ↓").arg(tr("By Name")), player);
        nextFileByNameAscdAction->setCheckable(true);
        nextFileByNameAscdAction->setObjectName("NextFileByNameAscdAction");
        connect(nextFileByNameAscdAction, &NAction::triggered, [player](bool checked) {
            player->settings()->setValue("LoadNextSort", (int)QDir::Name);
        });
        playlistSubMenu->addAction(nextFileByNameAscdAction);
        controlsPlaylistSubMenu->addAction(nextFileByNameAscdAction);

        NAction *nextFileByNameDescAction =
            new NAction(QString::fromUtf8("    ├  %1 ↑").arg(tr("By Name")), player);
        nextFileByNameDescAction->setCheckable(true);
        nextFileByNameDescAction->setObjectName("NextFileByNameDescAction");
        connect(nextFileByNameDescAction, &NAction::triggered, [player](bool checked) {
            player->settings()->setValue("LoadNextSort", (int)(QDir::Name | QDir::Reversed));
        });
        playlistSubMenu->addAction(nextFileByNameDescAction);
        controlsPlaylistSubMenu->addAction(nextFileByNameDescAction);

        NAction *nextFileByDateAscd =
            new NAction(QString::fromUtf8("    ├  %1 ↓").arg(tr("By Date")), player);
        nextFileByDateAscd->setCheckable(true);
        nextFileByDateAscd->setObjectName("NextFileByDateAscd");
        connect(nextFileByDateAscd, &NAction::triggered, [player](bool checked) {
            player->settings()->setValue("LoadNextSort", (int)(QDir::Time | QDir::Reversed));
        });
        playlistSubMenu->addAction(nextFileByDateAscd);
        controlsPlaylistSubMenu->addAction(nextFileByDateAscd);

        NAction *nextFileByDateDesc =
            new NAction(QString::fromUtf8("    └  %1 ↑").arg(tr("By Date")), player);
        nextFileByDateDesc->setCheckable(true);
        nextFileByDateDesc->setObjectName("NextFileByDateDesc");
        connect(nextFileByDateDesc, &NAction::triggered, [player](bool checked) {
            player->settings()->setValue("LoadNextSort", (int)(QDir::Time));
        });
        playlistSubMenu->addAction(nextFileByDateDesc);
        controlsPlaylistSubMenu->addAction(nextFileByDateDesc);

        QDir::SortFlag flag = (QDir::SortFlag)player->settings()->value("LoadNextSort").toInt();
        if (flag == (QDir::Name | QDir::Reversed)) {
            nextFileByNameDescAction->setChecked(true);
        } else if (flag == (QDir::Time | QDir::Reversed)) {
            nextFileByDateAscd->setChecked(true);
        } else if (flag == (QDir::Time)) {
            nextFileByDateDesc->setChecked(true);
        } else { // flag == (QDir::Name)
            nextFileByNameAscdAction->setChecked(true);
        }

        QActionGroup *loadNextFileGroup = new QActionGroup(player);
        nextFileByNameAscdAction->setActionGroup(loadNextFileGroup);
        nextFileByNameDescAction->setActionGroup(loadNextFileGroup);
        nextFileByDateAscd->setActionGroup(loadNextFileGroup);
        nextFileByDateDesc->setActionGroup(loadNextFileGroup);
    }

    {
        NAction *revealAction = new NAction(QIcon::fromTheme("fileopen", winIcons.value(13)),
                                            tr("Reveal in File Manager..."), player);
        revealAction->setObjectName("RevealInFileManagerAction");
        revealAction->setStatusTip(tr("Open file manager for selected file"));
        revealAction->setCustomizable(true);
        connect(revealAction, &NAction::triggered, [player]() {
            //QStringList files = player->playlistWidget()->selectedFiles();
            QStringList files = player->playlistController()->selectedFiles();
            if (files.isEmpty()) {
                return;
            }

            QString error;
            if (!player->revealInFileManager(files.first(), &error)) {
                QMessageBox::warning(player->mainWindow(), tr("Reveal in File Manager Error"),
                                     error, QMessageBox::Close);
            }
        });
        m_playlistContextMenu->addAction(revealAction);
    }

    {
        NAction *removeSelectedAction = new NAction(QIcon::fromTheme("remove"),
                                                    tr("Remove From Playlist"), player);
        removeSelectedAction->setObjectName("RemoveFromPlaylistAction");
        removeSelectedAction->setStatusTip(tr("Remove selected files from playlist"));
        removeSelectedAction->setCustomizable(true);
        //connect(removeSelectedAction, &NAction::triggered, player->playlistWidget(),
        //        &NPlaylistWidget::removeSelected);
        connect(removeSelectedAction, &NAction::triggered, player->playlistController(),
                &NPlaylistController::removeSelected);
        m_playlistContextMenu->addAction(removeSelectedAction);
    }

    {
        NAction *trashSelectedAction = new NAction(QIcon::fromTheme("trashcan_empty",
                                                                    winIcons.value(49)),
                                                   tr("Move To Trash"), player);
        trashSelectedAction->setObjectName("MoveToTrashAction");
        trashSelectedAction->setStatusTip(tr("Move selected files to trash bin"));
        trashSelectedAction->setCustomizable(true);
        connect(trashSelectedAction, &NAction::triggered, [player]() {
            //QStringList files = player->playlistWidget()->selectedFiles();
            QStringList files = player->playlistController()->selectedFiles();
            if (files.isEmpty()) {
                return;
            }

            QStringList deleted = NTrash::moveToTrash(files);
            //player->playlistWidget()->removeFiles(deleted);
            player->playlistController()->removeFiles(deleted);
        });
        m_playlistContextMenu->addAction(trashSelectedAction);
    }

    if (player->tagReader()->isWriteSupported()) {
        NAction *tagEditorAction = new NAction(QIcon::fromTheme("edit", winIcons.value(289)),
                                               tr("Tag Editor"), player);
        tagEditorAction->setObjectName("TagEditorAction");
        tagEditorAction->setStatusTip(tr("Open tag editor for selected file"));
        tagEditorAction->setCustomizable(true);
        connect(tagEditorAction, &NAction::triggered, [player]() {
            //QStringList files = player->playlistWidget()->selectedFiles();
            QStringList files = player->playlistController()->selectedFiles();
            if (files.isEmpty()) {
                return;
            }
            player->showTagEditor(files.first());
        });
        m_playlistContextMenu->addAction(tagEditorAction);
    }

    for (int i = 1; i <= 3; ++i) {
        QString num = QString::number(i);

        NAction *jumpForwardAction = new NAction(tr("Jump Forward #%1").arg(num), player);
        jumpForwardAction->setObjectName(QString("Jump%1ForwardAction").arg(num));
        jumpForwardAction->setStatusTip(tr("Make a jump forward #%1").arg(num));
        jumpForwardAction->setCustomizable(true);
        connect(jumpForwardAction, &NAction::triggered, [player, num]() {
            qreal seconds = player->settings()->value(QString("Jump%1").arg(num)).toDouble();
            player->playbackEngine()->jump(seconds * 1000);
        });

        NAction *jumpBackwardsAction = new NAction(tr("Jump Backwards #%1").arg(num), player);
        jumpBackwardsAction->setObjectName(QString("Jump%1BackwardsAction").arg(num));
        jumpBackwardsAction->setStatusTip(tr("Make a jump backwards #%1").arg(num));
        jumpBackwardsAction->setCustomizable(true);
        connect(jumpBackwardsAction, &NAction::triggered, [player, num]() {
            qreal seconds = player->settings()->value(QString("Jump%1").arg(num)).toDouble();
            player->playbackEngine()->jump(-seconds * 1000);
        });
    }

    {
        NAction *speedIncreaseAction = new NAction(tr("Speed Increase"), player);
        speedIncreaseAction->setObjectName("SpeedIncreaseAction");
        speedIncreaseAction->setStatusTip(tr("Increase playback speed"));
        speedIncreaseAction->setCustomizable(true);
        connect(speedIncreaseAction, &NAction::triggered, [player]() {
            qreal newSpeed = qMax(0.01, player->playbackEngine()->speed() +
                                            player->settings()->value("SpeedStep").toDouble());
            player->playbackEngine()->setSpeed(newSpeed);
            player->showToolTip(tr("Speed: %1").arg(player->playbackEngine()->speed()));
        });

        NAction *speedDecreaseAction = new NAction(tr("Speed Decrease"), player);
        speedDecreaseAction->setObjectName("SpeedDecreaseAction");
        speedDecreaseAction->setStatusTip(tr("Decrease playback speed"));
        speedDecreaseAction->setCustomizable(true);
        connect(speedDecreaseAction, &NAction::triggered, [player]() {
            qreal newSpeed = qMax(0.01, player->playbackEngine()->speed() -
                                            player->settings()->value("SpeedStep").toDouble());
            player->playbackEngine()->setSpeed(newSpeed);
            player->showToolTip(tr("Speed: %1").arg(player->playbackEngine()->speed()));
        });

        NAction *speedResetAction = new NAction(tr("Speed Reset"), player);
        speedResetAction->setObjectName("SpeedResetAction");
        speedResetAction->setStatusTip(tr("Reset playback speed to 1.0"));
        speedResetAction->setCustomizable(true);
        connect(speedResetAction, &NAction::triggered, [player]() {
            qreal newSpeed = 1.0;
            player->playbackEngine()->setSpeed(newSpeed);
            player->showToolTip(tr("Speed: %1").arg(player->playbackEngine()->speed()));
        });
    }

    /*
    {
        NAction *pitchIncreaseAction = new NAction(tr("Pitch Increase"), player);
        pitchIncreaseAction->setObjectName("PitchIncreaseAction");
        pitchIncreaseAction->setStatusTip(tr("Increase playback pitch"));
        pitchIncreaseAction->setCustomizable(true);
        connect(pitchIncreaseAction, &NAction::triggered, [player]() {
            qreal newPitch = qMax(0.01, player->playbackEngine()->pitch() +
                                            player->settings()->value("PitchStep").toDouble());
            player->playbackEngine()->setPitch(newPitch);
            player->showToolTip(tr("Pitch: %1").arg(player->playbackEngine()->pitch()));
        });

        NAction *pitchDecreaseAction = new NAction(tr("Pitch Decrease"), player);
        pitchDecreaseAction->setObjectName("PitchDecreaseAction");
        pitchDecreaseAction->setStatusTip(tr("Decrease playback pitch"));
        pitchDecreaseAction->setCustomizable(true);
        connect(pitchDecreaseAction, &NAction::triggered, [player]() {
            qreal newPitch = qMax(0.01, player->playbackEngine()->pitch() -
                                            player->settings()->value("PitchStep").toDouble());
            player->playbackEngine()->setPitch(newPitch);
            player->showToolTip(tr("Pitch: %1").arg(player->playbackEngine()->pitch()));
        });

        NAction *pitchResetAction = new NAction(tr("Pitch Reset"), player);
        pitchResetAction->setObjectName("PitchResetAction");
        pitchResetAction->setStatusTip(tr("Reset pitch to 1.0"));
        pitchResetAction->setCustomizable(true);
        connect(pitchResetAction, &NAction::triggered, [player]() {
            qreal newPitch = 1.0;
            player->playbackEngine()->setPitch(newPitch);
            player->showToolTip(tr("Pitch: %1").arg(player->playbackEngine()->pitch()));
        });
    }
    */

    for (NAction *action : m_player->findChildren<NAction *>()) {
#ifdef Q_OS_MAC
        // remove icons for macOS:
        action->setIcon(QIcon());
#endif
        if (action->objectName().isEmpty() || !action->isCustomizable()) {
            continue;
        }
        // load settings:
        action->setShortcuts(
            player->settings()->value("Shortcuts/" + action->objectName()).toStringList());
        action->setGlobalShortcuts(
            player->settings()->value("GlobalShortcuts/" + action->objectName()).toStringList());
    }
}

void NActionManager::saveSettings()
{
    for (NAction *action : m_player->findChildren<NAction *>()) {
        if (action->objectName().isEmpty() || !action->isCustomizable()) {
            continue;
        }
        m_player->settings()->setValue("Shortcuts/" + action->objectName(), action->shortcuts());
        m_player->settings()->setValue("GlobalShortcuts/" + action->objectName(),
                                       action->globalShortcuts());
    }
}

QMenu *NActionManager::contextMenu()
{
    return m_contextMenu;
}

QMenu *NActionManager::playlistContextMenu()
{
    return m_playlistContextMenu;
}

QMenu *NActionManager::trayIconMenu()
{
    return m_trayIconMenu;
}
