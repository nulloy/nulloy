/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2022 Sergey Vlasov <sergey@vlasov.me>
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

#include "player.h"

#include "aboutDialog.h"
#include "action.h"
#include "common.h"
#include "i18nLoader.h"
#include "logDialog.h"
#include "mainWindow.h"
#include "playbackEngineInterface.h"
#include "playlistDataItem.h"
#include "playlistStorage.h"
#include "playlistWidget.h"
#include "playlistWidgetItem.h"
#include "pluginLoader.h"
#include "preferencesDialog.h"
#include "scriptEngine.h"
#include "settings.h"
#include "tagReaderInterface.h"
#include "trackInfoWidget.h"
#include "utils.h"
#include "volumeSlider.h"
#include "waveformSlider.h"

#ifndef _N_NO_SKINS_
#include "skinFileSystem.h"
#include "skinLoader.h"
#endif

#ifdef Q_OS_WIN
#include "w7TaskBar.h"
#endif

#ifdef Q_OS_MAC
#include "macDock.h"
#endif

#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMetaObject>
#include <QResizeEvent>

#ifndef _N_NO_UPDATE_CHECK_
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#endif

NPlayer::NPlayer()
{
    qsrand((uint)QTime::currentTime().msec());
    m_settings = NSettings::instance();

    NI18NLoader::init();
    NPluginLoader::init();

    m_playbackEngine = dynamic_cast<NPlaybackEngineInterface *>(
        NPluginLoader::getPlugin(N::PlaybackEngine));
    Q_ASSERT(m_playbackEngine);
    m_playbackEngine->setParent(this);

#ifndef _N_NO_SKINS_
    m_mainWindow = new NMainWindow(NSkinLoader::skinUiFormFile());
#else
    m_mainWindow = new NMainWindow();
#endif

    // loading skin script
    m_scriptEngine = new NScriptEngine(this);
#ifndef _N_NO_SKINS_
    QString scriptFileName(NSkinLoader::skinScriptFile());
#else
    QString scriptFileName(":skins/native/script.js");
#endif
    QFile scriptFile(scriptFileName);
    scriptFile.open(QIODevice::ReadOnly);
    m_scriptEngine->evaluate(scriptFile.readAll(), scriptFileName);
    scriptFile.close();
    QScriptValue skinProgram = m_scriptEngine->evaluate("Main").construct();

    m_aboutDialog = NULL;
    m_logDialog = new NLogDialog(m_mainWindow);
    m_preferencesDialog = new NPreferencesDialog(m_mainWindow);
    m_volumeSlider = m_mainWindow->findChild<NVolumeSlider *>("volumeSlider");
    m_coverWidget = m_mainWindow->findChild<QWidget *>("coverWidget");

    m_playlistWidget = m_mainWindow->findChild<NPlaylistWidget *>("playlistWidget");
    if (QAbstractButton *repeatButton = m_mainWindow->findChild<QAbstractButton *>(
            "repeatButton")) {
        repeatButton->setChecked(m_playlistWidget->repeatMode());
    }

    m_trackInfoWidget = new NTrackInfoWidget();
    QVBoxLayout *trackInfoLayout = new QVBoxLayout;
    trackInfoLayout->setContentsMargins(0, 0, 0, 0);
    trackInfoLayout->addWidget(m_trackInfoWidget);
    m_waveformSlider = m_mainWindow->findChild<NWaveformSlider *>("waveformSlider");
    m_waveformSlider->setLayout(trackInfoLayout);

#ifndef _N_NO_UPDATE_CHECK_
    m_versionDownloader = new QNetworkAccessManager(this);
    connect(m_versionDownloader, SIGNAL(finished(QNetworkReply *)), this,
            SLOT(on_versionDownloader_finished(QNetworkReply *)));
    connect(m_preferencesDialog, SIGNAL(versionRequested()), this, SLOT(downloadVersion()));
#endif

    createActions();
    loadSettings();
    connectSignals();

#ifdef Q_OS_WIN
    NW7TaskBar::instance()->setWindow(m_mainWindow);
    NW7TaskBar::instance()->setEnabled(NSettings::instance()->value("TaskbarProgress").toBool());
    connect(m_playbackEngine, SIGNAL(positionChanged(qreal)), NW7TaskBar::instance(),
            SLOT(setProgress(qreal)));
#endif

#ifdef Q_OS_MAC
    NMacDock::instance()->registerClickHandler();
    connect(NMacDock::instance(), SIGNAL(clicked()), m_mainWindow, SLOT(show()));
#endif

    m_mainWindow->setTitle(QCoreApplication::applicationName() + " " +
                           QCoreApplication::applicationVersion());
    m_mainWindow->show();
    m_mainWindow->loadSettings();
    QResizeEvent e(m_mainWindow->size(), m_mainWindow->size());
    QCoreApplication::sendEvent(m_mainWindow, &e);

    skinProgram.property("afterShow").call(skinProgram);
}

NPlayer::~NPlayer()
{
    NPluginLoader::deinit();
    delete m_mainWindow;
    delete m_settings;
}

