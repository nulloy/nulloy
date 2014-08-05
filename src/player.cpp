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

#include "player.h"

#include "action.h"
#include "aboutDialog.h"
#include "common.h"
#include "logDialog.h"
#include "playlistStorage.h"
#include "mainWindow.h"
#include "playbackEngineInterface.h"
#include "playlistWidget.h"
#include "playlistWidgetItem.h"
#include "pluginLoader.h"
#include "preferencesDialog.h"
#include "scriptEngine.h"
#include "settings.h"
#include "tagReaderInterface.h"
#include "trackInfoWidget.h"
#include "i18nLoader.h"

#ifndef _N_NO_SKINS_
#include "skinLoader.h"
#include "skinFileSystem.h"
#endif

#ifdef Q_WS_WIN
#include "w7TaskBar.h"
#endif

#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMetaObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QResizeEvent>
#include <QMenuBar>

NPlayer::NPlayer()
{
	qsrand((uint)QTime::currentTime().msec());
	m_settings = NSettings::instance();

	NI18NLoader::init();

	// construct playbackEngine >>
	m_playbackEngine = dynamic_cast<NPlaybackEngineInterface *>(NPluginLoader::getPlugin(N::PlaybackEngine));
	m_playbackEngine->setParent(this);
	connect(m_playbackEngine, SIGNAL(mediaChanged(const QString &)), this, SLOT(on_playbackEngine_mediaChanged(const QString &)));
	connect(m_playbackEngine, SIGNAL(stateChanged(N::PlaybackState)), this, SLOT(on_playbackEngine_stateChanged(N::PlaybackState)));
	// << construct playbackEngine


	// construct mainWindow >>
	m_mainWindow = new NMainWindow();
	connect(m_mainWindow, SIGNAL(closed()), this, SLOT(on_mainWindow_closed()));
#ifndef _N_NO_SKINS_
	m_mainWindow->init(NSkinLoader::skinUiFormFile());
#else
	m_mainWindow->init(QString());
#endif
	// << construct mainWindow >>


	// loading skin script >>
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
	// << loading skin script


	m_preferencesDialog = new NPreferencesDialog(m_mainWindow);
	connect(m_preferencesDialog, SIGNAL(settingsChanged()), this, SLOT(on_preferencesDialog_settingsChanged()));
	connect(m_preferencesDialog, SIGNAL(versionRequested()), this, SLOT(downloadVersion()));

	m_playlistWidget = qFindChild<NPlaylistWidget *>(m_mainWindow, "playlistWidget");

	m_trackInfoWidget = new NTrackInfoWidget();
	QVBoxLayout *trackInfoLayout = new QVBoxLayout;
	trackInfoLayout->setContentsMargins(0, 0, 0, 0);
	trackInfoLayout->addWidget(m_trackInfoWidget);
	QWidget *waveformSlider = qFindChild<QWidget *>(m_mainWindow, "waveformSlider");
	waveformSlider->setLayout(trackInfoLayout);
	connect(m_playbackEngine, SIGNAL(tick(qint64)), m_trackInfoWidget, SLOT(tick(qint64)));

	// actions >>
	NAction *showHideAction = new NAction(QIcon::fromTheme("preferences-system-windows", QIcon(":/trolltech/styles/commonstyle/images/dockdock-16.png")), tr("Show / Hide"), this);
	showHideAction->setObjectName("showHideAction");
	showHideAction->setStatusTip(tr("Toggle window visibility"));
	showHideAction->setGlobal(TRUE);
	showHideAction->setCustomizable(TRUE);
	connect(showHideAction, SIGNAL(triggered()), this, SLOT(toggleWindowVisibility()));

	NAction *playAction = new NAction(QIcon::fromTheme("media-playback-start", style()->standardIcon(QStyle::SP_MediaPlay)), tr("Play / Pause"), this);
	playAction->setObjectName("playAction");
	playAction->setStatusTip(tr("Toggle playback"));
	playAction->setGlobal(TRUE);
	playAction->setCustomizable(TRUE);
	connect(playAction, SIGNAL(triggered()), m_playbackEngine, SLOT(play()));

	NAction *stopAction = new NAction(QIcon::fromTheme("media-playback-stop", style()->standardIcon(QStyle::SP_MediaStop)), tr("Stop"), this);
	stopAction->setObjectName("stopAction");
	stopAction->setStatusTip(tr("Stop playback"));
	stopAction->setGlobal(TRUE);
	stopAction->setCustomizable(TRUE);
	connect(stopAction, SIGNAL(triggered()), m_playbackEngine, SLOT(stop()));

	NAction *prevAction = new NAction(QIcon::fromTheme("media-playback-backward", style()->standardIcon(QStyle::SP_MediaSkipBackward)), tr("Previous"), this);
	prevAction->setObjectName("prevAction");
	prevAction->setStatusTip(tr("Play previous track in playlist"));
	prevAction->setGlobal(TRUE);
	prevAction->setCustomizable(TRUE);
	connect(prevAction, SIGNAL(triggered()), m_playlistWidget, SLOT(playPreviousRow()));

	NAction *nextAction = new NAction(QIcon::fromTheme("media-playback-forward", style()->standardIcon(QStyle::SP_MediaSkipForward)), tr("Next"), this);
	nextAction->setObjectName("nextAction");
	nextAction->setStatusTip(tr("Play next track in playlist"));
	nextAction->setGlobal(TRUE);
	nextAction->setCustomizable(TRUE);
	connect(nextAction, SIGNAL(triggered()), m_playlistWidget, SLOT(playNextRow()));

	NAction *preferencesAction = new NAction(QIcon::fromTheme("preferences-desktop",
	                                         style()->standardIcon(QStyle::SP_MessageBoxInformation)),
	                                         tr("Preferences..."), this);
	preferencesAction->setShortcut(QKeySequence("Ctrl+P"));
	connect(preferencesAction, SIGNAL(triggered()), m_preferencesDialog, SLOT(exec()));

	NAction *exitAction = new NAction(QIcon::fromTheme("exit",
	                                  style()->standardIcon(QStyle::SP_DialogCloseButton)),
	                                  tr("Exit"), this);
	exitAction->setShortcut(QKeySequence("Ctrl+Q"));
	connect(exitAction, SIGNAL(triggered()), this, SLOT(quit()));

	NAction *openFileDialogAction = new NAction(style()->standardIcon(QStyle::SP_DialogOpenButton), tr("Add Files..."), this);
	openFileDialogAction->setShortcut(QKeySequence("Ctrl+O"));
	connect(openFileDialogAction, SIGNAL(triggered()), this, SLOT(showOpenFileDialog()));
	connect(m_playlistWidget, SIGNAL(activateEmptyFail()), openFileDialogAction, SLOT(trigger()));

	NAction *openDirDialogAction = new NAction(style()->standardIcon(QStyle::SP_FileDialogNewFolder), tr("Add Directory..."), this);
	openDirDialogAction->setShortcut(QKeySequence("Ctrl+Shift+O"));
	connect(openDirDialogAction, SIGNAL(triggered()), this, SLOT(showOpenDirDialog()));

	NAction *savePlaylistDialogAction = new NAction(style()->standardIcon(QStyle::SP_DialogSaveButton), tr("Save Playlist..."), this);
	savePlaylistDialogAction->setShortcut(QKeySequence("Ctrl+S"));
	connect(savePlaylistDialogAction, SIGNAL(triggered()), this, SLOT(showSavePlaylistDialog()));

	NAction *showCoverAction = new NAction(tr("Show Cover Art"), this);
	showCoverAction->setCheckable(TRUE);
	showCoverAction->setObjectName("showCoverAction");
	connect(showCoverAction, SIGNAL(toggled(bool)), this, SLOT(on_showCoverAction_toggled(bool)));

	NAction *aboutAction = new NAction(QIcon::fromTheme("help-about",
	                                   style()->standardIcon(QStyle::SP_MessageBoxQuestion)),
	                                   tr("About"), this);
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(showAboutMessageBox()));
	m_aboutDialog = NULL;

	NAction *whilePlayingOnTopAction = new NAction(tr("On Top During Playback"), this);
	whilePlayingOnTopAction->setCheckable(TRUE);
	whilePlayingOnTopAction->setObjectName("whilePlayingOnTopAction");
	connect(whilePlayingOnTopAction, SIGNAL(toggled(bool)), this, SLOT(on_whilePlayingOnTopAction_toggled(bool)));

	NAction *alwaysOnTopAction = new NAction(tr("Always On Top"), this);
	alwaysOnTopAction->setCheckable(TRUE);
	alwaysOnTopAction->setObjectName("alwaysOnTopAction");
	connect(alwaysOnTopAction, SIGNAL(toggled(bool)), this, SLOT(on_alwaysOnTopAction_toggled(bool)));

	NAction *fullScreenAction = new NAction(tr("Fullscreen Mode"), this);
	fullScreenAction->setStatusTip(tr("Hide all controll except waveform"));
	fullScreenAction->setObjectName("fullScreenAction");
	fullScreenAction->setCustomizable(TRUE);
	connect(fullScreenAction, SIGNAL(triggered()), m_mainWindow, SLOT(toggleFullScreen()));
	// << actions


	// playlist actions >>
	NAction *loopPlaylistAction = new NAction(tr("Loop playlist"), this);
	loopPlaylistAction->setCheckable(TRUE);
	loopPlaylistAction->setObjectName("loopPlaylistAction");
	connect(loopPlaylistAction, SIGNAL(triggered()), this, SLOT(playlistActionTriggered()));

	NAction *loadNextAction = new NAction(tr("Load next file in directory when finished"), this);
	loadNextAction->setCheckable(TRUE);
	loadNextAction->setObjectName("loadNextAction");
	connect(loadNextAction, SIGNAL(triggered()), this, SLOT(playlistActionTriggered()));

	NAction *loadNextNameDownAction = new NAction(QString::fromUtf8("    ├  %1 ↓").arg(tr("By Name")), this);
	loadNextNameDownAction->setCheckable(TRUE);
	loadNextNameDownAction->setObjectName("loadNextNameDownAction");
	connect(loadNextNameDownAction, SIGNAL(triggered()), this, SLOT(playlistActionTriggered()));

	NAction *loadNextNameUpAction = new NAction(QString::fromUtf8("    ├  %1 ↑").arg(tr("By Name")), this);
	loadNextNameUpAction->setCheckable(TRUE);
	loadNextNameUpAction->setObjectName("loadNextNameUpAction");
	connect(loadNextNameUpAction, SIGNAL(triggered()), this, SLOT(playlistActionTriggered()));

	NAction *loadNextDateDownAction = new NAction(QString::fromUtf8("    ├  %1 ↓").arg(tr("By Date")), this);
	loadNextDateDownAction->setCheckable(TRUE);
	loadNextDateDownAction->setObjectName("loadNextDateDownAction");
	connect(loadNextDateDownAction, SIGNAL(triggered()), this, SLOT(playlistActionTriggered()));

	NAction *loadNextDateUpAction = new NAction(QString::fromUtf8("    └  %1 ↑").arg(tr("By Date")), this);
	loadNextDateUpAction->setCheckable(TRUE);
	loadNextDateUpAction->setObjectName("loadNextDateUpAction");
	connect(loadNextDateUpAction, SIGNAL(triggered()), this, SLOT(playlistActionTriggered()));

	QActionGroup *group = new QActionGroup(this);
	loadNextNameDownAction->setActionGroup(group);
	loadNextNameUpAction->setActionGroup(group);
	loadNextDateDownAction->setActionGroup(group);
	loadNextDateUpAction->setActionGroup(group);
	// << playlist actions


	// keyboard shortcuts >>
	m_settings->initShortcuts(this);
	m_settings->loadShortcuts();
	foreach (NAction *action, findChildren<NAction *>()) {
		if (!action->shortcuts().isEmpty())
			m_mainWindow->addAction(action);
	}
	// << keyboard shortcuts


	// tray icon >>
	QMenu *trayIconMenu = new QMenu(this);
	trayIconMenu->addAction(showHideAction);
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(playAction);
	trayIconMenu->addAction(stopAction);
	trayIconMenu->addAction(prevAction);
	trayIconMenu->addAction(nextAction);
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(preferencesAction);
	trayIconMenu->addAction(exitAction);
	m_systemTray = new QSystemTrayIcon(this);
	m_systemTray->setContextMenu(trayIconMenu);
	connect(m_systemTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(on_trayIcon_activated(QSystemTrayIcon::ActivationReason)));