void NPlayer::createActions()
{
    m_showHideAction =
        new NAction(QIcon::fromTheme("preferences-system-windows",
                                     QIcon(
                                         ":/trolltech/styles/commonstyle/images/dockdock-16.png")),
                    tr("Show / Hide"), this);
    m_showHideAction->setObjectName("ShowHideAction");
    m_showHideAction->setStatusTip(tr("Toggle window visibility"));
    m_showHideAction->setCustomizable(true);

    m_playAction = new NAction(QIcon::fromTheme("media-playback-start",
                                                style()->standardIcon(QStyle::SP_MediaPlay)),
                               tr("Play / Pause"), this);
    m_playAction->setObjectName("PlayAction");
    m_playAction->setStatusTip(tr("Toggle playback"));
    m_playAction->setCustomizable(true);

    m_stopAction = new NAction(QIcon::fromTheme("media-playback-stop",
                                                style()->standardIcon(QStyle::SP_MediaStop)),
                               tr("Stop"), this);
    m_stopAction->setObjectName("StopAction");
    m_stopAction->setStatusTip(tr("Stop playback"));
    m_stopAction->setCustomizable(true);

    m_prevAction = new NAction(QIcon::fromTheme("media-playback-backward",
                                                style()->standardIcon(QStyle::SP_MediaSkipBackward)),
                               tr("Previous"), this);
    m_prevAction->setObjectName("PrevAction");
    m_prevAction->setStatusTip(tr("Play previous track in playlist"));
    m_prevAction->setCustomizable(true);

    m_nextAction = new NAction(QIcon::fromTheme("media-playback-forward",
                                                style()->standardIcon(QStyle::SP_MediaSkipForward)),
                               tr("Next"), this);
    m_nextAction->setObjectName("NextAction");
    m_nextAction->setStatusTip(tr("Play next track in playlist"));
    m_nextAction->setCustomizable(true);

    m_preferencesAction = new NAction(QIcon::fromTheme("preferences-desktop",
                                                       style()->standardIcon(
                                                           QStyle::SP_MessageBoxInformation)),
                                      tr("Preferences..."), this);
    m_preferencesAction->setShortcut(QKeySequence("Ctrl+P"));

    m_exitAction = new NAction(QIcon::fromTheme("exit", style()->standardIcon(
                                                            QStyle::SP_DialogCloseButton)),
                               tr("Exit"), this);
    m_exitAction->setShortcut(QKeySequence("Ctrl+Q"));

    m_addFilesAction = new NAction(style()->standardIcon(QStyle::SP_DialogOpenButton),
                                   tr("Add Files") + "...", this);
    m_addFilesAction->setShortcut(QKeySequence("Ctrl+O"));

    m_addDirAction = new NAction(style()->standardIcon(QStyle::SP_FileDialogNewFolder),
                                 tr("Add Directory") + "...", this);
    m_addDirAction->setShortcut(QKeySequence("Ctrl+Shift+O"));

    m_savePlaylistAction = new NAction(style()->standardIcon(QStyle::SP_DialogSaveButton),
                                       tr("Save Playlist") + "...", this);
    m_savePlaylistAction->setShortcut(QKeySequence("Ctrl+S"));

    m_showCoverAction = new NAction(tr("Show Cover Art"), this);
    m_showCoverAction->setCheckable(true);
    m_showCoverAction->setObjectName("ShowCoverAction");

    m_showPlaybackControlsAction = new NAction(tr("Show Playback Controls"), this);
    m_showPlaybackControlsAction->setCheckable(true);
    m_showPlaybackControlsAction->setObjectName("ShowPlaybackControls");

    m_aboutAction = new NAction(QIcon::fromTheme("help-about", style()->standardIcon(
                                                                   QStyle::SP_MessageBoxQuestion)),
                                tr("About"), this);

    m_playingOnTopAction = new NAction(tr("On Top During Playback"), this);
    m_playingOnTopAction->setCheckable(true);
    m_playingOnTopAction->setObjectName("PlayingOnTopAction");

    m_alwaysOnTopAction = new NAction(tr("Always On Top"), this);
    m_alwaysOnTopAction->setCheckable(true);
    m_alwaysOnTopAction->setObjectName("AlwaysOnTopAction");

    m_fullScreenAction = new NAction(tr("Fullscreen Mode"), this);
    m_fullScreenAction->setStatusTip(tr("Hide all controls except waveform"));
    m_fullScreenAction->setObjectName("FullScreenAction");
    m_fullScreenAction->setCustomizable(true);

    // playlist actions >>
    m_shufflePlaylistAction = new NAction(tr("Shuffle"), this);
    m_shufflePlaylistAction->setCheckable(true);
    m_shufflePlaylistAction->setObjectName("ShufflePlaylistAction");
    m_shufflePlaylistAction->setStatusTip(tr("Toggle playlist shuffle"));
    m_shufflePlaylistAction->setCustomizable(true);

    m_repeatPlaylistAction = new NAction(tr("Repeat"), this);
    m_repeatPlaylistAction->setCheckable(true);
    m_repeatPlaylistAction->setObjectName("RepeatPlaylistAction");
    m_repeatPlaylistAction->setStatusTip(tr("Toggle current item repeat"));
    m_repeatPlaylistAction->setCustomizable(true);

    m_loopPlaylistAction = new NAction(tr("Loop playlist"), this);
    m_loopPlaylistAction->setCheckable(true);
    m_loopPlaylistAction->setObjectName("LoopPlaylistAction");

    m_nextFileEnableAction = new NAction(tr("Load next file in directory when finished"), this);
    m_nextFileEnableAction->setCheckable(true);
    m_nextFileEnableAction->setObjectName("NextFileEnableAction");

    m_nextFileByNameAscdAction = new NAction(QString::fromUtf8("    ├  %1 ↓").arg(tr("By Name")),
                                             this);
    m_nextFileByNameAscdAction->setCheckable(true);
    m_nextFileByNameAscdAction->setObjectName("NextFileByNameAscdAction");

    m_nextFileByNameDescAction = new NAction(QString::fromUtf8("    ├  %1 ↑").arg(tr("By Name")),
                                             this);
    m_nextFileByNameDescAction->setCheckable(true);
    m_nextFileByNameDescAction->setObjectName("NextFileByNameDescAction");

    m_nextFileByDateAscd = new NAction(QString::fromUtf8("    ├  %1 ↓").arg(tr("By Date")), this);
    m_nextFileByDateAscd->setCheckable(true);
    m_nextFileByDateAscd->setObjectName("NextFileByDateAscd");

    m_nextFileByDateDesc = new NAction(QString::fromUtf8("    └  %1 ↑").arg(tr("By Date")), this);
    m_nextFileByDateDesc->setCheckable(true);
    m_nextFileByDateDesc->setObjectName("NextFileByDateDesc");

    QActionGroup *group = new QActionGroup(this);
    m_nextFileByNameAscdAction->setActionGroup(group);
    m_nextFileByNameDescAction->setActionGroup(group);
    m_nextFileByDateAscd->setActionGroup(group);
    m_nextFileByDateDesc->setActionGroup(group);
    // << playlist actions

    // jump actions >>
    for (int i = 1; i <= 3; ++i) {
        QString num = QString::number(i);

        NAction *jumpFwAction = new NAction(QString(tr("Jump Forward #%1")).arg(num), this);
        jumpFwAction->setObjectName(QString("Jump%1ForwardAction").arg(num));
        jumpFwAction->setStatusTip(QString(tr("Make a jump forward #%1")).arg(num));
        jumpFwAction->setCustomizable(true);

        NAction *jumpBwAction = new NAction(QString(tr("Jump Backwards #%1")).arg(num), this);
        jumpBwAction->setObjectName(QString("Jump%1BackwardsAction").arg(num));
        jumpBwAction->setStatusTip(QString(tr("Make a jump backwards #%1")).arg(num));
        jumpBwAction->setCustomizable(true);
    }
    // << jump actions

    // keyboard shortcuts
    m_settings->initShortcuts(this);
    m_settings->loadShortcuts();
    foreach (NAction *action, findChildren<NAction *>()) {
        if (!action->shortcuts().isEmpty()) {
            m_mainWindow->addAction(action);
        }
    }

    createContextMenu();
    createGlobalMenu();
    createTrayIcon();
}

NMainWindow *NPlayer::mainWindow()
{
    return m_mainWindow;
}

void NPlayer::createContextMenu()
{
    m_contextMenu = new QMenu(m_mainWindow);
    m_mainWindow->setContextMenuPolicy(Qt::CustomContextMenu);
    m_contextMenu->addAction(m_addFilesAction);
    m_contextMenu->addAction(m_addDirAction);
    m_contextMenu->addAction(m_savePlaylistAction);

    m_windowSubMenu = new QMenu(tr("Window"), m_mainWindow);
    m_windowSubMenu->addAction(m_showCoverAction);
    m_windowSubMenu->addAction(m_showPlaybackControlsAction);
    m_windowSubMenu->addAction(m_playingOnTopAction);
    m_windowSubMenu->addAction(m_alwaysOnTopAction);
    m_windowSubMenu->addAction(m_fullScreenAction);
    m_contextMenu->addMenu(m_windowSubMenu);

    m_playlistSubMenu = new QMenu(tr("Playlist"), m_mainWindow);
    m_playlistSubMenu->addAction(m_shufflePlaylistAction);
    m_playlistSubMenu->addAction(m_repeatPlaylistAction);
    m_playlistSubMenu->addAction(m_loopPlaylistAction);
    m_playlistSubMenu->addAction(m_nextFileEnableAction);
    m_playlistSubMenu->addAction(m_nextFileByNameAscdAction);
    m_playlistSubMenu->addAction(m_nextFileByNameDescAction);
    m_playlistSubMenu->addAction(m_nextFileByDateAscd);
    m_playlistSubMenu->addAction(m_nextFileByDateDesc);
    m_contextMenu->addMenu(m_playlistSubMenu);

    m_contextMenu->addAction(m_preferencesAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_aboutAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_exitAction);
}

void NPlayer::createGlobalMenu()
{
#ifdef Q_OS_MAC
    // removing icons from context menu
    QList<NAction *> actions = findChildren<NAction *>();
    for (int i = 0; i < actions.size(); ++i)
        actions.at(i)->setIcon(QIcon());

    QMenuBar *menuBar = new QMenuBar(m_mainWindow);

    QMenu *fileMenu = menuBar->addMenu(tr("File"));
    fileMenu->addAction(m_addFilesAction);
    fileMenu->addAction(m_addDirAction);
    fileMenu->addAction(m_savePlaylistAction);
    fileMenu->addAction(m_aboutAction);
    fileMenu->addAction(m_exitAction);
    fileMenu->addAction(m_preferencesAction);

    QMenu *controlsMenu = menuBar->addMenu(tr("Controls"));
    controlsMenu->addAction(m_playAction);
    controlsMenu->addAction(m_stopAction);
    controlsMenu->addAction(m_prevAction);
    controlsMenu->addAction(m_nextAction);
    controlsMenu->addSeparator();

    QMenu *playlistSubMenu = controlsMenu->addMenu(tr("Playlist"));
    playlistSubMenu->addAction(m_shufflePlaylistAction);
    playlistSubMenu->addAction(m_repeatPlaylistAction);
    playlistSubMenu->addAction(m_loopPlaylistAction);
    playlistSubMenu->addAction(m_nextFileEnableAction);
    playlistSubMenu->addAction(m_nextFileByNameAscdAction);
    playlistSubMenu->addAction(m_nextFileByNameDescAction);
    playlistSubMenu->addAction(m_nextFileByDateAscd);
    playlistSubMenu->addAction(m_nextFileByDateDesc);
    controlsMenu->addMenu(playlistSubMenu);

    QMenu *windowMenu = menuBar->addMenu(tr("Window"));
    windowMenu->addAction(m_showCoverAction);
    windowMenu->addAction(m_showPlaybackControlsAction);
    windowMenu->addAction(m_playingOnTopAction);
    windowMenu->addAction(m_alwaysOnTopAction);
    windowMenu->addAction(m_fullScreenAction);
#endif
}