#ifdef Q_WS_MAC
	m_systemTray->setIcon(QIcon(":mac-systray.png"));
#else
	m_systemTray->setIcon(m_mainWindow->windowIcon());
#endif
	m_trayClickTimer = new QTimer(this);
	m_trayClickTimer->setSingleShot(TRUE);
	connect(m_trayClickTimer, SIGNAL(timeout()), this, SLOT(on_trayClickTimer_timeout()));
	// << tray icon


	// context menu >>
	m_contextMenu = new QMenu(m_mainWindow);
	m_mainWindow->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_mainWindow, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
	m_contextMenu->addAction(openFileDialogAction);
	m_contextMenu->addAction(openDirDialogAction);
	m_contextMenu->addAction(savePlaylistDialogAction);

	QMenu *windowSubMenu = new QMenu(tr("Window"), m_mainWindow);
	windowSubMenu->addAction(whilePlayingOnTopAction);
	windowSubMenu->addAction(alwaysOnTopAction);
	windowSubMenu->addAction(fullScreenAction);
	m_contextMenu->addMenu(windowSubMenu);

	QMenu *playlistSubMenu = new QMenu(tr("Playlist"), m_mainWindow);
	playlistSubMenu->addAction(loopPlaylistAction);
	playlistSubMenu->addAction(loadNextAction);
	playlistSubMenu->addAction(loadNextNameDownAction);
	playlistSubMenu->addAction(loadNextNameUpAction);
	playlistSubMenu->addAction(loadNextDateDownAction);
	playlistSubMenu->addAction(loadNextDateUpAction);
	m_contextMenu->addMenu(playlistSubMenu);

	m_contextMenu->addAction(showCoverAction);
	m_contextMenu->addAction(preferencesAction);
	m_contextMenu->addSeparator();
	m_contextMenu->addAction(aboutAction);
	m_contextMenu->addSeparator();
	m_contextMenu->addAction(exitAction);
	// << context menu