void NPlayer::createTrayIcon()
{
    QMenu *trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(m_showHideAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(m_playAction);
    trayIconMenu->addAction(m_stopAction);
    trayIconMenu->addAction(m_prevAction);
    trayIconMenu->addAction(m_nextAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(m_preferencesAction);
    trayIconMenu->addAction(m_exitAction);
    m_systemTray = new QSystemTrayIcon(this);
    m_systemTray->setContextMenu(trayIconMenu);
#ifdef Q_OS_MAC
    m_systemTray->setIcon(QIcon(":mac-systray.png"));
#else
    m_systemTray->setIcon(m_mainWindow->windowIcon());
#endif
    m_trayClickTimer = new QTimer(this);
    m_trayClickTimer->setSingleShot(true);
}

void NPlayer::connectSignals()
{
    connect(m_playbackEngine, SIGNAL(mediaChanged(const QString &)), this,
            SLOT(on_playbackEngine_mediaChanged(const QString &)));
    connect(m_playbackEngine, SIGNAL(mediaChanged(const QString &)), m_waveformSlider,
            SLOT(setMedia(const QString &)));
    connect(m_playbackEngine, SIGNAL(stateChanged(N::PlaybackState)), this,
            SLOT(on_playbackEngine_stateChanged(N::PlaybackState)));
    connect(m_playbackEngine, SIGNAL(aboutToFinish()), m_playlistWidget, SLOT(currentFinished()),
            Qt::BlockingQueuedConnection);
    connect(m_playbackEngine, SIGNAL(positionChanged(qreal)), m_waveformSlider,
            SLOT(setValue(qreal)));
    connect(m_playbackEngine, SIGNAL(tick(qint64)), m_trackInfoWidget, SLOT(tick(qint64)));
    connect(m_playbackEngine, SIGNAL(finished()), m_playlistWidget, SLOT(currentFinished()));
    connect(m_playbackEngine, SIGNAL(failed()), this, SLOT(on_playbackEngine_failed()));
    connect(m_playbackEngine, SIGNAL(message(N::MessageIcon, const QString &, const QString &)),
            m_logDialog, SLOT(showMessage(N::MessageIcon, const QString &, const QString &)));

    connect(m_mainWindow, SIGNAL(closed()), this, SLOT(on_mainWindow_closed()));

    connect(m_preferencesDialog, SIGNAL(settingsChanged()), this,
            SLOT(on_preferencesDialog_settingsChanged()));

    if (m_coverWidget) {
        connect(m_playbackEngine, SIGNAL(mediaChanged(const QString &)), m_coverWidget,
                SLOT(setSource(const QString &)));
    }

    if (QAbstractButton *playButton = m_mainWindow->findChild<QAbstractButton *>("playButton")) {
        connect(playButton, SIGNAL(clicked()), this, SLOT(on_playButton_clicked()));
    }

    if (QAbstractButton *stopButton = m_mainWindow->findChild<QAbstractButton *>("stopButton")) {
        connect(stopButton, SIGNAL(clicked()), m_playbackEngine, SLOT(stop()));
    }

    if (QAbstractButton *prevButton = m_mainWindow->findChild<QAbstractButton *>("prevButton")) {
        connect(prevButton, SIGNAL(clicked()), m_playlistWidget, SLOT(playPrevItem()));
    }

    if (QAbstractButton *nextButton = m_mainWindow->findChild<QAbstractButton *>("nextButton")) {
        connect(nextButton, SIGNAL(clicked()), m_playlistWidget, SLOT(playNextItem()));
    }

    if (QAbstractButton *closeButton = m_mainWindow->findChild<QAbstractButton *>("closeButton")) {
        connect(closeButton, SIGNAL(clicked()), m_mainWindow, SLOT(close()));
    }

    if (QAbstractButton *minimizeButton = m_mainWindow->findChild<QAbstractButton *>(
            "minimizeButton")) {
        connect(minimizeButton, SIGNAL(clicked()), m_mainWindow, SLOT(showMinimized()));
    }

    if (m_volumeSlider) {
        connect(m_volumeSlider, SIGNAL(sliderMoved(qreal)), m_playbackEngine,
                SLOT(setVolume(qreal)));
        connect(m_playbackEngine, SIGNAL(volumeChanged(qreal)), m_volumeSlider,
                SLOT(setValue(qreal)));
        connect(m_mainWindow, SIGNAL(scrolled(int)), this, SLOT(on_mainWindow_scrolled(int)));
    }

    if (QAbstractButton *repeatButton = m_mainWindow->findChild<QAbstractButton *>(
            "repeatButton")) {
        connect(repeatButton, SIGNAL(clicked(bool)), m_playlistWidget, SLOT(setRepeatMode(bool)));
        connect(m_playlistWidget, SIGNAL(repeatModeChanged(bool)), repeatButton,
                SLOT(setChecked(bool)));
    }

    if (QAbstractButton *shuffleButton = m_mainWindow->findChild<QAbstractButton *>(
            "shuffleButton")) {
        connect(shuffleButton, SIGNAL(clicked(bool)), m_playlistWidget, SLOT(setShuffleMode(bool)));
        connect(m_playlistWidget, SIGNAL(shuffleModeChanged(bool)), shuffleButton,
                SLOT(setChecked(bool)));
    }

    connect(m_playlistWidget, SIGNAL(setMedia(const QString &)), m_playbackEngine,
            SLOT(setMedia(const QString &)));
    connect(m_playlistWidget, SIGNAL(currentActivated()), m_playbackEngine, SLOT(play()));
    connect(m_playlistWidget, SIGNAL(shuffleModeChanged(bool)), m_shufflePlaylistAction,
            SLOT(setChecked(bool)));
    connect(m_playlistWidget, SIGNAL(repeatModeChanged(bool)), m_repeatPlaylistAction,
            SLOT(setChecked(bool)));

    connect(m_waveformSlider, SIGNAL(filesDropped(const QList<NPlaylistDataItem> &)),
            m_playlistWidget, SLOT(playItems(const QList<NPlaylistDataItem> &)));
    connect(m_waveformSlider, SIGNAL(sliderMoved(qreal)), m_playbackEngine,
            SLOT(setPosition(qreal)));

    connect(m_showHideAction, SIGNAL(triggered()), this, SLOT(toggleWindowVisibility()));
    connect(m_playAction, SIGNAL(triggered()), m_playbackEngine, SLOT(play()));
    connect(m_stopAction, SIGNAL(triggered()), m_playbackEngine, SLOT(stop()));
    connect(m_prevAction, SIGNAL(triggered()), m_playlistWidget, SLOT(playPrevItem()));
    connect(m_nextAction, SIGNAL(triggered()), m_playlistWidget, SLOT(playNextItem()));
    connect(m_preferencesAction, SIGNAL(triggered()), m_preferencesDialog, SLOT(exec()));
    connect(m_exitAction, SIGNAL(triggered()), this, SLOT(quit()));
    connect(m_addFilesAction, SIGNAL(triggered()), this, SLOT(showOpenFileDialog()));
    connect(m_addDirAction, SIGNAL(triggered()), this, SLOT(showOpenDirDialog()));
    connect(m_savePlaylistAction, SIGNAL(triggered()), this, SLOT(showSavePlaylistDialog()));
    connect(m_showCoverAction, SIGNAL(toggled(bool)), this, SLOT(on_showCoverAction_toggled(bool)));
    connect(m_showPlaybackControlsAction, SIGNAL(toggled(bool)), m_mainWindow,
            SLOT(showPlaybackControls(bool)));
    connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(showAboutMessageBox()));
    connect(m_playingOnTopAction, SIGNAL(toggled(bool)), this,
            SLOT(on_whilePlayingOnTopAction_toggled(bool)));
    connect(m_alwaysOnTopAction, SIGNAL(toggled(bool)), this,
            SLOT(on_alwaysOnTopAction_toggled(bool)));
    connect(m_fullScreenAction, SIGNAL(triggered()), m_mainWindow, SLOT(toggleFullScreen()));
    connect(m_shufflePlaylistAction, SIGNAL(triggered(bool)), m_playlistWidget,
            SLOT(setShuffleMode(bool)));
    connect(m_repeatPlaylistAction, SIGNAL(triggered(bool)), m_playlistWidget,
            SLOT(setRepeatMode(bool)));
    connect(m_loopPlaylistAction, SIGNAL(triggered()), this, SLOT(on_playlistAction_triggered()));
    connect(m_nextFileEnableAction, SIGNAL(triggered()), this, SLOT(on_playlistAction_triggered()));
    connect(m_nextFileByNameAscdAction, SIGNAL(triggered()), this,
            SLOT(on_playlistAction_triggered()));
    connect(m_nextFileByNameDescAction, SIGNAL(triggered()), this,
            SLOT(on_playlistAction_triggered()));
    connect(m_nextFileByDateAscd, SIGNAL(triggered()), this, SLOT(on_playlistAction_triggered()));
    connect(m_nextFileByDateDesc, SIGNAL(triggered()), this, SLOT(on_playlistAction_triggered()));

    foreach (NAction *action, findChildren<NAction *>()) {
        if (action->objectName().startsWith("Jump")) {
            connect(action, SIGNAL(triggered()), this, SLOT(on_jumpAction_triggered()));
        }
    }

    connect(m_mainWindow, SIGNAL(customContextMenuRequested(QPoint)), this,
            SLOT(showContextMenu(QPoint)));
    connect(m_systemTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this,
            SLOT(on_trayIcon_activated(QSystemTrayIcon::ActivationReason)));
    connect(m_trayClickTimer, SIGNAL(timeout()), this, SLOT(on_trayClickTimer_timeout()));
}

NPlaybackEngineInterface *NPlayer::playbackEngine()
{
    return m_playbackEngine;
}

bool NPlayer::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FileOpen) {
        QFileOpenEvent *fileEvent = static_cast<QFileOpenEvent *>(event);

        if (!fileEvent->file().isEmpty()) {
            m_playlistWidget->playFiles(QStringList() << fileEvent->file());
        }

        return false;
    }

    return QObject::eventFilter(obj, event);
}

void NPlayer::readMessage(const QString &str)
{
    if (str.isEmpty()) {
        m_mainWindow->show();
        m_mainWindow->activateWindow();
        m_mainWindow->raise();
        return;
    }
    QStringList argList = str.split(MSG_SPLITTER);
    QStringList files;
    QStringList options;
    foreach (QString arg, argList) {
        if (arg.startsWith("--")) {
            options << arg;
        } else if (QFile(arg).exists()) {
            files << arg;
        }
    }

    foreach (QString arg, options) {
        if (arg == "--next") {
            m_playlistWidget->playNextItem();
            return;
        } else if (arg == "--prev") {
            m_playlistWidget->playPrevItem();
            return;
        } else if (arg == "--stop") {
            m_playbackEngine->stop();
            return;
        } else if (arg == "--pause") {
            m_playbackEngine->play();
            return;
        }
    }

    if (!files.isEmpty()) {
        if (NSettings::instance()->value("EnqueueFiles").toBool()) {
            int lastRow = m_playlistWidget->count();
            m_playlistWidget->addFiles(files);
            if (m_playbackEngine->state() == N::PlaybackStopped ||
                NSettings::instance()->value("PlayEnqueued").toBool()) {
                m_playlistWidget->playRow(lastRow);
                m_playbackEngine->setPosition(0); // overrides setPosition() in loadDefaultPlaylist()
            }
        } else {
            m_playlistWidget->playFiles(files);
        }

        // re-shuffle
        m_playlistWidget->setShuffleMode(NSettings::instance()->value("Shuffle").toBool());
    }
}