#ifdef Q_WS_MAC
	// removing icons from context menu
	QList<NAction *> actions = findChildren<NAction *>();
	for (int i = 0; i < actions.size(); ++i)
		actions.at(i)->setIcon(QIcon());


	// global menu >>
	QMenuBar *menuBar = new QMenuBar(m_mainWindow);

	QMenu *fileMenu = menuBar->addMenu(tr("File"));
	fileMenu->addAction(openFileDialogAction);
	fileMenu->addAction(openDirDialogAction);
	fileMenu->addAction(savePlaylistDialogAction);
	fileMenu->addAction(aboutAction);
	fileMenu->addAction(exitAction);
	fileMenu->addAction(preferencesAction);

	QMenu *controlsMenu = menuBar->addMenu(tr("Controls"));
	controlsMenu->addAction(playAction);
	controlsMenu->addAction(stopAction);
	controlsMenu->addAction(prevAction);
	controlsMenu->addAction(nextAction);
	controlsMenu->addSeparator();
	controlsMenu->addMenu(playlistSubMenu);

	QMenu *windowMenu = menuBar->addMenu(tr("Window"));
	windowMenu->addAction(showCoverAction);
	windowMenu->addAction(whilePlayingOnTopAction);
	windowMenu->addAction(alwaysOnTopAction);
	windowMenu->addAction(fullScreenAction);
	// << global menu
#endif

#ifdef Q_WS_WIN
	NW7TaskBar::instance()->setWindow(m_mainWindow);
	NW7TaskBar::instance()->setEnabled(NSettings::instance()->value("TaskbarProgress").toBool());
	connect(m_playbackEngine, SIGNAL(positionChanged(qreal)), NW7TaskBar::instance(), SLOT(setProgress(qreal)));
#endif

	m_versionDownloader = new QNetworkAccessManager(this);
	connect(m_versionDownloader, SIGNAL(finished(QNetworkReply *)), this, SLOT(on_versionDownloader_finished(QNetworkReply *)));

	m_logDialog = new NLogDialog(m_mainWindow);
	connect(m_playbackEngine, SIGNAL(message(QMessageBox::Icon, const QString &, const QString &)),
	        m_logDialog, SLOT(showMessage(QMessageBox::Icon, const QString &, const QString &)));

	loadSettings();

	m_mainWindow->setTitle(QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion());
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

NMainWindow* NPlayer::mainWindow()
{
	return m_mainWindow;
}

NPlaybackEngineInterface* NPlayer::playbackEngine()
{
	return m_playbackEngine;
}

bool NPlayer::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::FileOpen) {
		QFileOpenEvent *fileEvent = static_cast<QFileOpenEvent *>(event);

		if (!fileEvent->file().isEmpty())
		m_playlistWidget->playFiles(QStringList() << fileEvent->file());

		return FALSE;
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
	QStringList argList = str.split("<|>");
	QStringList files;
	QStringList notPathArgList;
	foreach (QString arg, argList) {
		if (QFile(arg).exists())
			files << arg;
		else
			notPathArgList << arg;
	}

	foreach (QString arg, notPathArgList) {
		if (arg == "--next")
			m_playlistWidget->playNextRow();
		else if (arg == "--prev")
			m_playlistWidget->playPreviousRow();
		else if (arg == "--stop")
			m_playbackEngine->stop();
		else if (arg == "--pause")
			m_playbackEngine->play();
	}

	if (!files.isEmpty()) {
		if (NSettings::instance()->value("EnqueueFiles").toBool()) {
			m_playlistWidget->addFiles(files);
			if (m_playbackEngine->state() == N::PlaybackStopped || NSettings::instance()->value("PlayEnqueued").toBool())
				m_playlistWidget->playRow(m_playlistWidget->count() - 1);
		} else {
			m_playlistWidget->playFiles(files);
		}
	}

	m_playlistWidget->setShuffleMode(NSettings::instance()->value("Shuffle").toBool());
}