void NPlayer::loadDefaultPlaylist()
{
    if (!QFileInfo(NCore::defaultPlaylistPath()).exists() ||
        !m_playlistWidget->setPlaylist(NCore::defaultPlaylistPath())) {
        return;
    }

    QStringList playlistRowValues = m_settings->value("PlaylistRow").toStringList();
    if (!playlistRowValues.isEmpty()) {
        int row = playlistRowValues.at(0).toInt();
        qreal pos = playlistRowValues.at(1).toFloat();

        if (row > m_playlistWidget->count() - 1) {
            return;
        }

        if (pos > 0 && pos < 1) {
            m_playlistWidget->playRow(row);
            m_playbackEngine->setPosition(pos); // postponed till file duration is available
            if (m_settings->value("StartPaused").toBool()) {
                m_playbackEngine->pause();
            }
        } else {
            m_playlistWidget->setCurrentRow(row);
        }
    }

    m_playlistWidget->setShuffleMode(NSettings::instance()->value("Shuffle").toBool());
}

void NPlayer::writePlaylist(const QString &file, N::M3uExtention ext)
{
    QList<NPlaylistDataItem> dataItemsList;
    for (int i = 0; i < m_playlistWidget->count(); ++i) {
        NPlaylistDataItem dataItem = m_playlistWidget->item(i)->dataItem();
        dataItem.title = m_playlistWidget->item(i)->text();
        dataItemsList << dataItem;
    }
    NPlaylistStorage::writeM3u(file, dataItemsList, ext);
}

void NPlayer::saveDefaultPlaylist()
{
    writePlaylist(NCore::defaultPlaylistPath(), N::NulloyM3u);

    int row = m_playlistWidget->currentRow();
    qreal pos = m_playbackEngine->position();
    m_settings->setValue("PlaylistRow", QStringList()
                                            << QString::number(row) << QString::number(pos));
}

void NPlayer::loadSettings()
{
    m_systemTray->setVisible(m_settings->value("TrayIcon").toBool());

#ifndef _N_NO_UPDATE_CHECK_
    if (m_settings->value("AutoCheckUpdates").toBool()) {
        downloadVersion();
    }
#endif

    m_showCoverAction->setChecked(m_settings->value("ShowCoverArt").toBool());
    if (m_coverWidget) {
        m_coverWidget->setEnabled(m_settings->value("ShowCoverArt").toBool());
    }

    m_showPlaybackControlsAction->setChecked(m_settings->value("ShowPlaybackControls").toBool());
    m_mainWindow->showPlaybackControls(m_settings->value("ShowPlaybackControls").toBool());

    m_alwaysOnTopAction->setChecked(m_settings->value("AlwaysOnTop").toBool());
    m_playingOnTopAction->setChecked(m_settings->value("WhilePlayingOnTop").toBool());
    m_loopPlaylistAction->setChecked(m_settings->value("LoopPlaylist").toBool());
    m_nextFileEnableAction->setChecked(m_settings->value("LoadNext").toBool());

    QDir::SortFlag flag = (QDir::SortFlag)m_settings->value("LoadNextSort").toInt();
    if (flag == (QDir::Name)) {
        m_nextFileByNameAscdAction->setChecked(true);
    } else if (flag == (QDir::Name | QDir::Reversed)) {
        m_nextFileByNameDescAction->setChecked(true);
    } else if (flag == (QDir::Time | QDir::Reversed)) {
        m_nextFileByDateAscd->setChecked(true);
    } else if (flag == (QDir::Time)) {
        m_nextFileByDateDesc->setChecked(true);
    } else {
        m_nextFileByNameAscdAction->setChecked(true);
    }

    m_playbackEngine->setVolume(m_settings->value("Volume").toFloat());
}

void NPlayer::saveSettings()
{
    m_settings->setValue("Volume", QString::number(m_playbackEngine->volume()));
}