void NPlayer::loadDefaultPlaylist()
{
	if (m_playlistWidget->count() > 0 || // files were added by calling message() directly in main()
	    !QFileInfo(NCore::defaultPlaylistPath()).exists() ||
	    !m_playlistWidget->setPlaylist(NCore::defaultPlaylistPath()))
	{
		return;
	}

	QStringList playlistRowValues = m_settings->value("PlaylistRow").toStringList();
	if (!playlistRowValues.isEmpty()) {
		int row = playlistRowValues.at(0).toInt();
		qreal pos = playlistRowValues.at(1).toFloat();
		if (pos != 0 && pos != 1) {
			m_playlistWidget->playRow(row);
			m_playbackEngine->setPosition(pos);
			if (m_settings->value("StartPaused").toBool())
				m_playbackEngine->pause();
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
	m_settings->setValue("PlaylistRow", QStringList() << QString::number(row) << QString::number(pos));
}

void NPlayer::loadSettings()
{
	m_systemTray->setVisible(m_settings->value("TrayIcon").toBool());

	if (m_settings->value("AutoCheckUpdates").toBool())
		downloadVersion();

	qFindChild<NAction *>(this, "showCoverAction")->setChecked(m_settings->value("ShowCoverArt").toBool());
	qFindChild<QWidget *>(m_mainWindow, "coverWidget")->setEnabled(m_settings->value("ShowCoverArt").toBool());

	qFindChild<NAction *>(this, "alwaysOnTopAction"      )->setChecked(m_settings->value("AlwaysOnTop").toBool());
	qFindChild<NAction *>(this, "whilePlayingOnTopAction")->setChecked(m_settings->value("WhilePlayingOnTop").toBool());
	qFindChild<NAction *>(this, "loopPlaylistAction"     )->setChecked(m_settings->value("LoopPlaylist").toBool());
	qFindChild<NAction *>(this, "loadNextAction"         )->setChecked(m_settings->value("LoadNext").toBool());

	QDir::SortFlag flag = (QDir::SortFlag)m_settings->value("LoadNextSort").toInt();
	NAction *action;
	if (flag == (QDir::Name))
		action = qFindChild<NAction *>(this, "loadNextNameDownAction");
	else if (flag == (QDir::Name | QDir::Reversed))
		action = qFindChild<NAction *>(this, "loadNextNameUpAction");
	else if (flag == (QDir::Time | QDir::Reversed))
		action = qFindChild<NAction *>(this, "loadNextDateDownAction");
	else if (flag == (QDir::Time))
		action = qFindChild<NAction *>(this, "loadNextDateUpAction");
	else
		action = qFindChild<NAction *>(this, "loadNextNameDownAction");
	action->setChecked(TRUE);

	m_playbackEngine->setVolume(m_settings->value("Volume").toFloat());
}

void NPlayer::saveSettings()
{
	m_settings->setValue("Volume", QString::number(m_playbackEngine->volume()));
}

void NPlayer::on_preferencesDialog_settingsChanged()
{
	m_systemTray->setVisible(m_settings->value("TrayIcon").toBool());
	m_trackInfoWidget->readSettings();
	m_trackInfoWidget->updateInfo();
}

void NPlayer::downloadVersion()
{
	QString suffix;
#if defined Q_WS_WIN
	suffix = "win";
#elif defined Q_WS_X11
	suffix = "x11";
#elif defined Q_WS_MAC
	suffix = "mac";
#endif

	if (!suffix.isEmpty())
		m_versionDownloader->get(QNetworkRequest(QUrl("http://" +
		                      QCoreApplication::organizationDomain() + "/version_" + suffix)));
}

void NPlayer::on_versionDownloader_finished(QNetworkReply *reply)
{
	if (!reply->error()) {
		QString versionOnline = reply->readAll().simplified();

		if (m_preferencesDialog->isVisible())
			m_preferencesDialog->setVersionLabel(tr("Latest: ") + versionOnline);

		if (QCoreApplication::applicationVersion() < versionOnline) {
			QMessageBox::information(m_mainWindow,
			                         QCoreApplication::applicationName() + tr(" Update"),
			                         tr("A newer version is available: ") + versionOnline + "<br><br>" +
			                         "<a href='http://" + QCoreApplication::organizationDomain() + "'>http://" +
			                         QCoreApplication::organizationDomain() + "/download</a>");
		}
	}

	reply->deleteLater();
}

void NPlayer::on_mainWindow_closed()
{
	if (m_settings->value("MinimizeToTray").toBool()) {
		m_systemTray->setVisible(TRUE);
	} else {
		quit();
	}
}

void NPlayer::on_trayClickTimer_timeout()
{
	if (!m_trayIconDoubleClickCheck)
		trayIconCountClicks(1);
}

void NPlayer::on_trayIcon_activated(QSystemTrayIcon::ActivationReason reason)
{
	if (reason == QSystemTrayIcon::Trigger) { // single click
		m_trayIconDoubleClickCheck = FALSE;
		m_trayClickTimer->start(QApplication::doubleClickInterval());
	} else if (reason == QSystemTrayIcon::DoubleClick) {
		m_trayIconDoubleClickCheck = TRUE;
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
		m_mainWindow->setVisible(FALSE);
		m_systemTray->setVisible(TRUE);
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
	if (!m_settings->value("TrayIcon").toBool())
		m_systemTray->setVisible(!m_mainWindow->isVisible());
}

void NPlayer::quit()
{
	m_mainWindow->saveSettings();
	saveDefaultPlaylist();
	saveSettings();
	QCoreApplication::quit();
}

void NPlayer::on_playbackEngine_mediaChanged(const QString &path)
{
	QString title;
	QString app_title_version = QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion();
	if (QFile(path).exists()) {
		NTagReaderInterface *tagReader = dynamic_cast<NTagReaderInterface *>(NPluginLoader::getPlugin(N::TagReader));
		QString format = NSettings::instance()->value("WindowTitleTrackInfo").toString();
		if (!format.isEmpty() && tagReader->isValid())
			title = tagReader->toString(format);
		else
			title = app_title_version;
	} else {
		title = app_title_version;
	}
	m_mainWindow->setTitle(title);
	m_systemTray->setToolTip(title);
	m_trackInfoWidget->updateInfo();
}

void NPlayer::on_playbackEngine_stateChanged(N::PlaybackState state)
{
	bool whilePlaying = m_settings->value("WhilePlayingOnTop").toBool();
	bool alwaysOnTop = m_settings->value("AlwaysOnTop").toBool();
	if (!alwaysOnTop)
		m_mainWindow->setOnTop(whilePlaying && state == N::PlaybackPlaying);
#ifdef Q_WS_WIN
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
}

void NPlayer::on_alwaysOnTopAction_toggled(bool checked)
{
	m_settings->setValue("AlwaysOnTop", checked);

	bool whilePlaying = m_settings->value("WhilePlayingOnTop").toBool();
	if (!whilePlaying || m_playbackEngine->state() != N::PlaybackPlaying)
		m_mainWindow->setOnTop(checked);
}

void NPlayer::on_whilePlayingOnTopAction_toggled(bool checked)
{
	m_settings->setValue("WhilePlayingOnTop", checked);

	bool alwaysOnTop = m_settings->value("AlwaysOnTop").toBool();
	if (!alwaysOnTop)
		m_mainWindow->setOnTop(checked && m_playbackEngine->state() == N::PlaybackPlaying);
}

void NPlayer::playlistActionTriggered()
{
	NAction *action = reinterpret_cast<NAction *>(QObject::sender());
	QString name = action->objectName();
	bool checked = action->isChecked();
	if (name == "loopPlaylistAction")
		m_settings->setValue("LoopPlaylist", checked);
	else if (name == "loadNextAction")
		m_settings->setValue("LoadNext", checked);
	else if (name == "loadNextNameDownAction")
		m_settings->setValue("LoadNextSort", (int)QDir::Name);
	else if (name == "loadNextNameUpAction")
		m_settings->setValue("LoadNextSort", (int)(QDir::Name | QDir::Reversed));
	else if (name == "loadNextDateDownAction")
		m_settings->setValue("LoadNextSort", (int)QDir::Time | QDir::Reversed);
	else if (name == "loadNextDateUpAction")
		m_settings->setValue("LoadNextSort", (int)(QDir::Time));
}

void NPlayer::on_showCoverAction_toggled(bool checked)
{
	m_settings->setValue("ShowCoverArt", checked);
	QWidget *coverWidget = qFindChild<QWidget *>(m_mainWindow, "coverWidget");
	coverWidget->setEnabled(checked);
}

void NPlayer::showAboutMessageBox()
{
	if (!m_aboutDialog)
		m_aboutDialog = new NAboutDialog(m_mainWindow);
	m_aboutDialog->show();
}

void NPlayer::showOpenFileDialog()
{
	QString filters = NSettings::instance()->value("FileFilters").toStringList().join(" ");
	QStringList files = QFileDialog::getOpenFileNames(
		m_mainWindow, qobject_cast<QAction *>(QObject::sender())->text().remove("..."),
		m_settings->value("LastDirectory").toString(),
		tr("All supported") + " (" + filters + ");;" +
		tr("All files") + " (*)"
	);

	if (files.isEmpty())
		return;

	QString lastDir = QFileInfo(files.first()).path();
	m_settings->setValue("LastDirectory", lastDir);

	bool isEmpty = (m_playlistWidget->count() == 0);
	m_playlistWidget->addFiles(files);
	if (isEmpty)
		m_playlistWidget->playRow(0);
}

void NPlayer::showOpenDirDialog()
{

	QString dir = QFileDialog::getExistingDirectory(
		m_mainWindow, qobject_cast<QAction *>(QObject::sender())->text().remove("..."),
		m_settings->value("LastDirectory").toString(),
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
	);

	if (dir.isEmpty())
		return;

	QString lastDir = QFileInfo(dir).path();
	m_settings->setValue("LastDirectory", lastDir);

	bool isEmpty = (m_playlistWidget->count() == 0);
	m_playlistWidget->addFiles(NCore::dirListRecursive(dir, NSettings::instance()->value("FileFilters").toStringList()));
	if (isEmpty)
		m_playlistWidget->playRow(0);
}

void NPlayer::showSavePlaylistDialog()
{
	QString selectedFilter;
	QString file = QFileDialog::getSaveFileName(
		m_mainWindow, qobject_cast<QAction *>(QObject::sender())->text().remove("..."),
		m_settings->value("LastDirectory").toString(),
		tr("M3U Playlist") + " (*.m3u);;" +
		tr("Extended M3U Playlist") + " (*.m3u)",
		&selectedFilter
	);

	if (file.isEmpty())
		return;

	QString lastDir = QFileInfo(file).path();
	m_settings->setValue("LastDirectory", lastDir);

	if (!file.endsWith(".m3u"))
		file.append(".m3u");

	if (selectedFilter.startsWith("Extended"))
		writePlaylist(file, N::ExtM3u);
	else
		writePlaylist(file, N::MinimalM3u);
}

void NPlayer::showContextMenu(QPoint pos)
{
	m_contextMenu->exec(m_mainWindow->mapToGlobal(pos));
}