void NPlayer::on_preferencesDialog_settingsChanged()
{
    m_systemTray->setVisible(m_settings->value("TrayIcon").toBool());
    m_trackInfoWidget->loadSettings();
    m_trackInfoWidget->updateStaticTags(m_playbackEngine->currentMedia());
    m_playlistWidget->processVisibleItems();
}

#ifndef _N_NO_UPDATE_CHECK_
void NPlayer::downloadVersion()
{
    m_versionDownloader->get(QNetworkRequest(
        QUrl("https://static." + QCoreApplication::organizationDomain() + "/release/version")));
}

void NPlayer::on_versionDownloader_finished(QNetworkReply *reply)
{
    if (!reply->error()) {
        QString versionOnline = reply->readAll().simplified();

        if (m_preferencesDialog->isVisible()) {
            m_preferencesDialog->setVersionLabel(tr("Latest: ") + versionOnline);
        }

        if (QCoreApplication::applicationVersion() < versionOnline) {
            QMessageBox::information(m_mainWindow,
                                     QCoreApplication::applicationName() + tr(" Update"),
                                     tr("A newer version is available: ") + versionOnline +
                                         "<br><br>" + "<a href='https://" +
                                         QCoreApplication::organizationDomain() +
                                         "/download'>https://" +
                                         QCoreApplication::organizationDomain() + "/download</a>");
        }
    }

    reply->deleteLater();
}
#endif

void NPlayer::on_mainWindow_closed()
{
    if (m_settings->value("MinimizeToTray").toBool()) {
        m_systemTray->setVisible(true);
    } else {
#ifdef Q_OS_MAC
        if (m_settings->value("QuitOnClose").toBool()) {
            quit();
        }
#else
        quit();
#endif
    }
}

void NPlayer::on_trayClickTimer_timeout()
{
    if (!m_trayIconDoubleClickCheck) {
        trayIconCountClicks(1);
    }
}

void NPlayer::on_trayIcon_activated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) { // single click
        m_trayIconDoubleClickCheck = false;
        m_trayClickTimer->start(QApplication::doubleClickInterval());
    } else if (reason == QSystemTrayIcon::DoubleClick) {
        m_trayIconDoubleClickCheck = true;
        trayIconCountClicks(2);
    }
}

void NPlayer::toggleWindowVisibility()
{
    if (!m_mainWindow->isVisible() || m_mainWindow->isMinimized()) {
        m_mainWindow->show();
        m_mainWindow->activateWindow();
        m_mainWindow->raise();

    } else if (m_settings->value("MinimizeToTray").toBool()) {
        m_mainWindow->setVisible(false);
        m_systemTray->setVisible(true);
    } else {
        m_mainWindow->showMinimized();
    }
}

void NPlayer::trayIconCountClicks(int clicks)
{
    if (clicks == 1) {
        m_mainWindow->show();
        m_mainWindow->activateWindow();
        m_mainWindow->raise();
    } else if (clicks == 2) {
        toggleWindowVisibility();
    }
    if (!m_settings->value("TrayIcon").toBool()) {
        m_systemTray->setVisible(!m_mainWindow->isVisible());
    }
}

void NPlayer::quit()
{
    m_mainWindow->saveSettings();
    saveDefaultPlaylist();
    saveSettings();
    QCoreApplication::quit();
}

void NPlayer::on_playbackEngine_mediaChanged(const QString &file)
{
    QString title;
    QString titleDefault = QCoreApplication::applicationName() + " " +
                           QCoreApplication::applicationVersion();
    if (QFile(file).exists()) {
        NTagReaderInterface *tagReader = dynamic_cast<NTagReaderInterface *>(
            NPluginLoader::getPlugin(N::TagReader));
        if (!tagReader) {
            return;
        }
        QString encoding = NSettings::instance()->value("EncodingTrackInfo").toString();
        QString format = NSettings::instance()->value("WindowTitleTrackInfo").toString();
        QString title;
        if (!format.isEmpty()) {
            title = tagReader->toString(file, format, encoding);
        }
        if (title.isEmpty()) { // reading tags failed
            title = titleDefault;
        }
    } else {
        title = titleDefault;
    }
    m_mainWindow->setTitle(title);
    m_systemTray->setToolTip(title);
    m_trackInfoWidget->updateStaticTags(file);
}

void NPlayer::on_playbackEngine_stateChanged(N::PlaybackState state)
{
    bool whilePlaying = m_settings->value("WhilePlayingOnTop").toBool();
    bool alwaysOnTop = m_settings->value("AlwaysOnTop").toBool();
    bool oldOnTop = m_mainWindow->isOnTop();
    bool newOnTop = (whilePlaying && state == N::PlaybackPlaying);
    if (!alwaysOnTop && (oldOnTop != newOnTop)) {
        m_mainWindow->setOnTop(newOnTop);
    }
#ifdef Q_OS_WIN
    if (NW7TaskBar::instance()->isEnabled()) {
        if (state == N::PlaybackPlaying) {
            NW7TaskBar::instance()->setState(NW7TaskBar::Normal);
        } else {
            if (m_playbackEngine->position() != 0)
                NW7TaskBar::instance()->setState(NW7TaskBar::Paused);
            else
                NW7TaskBar::instance()->setState(NW7TaskBar::NoProgress);
        }
    }
#endif

    m_waveformSlider->setPausedState(state == N::PlaybackPaused);
}

void NPlayer::on_alwaysOnTopAction_toggled(bool checked)
{
    m_settings->setValue("AlwaysOnTop", checked);

    bool whilePlaying = m_settings->value("WhilePlayingOnTop").toBool();
    if (!whilePlaying || m_playbackEngine->state() != N::PlaybackPlaying) {
        m_mainWindow->setOnTop(checked);
    }
}

void NPlayer::on_whilePlayingOnTopAction_toggled(bool checked)
{
    m_settings->setValue("WhilePlayingOnTop", checked);

    bool alwaysOnTop = m_settings->value("AlwaysOnTop").toBool();
    if (!alwaysOnTop) {
        m_mainWindow->setOnTop(checked && m_playbackEngine->state() == N::PlaybackPlaying);
    }
}

void NPlayer::on_playlistAction_triggered()
{
    NAction *action = reinterpret_cast<NAction *>(QObject::sender());
    if (action == m_loopPlaylistAction) {
        m_settings->setValue("LoopPlaylist", action->isChecked());
    } else if (action == m_nextFileEnableAction) {
        m_settings->setValue("LoadNext", action->isChecked());
    } else if (action == m_nextFileByNameAscdAction) {
        m_settings->setValue("LoadNextSort", (int)QDir::Name);
    } else if (action == m_nextFileByNameDescAction) {
        m_settings->setValue("LoadNextSort", (int)(QDir::Name | QDir::Reversed));
    } else if (action == m_nextFileByDateAscd) {
        m_settings->setValue("LoadNextSort", (int)QDir::Time | QDir::Reversed);
    } else if (action == m_nextFileByDateDesc) {
        m_settings->setValue("LoadNextSort", (int)(QDir::Time));
    }
}

void NPlayer::on_jumpAction_triggered()
{
    NAction *action = reinterpret_cast<NAction *>(QObject::sender());
    QRegExp regex("(\\w+\\d)(\\w+)Action");
    regex.indexIn(action->objectName());
    m_playbackEngine->jump((regex.cap(2) == "Forward" ? 1 : -1) *
                           NSettings::instance()->value(regex.cap(1)).toDouble() * 1000);
}

void NPlayer::on_showCoverAction_toggled(bool checked)
{
    m_settings->setValue("ShowCoverArt", checked);
    if (m_coverWidget) {
        m_coverWidget->setEnabled(checked);
    }
}

void NPlayer::on_playButton_clicked()
{
    if (!m_playlistWidget->hasCurrent()) {
        if (m_playlistWidget->count() > 0) {
            m_playlistWidget->playRow(0);
        } else {
            showOpenFileDialog();
        }
    } else {
        m_playbackEngine->play(); // toggle play/pause
    }
}

void NPlayer::on_playbackEngine_failed()
{
    m_playlistWidget->currentFailed();
}

void NPlayer::showAboutMessageBox()
{
    if (!m_aboutDialog) {
        m_aboutDialog = new NAboutDialog(m_mainWindow);
    }
    m_aboutDialog->show();
}

void NPlayer::showOpenFileDialog()
{
    QString filters = NSettings::instance()->value("FileFilters").toString();
    QStringList files = QFileDialog::getOpenFileNames(m_mainWindow, tr("Add Files"),
                                                      m_settings->value("LastDirectory").toString(),
                                                      tr("All supported") + " (" + filters + ");;" +
                                                          tr("All files") + " (*)");

    if (files.isEmpty()) {
        return;
    }

    QString lastDir = QFileInfo(files.first()).path();
    m_settings->setValue("LastDirectory", lastDir);

    bool isEmpty = (m_playlistWidget->count() == 0);
    m_playlistWidget->addFiles(files);
    if (isEmpty) {
        m_playlistWidget->playRow(0);
    }
}

void NPlayer::on_mainWindow_scrolled(int delta)
{
    QWheelEvent event(QPoint(), delta, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(m_volumeSlider, &event);
}

void NPlayer::showOpenDirDialog()
{
    QString dir = QFileDialog::getExistingDirectory(m_mainWindow, tr("Add Directory"),
                                                    m_settings->value("LastDirectory").toString(),
                                                    QFileDialog::ShowDirsOnly |
                                                        QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty()) {
        return;
    }

    QString lastDir = QFileInfo(dir).path();
    m_settings->setValue("LastDirectory", lastDir);

    bool isEmpty = (m_playlistWidget->count() == 0);
    m_playlistWidget->addItems(NUtils::dirListRecursive(dir));
    if (isEmpty) {
        m_playlistWidget->playRow(0);
    }
}

void NPlayer::showSavePlaylistDialog()
{
    QString selectedFilter;
    QString file = QFileDialog::getSaveFileName(m_mainWindow, tr("Save Playlist"),
                                                m_settings->value("LastDirectory").toString(),
                                                tr("M3U Playlist") + " (*.m3u);;" +
                                                    tr("Extended M3U Playlist") + " (*.m3u)",
                                                &selectedFilter);

    if (file.isEmpty()) {
        return;
    }

    QString lastDir = QFileInfo(file).path();
    m_settings->setValue("LastDirectory", lastDir);

    if (!file.endsWith(".m3u")) {
        file.append(".m3u");
    }

    if (selectedFilter.startsWith("Extended")) {
        writePlaylist(file, N::ExtM3u);
    } else {
        writePlaylist(file, N::MinimalM3u);
    }
}

void NPlayer::showContextMenu(QPoint pos)
{
    // (1, 1) offset to avoid accidental item activation
    m_contextMenu->exec(m_mainWindow->mapToGlobal(pos) + QPoint(1, 1));
}
