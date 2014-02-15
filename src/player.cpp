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

#include "player.h"

#include "action.h"
#include "core.h"
#include "logDialog.h"
#include "playlistStorage.h"
#include "mainWindow.h"
#include "playbackEngineInterface.h"
#include "playlistWidget.h"
#include "playlistWidgetItem.h"
#include "preferencesDialog.h"
#include "scriptEngine.h"
#include "settings.h"
#include "systemTray.h"
#include "tagReaderInterface.h"
#include "trackInfoWidget.h"

#ifndef _N_NO_SKINS_
#include "skinLoader.h"
#include "skinFileSystem.h"
#endif

#ifndef _N_NO_PLUGINS_
#include "pluginLoader.h"
#else
#include "playbackEngineGstreamer.h"
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
#include <QTextBrowser>
#include <QResizeEvent>

NPlayer::NPlayer()
{
	m_settings = new NSettings(this);


	// construct playbackEngine
#ifndef _N_NO_PLUGINS_
	m_playbackEngine = NPluginLoader::playbackPlugin();
#else
	m_playbackEngine = dynamic_cast<NPlaybackEngineInterface *>(new NPlaybackEngineGStreamer());
	dynamic_cast<NPluginElementInterface *>(m_playbackEngine)->init();
#endif
	m_playbackEngine->setParent(this);
	connect(m_playbackEngine, SIGNAL(mediaChanged(const QString &)), this, SLOT(on_playbackEngine_mediaChanged(const QString &)));
	connect(m_playbackEngine, SIGNAL(stateChanged(N::PlaybackState)), this, SLOT(on_playbackEngine_stateChanged(N::PlaybackState)));
	//


	// construct mainWindow
	m_mainWindow = new NMainWindow();
	connect(m_mainWindow, SIGNAL(closed()), this, SLOT(on_mainWindow_closed()));
#ifndef _N_NO_SKINS_
	m_mainWindow->init(NSkinLoader::skinUiFormFile());
#else
	m_mainWindow->init(QString());
#endif
	//


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

	QScriptValue constructor = m_scriptEngine->evaluate("Program");
	QScriptValue playerEngineObject = m_scriptEngine->newQObject(this, QScriptEngine::QtOwnership);
	QScriptValue skinProgram = constructor.construct(QScriptValueList() << playerEngineObject);
	//


	m_preferencesDialog = new NPreferencesDialog(m_mainWindow);
	connect(m_preferencesDialog, SIGNAL(settingsChanged()), this, SLOT(on_preferencesDialog_settingsChanged()));
	connect(m_preferencesDialog, SIGNAL(versionRequested()), this, SLOT(downloadVersion()));

	m_playlistWidget = qFindChild<NPlaylistWidget *>(m_mainWindow, "playlistWidget");
	connect(m_playlistWidget, SIGNAL(activateEmptyFail()), this, SLOT(showOpenFileDialog()));

	m_trackInfoWidget = new NTrackInfoWidget();
	m_trackInfoWidget->setStyleSheet(m_trackInfoWidget->styleSheet() + m_mainWindow->styleSheet());
	m_trackInfoWidget->setTagReader(NPluginLoader::tagReaderPlugin());
	QVBoxLayout *trackInfoLayout = new QVBoxLayout;
	trackInfoLayout->setContentsMargins(0, 0, 0, 0);
	trackInfoLayout->addWidget(m_trackInfoWidget);
	QWidget *waveformSlider = qFindChild<QWidget *>(m_mainWindow, "waveformSlider");
	waveformSlider->setLayout(trackInfoLayout);
	connect(m_playbackEngine, SIGNAL(tick(qint64)), m_trackInfoWidget, SLOT(tick(qint64)));

	// actions
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
	connect(prevAction, SIGNAL(triggered()), m_playlistWidget, SLOT(playPrevious()));

	NAction *nextAction = new NAction(QIcon::fromTheme("media-playback-forward", style()->standardIcon(QStyle::SP_MediaSkipForward)), tr("Next"), this);
	nextAction->setObjectName("nextAction");
	nextAction->setStatusTip(tr("Play next track in playlist"));
	nextAction->setGlobal(TRUE);
	nextAction->setCustomizable(TRUE);
	connect(nextAction, SIGNAL(triggered()), m_playlistWidget, SLOT(playNext()));

	NAction *preferencesAction = new NAction(QIcon::fromTheme("preferences-desktop",
	                                         style()->standardIcon(QStyle::SP_MessageBoxInformation)),
	                                         tr("Preferences..."), this);
	connect(preferencesAction, SIGNAL(triggered()), m_preferencesDialog, SLOT(exec()));

	NAction *exitAction = new NAction(QIcon::fromTheme("exit",
	                                  style()->standardIcon(QStyle::SP_DialogCloseButton)),
	                                  tr("Exit"), this);
	connect(exitAction, SIGNAL(triggered()), this, SLOT(quit()));

	NAction *openFileDialogAction = new NAction(style()->standardIcon(QStyle::SP_DialogOpenButton), tr("Add Files..."), this);
	connect(openFileDialogAction, SIGNAL(triggered()), this, SLOT(showOpenFileDialog()));

	NAction *openDirDialogAction = new NAction(style()->standardIcon(QStyle::SP_FileDialogNewFolder), tr("Add Directory..."), this);
	connect(openDirDialogAction, SIGNAL(triggered()), this, SLOT(showOpenDirDialog()));

	NAction *savePlaylistDialogAction = new NAction(style()->standardIcon(QStyle::SP_DialogSaveButton), tr("Save Playlist..."), this);
	connect(savePlaylistDialogAction, SIGNAL(triggered()), this, SLOT(showSavePlaylistDialog()));

	NAction *showCoverAction = new NAction(tr("Show Cover Art"), this);
	showCoverAction->setCheckable(TRUE);
	showCoverAction->setObjectName("showCoverAction");

	NAction *aboutAction = new NAction(QIcon::fromTheme("help-about",
	                                   style()->standardIcon(QStyle::SP_MessageBoxQuestion)),
	                                   tr("About") + " " + QCoreApplication::applicationName(), this);
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(showAboutMessageBox()));

	NAction *whilePlayingOnTopAction = new NAction(tr("On Top During Playback"), this);
	whilePlayingOnTopAction->setCheckable(TRUE);
	whilePlayingOnTopAction->setObjectName("whilePlayingOnTopAction");

	NAction *alwaysOnTopAction = new NAction(tr("Always On Top"), this);
	alwaysOnTopAction->setCheckable(TRUE);
	alwaysOnTopAction->setObjectName("alwaysOnTopAction");
	//


	// playlist actions
	NAction *loadNextAction = new NAction(tr("Load next file in directory when finished"), this);
	loadNextAction->setCheckable(TRUE);
	loadNextAction->setObjectName("loadNextAction");
	connect(loadNextAction, SIGNAL(triggered()), this, SLOT(loadNextActionTriggered()));

	NAction *loadNextNameDownAction = new NAction(trUtf8("    ├  By Name ↓"), this);
	loadNextNameDownAction->setCheckable(TRUE);
	loadNextNameDownAction->setObjectName("loadNextNameDownAction");
	connect(loadNextNameDownAction, SIGNAL(triggered()), this, SLOT(loadNextActionTriggered()));

	NAction *loadNextNameUpAction = new NAction(trUtf8("    ├  By Name ↑"), this);
	loadNextNameUpAction->setCheckable(TRUE);
	loadNextNameUpAction->setObjectName("loadNextNameUpAction");
	connect(loadNextNameUpAction, SIGNAL(triggered()), this, SLOT(loadNextActionTriggered()));

	NAction *loadNextDateDownAction = new NAction(trUtf8("    ├  By Date ↓"), this);
	loadNextDateDownAction->setCheckable(TRUE);
	loadNextDateDownAction->setObjectName("loadNextDateDownAction");
	connect(loadNextDateDownAction, SIGNAL(triggered()), this, SLOT(loadNextActionTriggered()));

	NAction *loadNextDateUpAction = new NAction(trUtf8("    └  By Date ↑"), this);
	loadNextDateUpAction->setCheckable(TRUE);
	loadNextDateUpAction->setObjectName("loadNextDateUpAction");
	connect(loadNextDateUpAction, SIGNAL(triggered()), this, SLOT(loadNextActionTriggered()));

	QActionGroup *group = new QActionGroup(this);
	loadNextNameDownAction->setActionGroup(group);
	loadNextNameUpAction->setActionGroup(group);
	loadNextDateDownAction->setActionGroup(group);
	loadNextDateUpAction->setActionGroup(group);
	//


	// tray icon
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
	NSystemTray::init(this);
	NSystemTray::setContextMenu(trayIconMenu);
#ifdef Q_WS_MAC
	NSystemTray::setIcon(QIcon(":mac-systray.png"));
#else
	NSystemTray::setIcon(m_mainWindow->windowIcon());
#endif
	m_trayClickTimer = new QTimer(this);
	m_trayClickTimer->setSingleShot(TRUE);
	connect(m_trayClickTimer, SIGNAL(timeout()), this, SLOT(on_trayClickTimer_timeout()));
	//


	// context menu
	m_contextMenu = new QMenu(m_mainWindow);
	m_mainWindow->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_mainWindow, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
	m_contextMenu->addAction(openFileDialogAction);
	m_contextMenu->addAction(openDirDialogAction);
	m_contextMenu->addAction(savePlaylistDialogAction);

	QMenu *windowSubMenu = new QMenu("Window", m_mainWindow);
	windowSubMenu->addAction(whilePlayingOnTopAction);
	windowSubMenu->addAction(alwaysOnTopAction);
	m_contextMenu->addMenu(windowSubMenu);

	QMenu *playlistSubMenu = new QMenu("Playlist", m_mainWindow);
	playlistSubMenu->addAction(loadNextAction);
	playlistSubMenu->addAction(loadNextNameDownAction);
	playlistSubMenu->addAction(loadNextNameUpAction);
	playlistSubMenu->addAction(loadNextDateDownAction);
	playlistSubMenu->addAction(loadNextDateUpAction);
	m_contextMenu->addMenu(playlistSubMenu);

	if (NPluginLoader::coverReaderPlugin())
		m_contextMenu->addAction(showCoverAction);
	m_contextMenu->addAction(preferencesAction);
	m_contextMenu->addSeparator();
	m_contextMenu->addAction(aboutAction);
	m_contextMenu->addSeparator();
	m_contextMenu->addAction(exitAction);
	//

#ifdef Q_WS_MAC
	// removing icons from context menu
	QList<NAction *> actions = findChildren<NAction *>();
	for (int i = 0; i < actions.size(); ++i)
		actions.at(i)->setIcon(QIcon());
#endif

#ifdef Q_WS_WIN
	NW7TaskBar::init(this);
	NW7TaskBar::setWindow(m_mainWindow);
	connect(m_playbackEngine, SIGNAL(positionChanged(qreal)), NW7TaskBar::instance(), SLOT(setProgress(qreal)));
#endif

	m_versionDownloader = new QNetworkAccessManager(this);
	connect(m_versionDownloader, SIGNAL(finished(QNetworkReply *)), this, SLOT(on_versionDownloader_finished(QNetworkReply *)));

	m_logDialog = new NLogDialog(m_mainWindow);
	connect(m_playbackEngine, SIGNAL(message(QMessageBox::Icon, const QString &, const QString &)),
	        m_logDialog, SLOT(showMessage(QMessageBox::Icon, const QString &, const QString &)));

	m_settings->initShortcuts(this);
	m_settings->loadShortcuts();
	foreach (NAction *action, m_settings->shortcuts())
		m_mainWindow->addAction(action);

	loadSettings();

	m_mainWindow->setTitle(QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion());
	m_mainWindow->show();
	QResizeEvent e(m_mainWindow->size(), m_mainWindow->size());
	QCoreApplication::sendEvent(m_mainWindow, &e);

	skinProgram.property("afterShow").call(skinProgram);
}

NPlayer::~NPlayer()
{
	NPluginLoader::deinit();
	delete m_mainWindow;
}

NMainWindow* NPlayer::mainWindow()
{
	return m_mainWindow;
}

NPlaybackEngineInterface* NPlayer::playbackEngine()
{
	return m_playbackEngine;
}

NSettings* NPlayer::settings()
{
	return NSettings::instance();
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

void NPlayer::message(const QString &str)
{
	if (str.isEmpty()) {
		m_mainWindow->showNormal();
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
			m_playlistWidget->playNext();
		else if (arg == "--prev")
			m_playlistWidget->playPrevious();
		else if (arg == "--stop")
			m_playbackEngine->stop();
		else if (arg == "--pause")
			m_playlistWidget->playCurrent();
	}

	if (!files.isEmpty())
		m_playlistWidget->playFiles(files);
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
		m_playlistWidget->playRow(playlistRowValues.at(0).toInt());
		qreal pos = playlistRowValues.at(1).toFloat();
		if (m_settings->value("RestorePlayback").toBool() && pos != 0 && pos != 1) {
			m_playbackEngine->play();
			m_playbackEngine->setPosition(pos);
		}
	}
}

void NPlayer::writePlaylist(const QString &file)
{
	QList<NPlaylistDataItem> dataItemsList;
	for (int i = 0; i < m_playlistWidget->count(); ++i) {
		NPlaylistDataItem dataItem = m_playlistWidget->item(i)->dataItem();
		dataItem.title = m_playlistWidget->item(i)->text();
		dataItemsList << dataItem;
	}
	NPlaylistStorage::writeM3u(file, dataItemsList);
}

void NPlayer::saveDefaultPlaylist()
{
	writePlaylist(NCore::defaultPlaylistPath());

	int row = m_playlistWidget->currentRow();
	qreal pos = m_playbackEngine->position();
	m_settings->setValue("PlaylistRow", QStringList() << QString::number(row) << QString::number(pos));
}

void NPlayer::loadSettings()
{
	NSystemTray::setEnabled(m_settings->value("TrayIcon").toBool());

	if (m_settings->value("AutoCheckUpdates").toBool())
		downloadVersion();

	bool alwaysOnTop = m_settings->value("AlwaysOnTop").toBool();
	if (alwaysOnTop) {
		NAction *alwaysOnTopAction = qFindChild<NAction *>(this, "alwaysOnTopAction");
		alwaysOnTopAction->setChecked(TRUE);
		on_alwaysOnTopAction_toggled(TRUE);
	}

	bool showCover = m_settings->value("ShowCoverArt").toBool();
	NAction *showCoverAction = qFindChild<NAction *>(this, "showCoverAction");
	showCoverAction->setChecked(showCover);
	on_showCoverAction_toggled(showCover);

	bool whilePlaying = m_settings->value("WhilePlayingOnTop").toBool();
	if (whilePlaying) {
		NAction *whilePlayingOnTopAction = qFindChild<NAction *>(this, "whilePlayingOnTopAction");
		whilePlayingOnTopAction->setChecked(TRUE);
		on_whilePlayingOnTopAction_toggled(TRUE);
	}

	bool loadNext = m_settings->value("LoadNext").toBool();
	if (loadNext) {
		NAction *loadNextAction = qFindChild<NAction *>(this, "loadNextAction");
		loadNextAction->setChecked(TRUE);
	}
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
	NSystemTray::setEnabled(m_settings->value("TrayIcon").toBool());
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
			m_preferencesDialog->setVersionLabel("Latest: " + versionOnline);

		if (QCoreApplication::applicationVersion() < versionOnline) {
			QMessageBox::information(m_mainWindow,
			                         QCoreApplication::applicationName() + " Update",
			                         "A newer version is available: " + versionOnline + "<br><br>" +
			                         "<a href='http://" + QCoreApplication::organizationDomain() + "'>http://" +
			                         QCoreApplication::organizationDomain() + "/download</a>");
		}
	}

	reply->deleteLater();
}

void NPlayer::on_mainWindow_closed()
{
	if (m_settings->value("MinimizeToTray").toBool()) {
		NSystemTray::setEnabled(TRUE);
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
	if (!m_mainWindow->isVisible() || !m_mainWindow->isActiveWindow()) {
		m_mainWindow->showNormal();
		m_mainWindow->activateWindow();
		m_mainWindow->raise();
	} else if (m_settings->value("MinimizeToTray").toBool()) {
		m_mainWindow->setVisible(FALSE);
		NSystemTray::setEnabled(TRUE);
	} else {
		m_mainWindow->showMinimized();
	}
}

void NPlayer::trayIconCountClicks(int clicks)
{
	if (clicks == 1) {
		m_mainWindow->showNormal();
		m_mainWindow->activateWindow();
		m_mainWindow->raise();
	} else if (clicks == 2) {
		toggleWindowVisibility();
	}
	if (!m_settings->value("TrayIcon").toBool())
		NSystemTray::setEnabled(!m_mainWindow->isVisible());
}

void NPlayer::quit()
{
	m_mainWindow->close();
	saveDefaultPlaylist();
	saveSettings();
	QCoreApplication::quit();
}

void NPlayer::on_playbackEngine_mediaChanged(const QString &path)
{
	if (path.isEmpty())
		return;

	QString title;
	QString app_title_version = QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion();
	if (QFile(path).exists()) {
		NTagReaderInterface *tagReader = NPluginLoader::tagReaderPlugin();
		QString format = NSettings::instance()->value("WindowTitleTrackInfo").toString();
		if (!format.isEmpty() && tagReader->isValid())
			title = tagReader->toString(format);
		else
			title = app_title_version;
	} else {
		title = app_title_version;
	}
	m_mainWindow->setTitle(title);
	m_trackInfoWidget->updateInfo();
	NSystemTray::setToolTip(title);
}

void NPlayer::on_playbackEngine_stateChanged(N::PlaybackState state)
{
	bool whilePlaying = m_settings->value("WhilePlayingOnTop").toBool();
	bool alwaysOnTop = m_settings->value("AlwaysOnTop").toBool();
	if (!alwaysOnTop)
		m_mainWindow->setOnTop(whilePlaying && state == N::PlaybackPlaying);
#ifdef Q_WS_WIN
	if (state == N::PlaybackPlaying) {
		NW7TaskBar::instance()->setState(NW7TaskBar::Normal);
		NW7TaskBar::setOverlayIcon(QIcon(":/trolltech/styles/commonstyle/images/media-play-32.png"), "Playing");
	} else {
		if (m_playbackEngine->position() != 0) {
			NW7TaskBar::instance()->setState(NW7TaskBar::Paused);
			NW7TaskBar::setOverlayIcon(QIcon(":/trolltech/styles/commonstyle/images/media-pause-32.png"), "Paused");
		} else {
			NW7TaskBar::instance()->setState(NW7TaskBar::NoProgress);
			NW7TaskBar::setOverlayIcon(QIcon(), "Stopped");
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

void NPlayer::loadNextActionTriggered()
{
	NAction *action = reinterpret_cast<NAction *>(QObject::sender());
	QString name = action->objectName();
	bool checked = action->isChecked();
	if (name == "loadNextAction")
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
	QString aboutHtml = QString() +
#ifdef Q_WS_MAC
	               "<span style=\" font-size:14pt;\">" +
#else
	               "<span style=\" font-size:9pt;\">" +
#endif
	                 "<b>" +  QCoreApplication::applicationName() + " Music Player</b>" +
	                 "<br>" +
	                 "<a href='http://" + QCoreApplication::organizationDomain() + "'>http://" +
	                                      QCoreApplication::organizationDomain() + "</a>" +
	               "</span><br><br>" +
#ifdef Q_WS_MAC
	               "<span style=\" font-size:10pt;\">" +
#else
	               "<span style=\" font-size:8pt;\">" +
#endif
	                 "Version: " + QCoreApplication::applicationVersion() + "<br>" +
	                 (QString(_N_TIME_STAMP_).isEmpty() ? "" : "Build: " + QString(_N_TIME_STAMP_)) +
	                 "<br><br>" +
	                 "Copyright (C) 2010-2013  Sergey Vlasov &lt;sergey@vlasov.me&gt;" +
	               "</span>";

	QDialog *dialog = new QDialog(m_mainWindow);
	dialog->setWindowTitle(QObject::tr("About ") + QCoreApplication::applicationName());
	dialog->setMaximumSize(0, 0);

	QVBoxLayout *layout = new QVBoxLayout;
	dialog->setLayout(layout);

	QTabWidget *tabWidget = new QTabWidget(m_mainWindow);
	layout->addWidget(tabWidget);

	// about tab
	QWidget *tab1 = new QWidget(m_mainWindow);
	tabWidget->addTab(tab1, tr("About"));
	QVBoxLayout *tab1Layout = new QVBoxLayout;
	tab1->setLayout(tab1Layout);

	QLabel *iconLabel = new QLabel;
	QPixmap pixmap(":icon-96.png");
	iconLabel->setPixmap(pixmap);
#ifdef Q_WS_MAC
	iconLabel->setMask(pixmap.mask());
#endif

	QHBoxLayout *iconLayout = new QHBoxLayout();
	iconLayout->addStretch();
	iconLayout->addWidget(iconLabel);
	iconLayout->addStretch();
	tab1Layout->addLayout(iconLayout);

	QTextBrowser *aboutTextBrowser = new QTextBrowser(this);
	aboutTextBrowser->setStyleSheet("background: transparent");
	aboutTextBrowser->setFrameShape(QFrame::NoFrame);
	aboutTextBrowser->setHtml("<center>" + aboutHtml + "</center>");
	aboutTextBrowser->setMinimumWidth(350);
	aboutTextBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	aboutTextBrowser->setOpenExternalLinks(TRUE);

	tab1Layout->addWidget(aboutTextBrowser);
	//

	// changelog tab
	QWidget *tab2 = new QWidget(m_mainWindow);
	tabWidget->addTab(tab2, tr("Changelog"));
	QVBoxLayout *tab2Layout = new QVBoxLayout;
	tab2Layout->setContentsMargins(0, 0, 0, 0);
	tab2->setLayout(tab2Layout);

	QFile file( ":/ChangeLog");
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	QTextStream stream(&file);
	QString changelogHtml = stream.readAll();
	changelogHtml.replace("\n", "<br>\n");
	changelogHtml.replace(QRegExp("(\\*[^<]*)(<br>)"), "<b>\\1</b>\\2");
	file.close();

	QTextBrowser *changelogTextBrowser = new QTextBrowser(this);
	changelogTextBrowser->setHtml(changelogHtml);
	changelogTextBrowser->setOpenExternalLinks(TRUE);
	tab2Layout->addWidget(changelogTextBrowser);
	//

	QPushButton *closeButton = new QPushButton("Close");
	connect(closeButton, SIGNAL(clicked()), dialog, SLOT(accept()));
	QHBoxLayout *buttonLayout = new QHBoxLayout();
	buttonLayout->addStretch();
	buttonLayout->addWidget(closeButton);
	buttonLayout->addStretch();
	layout->addLayout(buttonLayout);

	dialog->show();

	// resize according to content
	QSize aboutTextSize = aboutTextBrowser->document()->size().toSize();
	aboutTextBrowser->setMinimumHeight(aboutTextSize.height());
}

void NPlayer::showOpenFileDialog()
{
	QString music = "*.mp3 *.ogg *.mp4 *.wma "
	                "*.flac *.ape *.wav *.wv "
	                "*.mpc *.spx *.opus "
	                "*.m4a *.aac "
	                "*.xm *.s3m *.it *.mod";
	QString playlist = "*.m3u *.m3u8";

	QStringList files = QFileDialog::getOpenFileNames(
		m_mainWindow, qobject_cast<QAction *>(QObject::sender())->text().remove("..."),
		m_settings->value("LastDirectory").toString(),
		"All supported (" + music + " " + playlist + ");;"
		"Music files (" + music + ");;"
		"Playlist files (" + playlist + ");;"
		"All files (*)"
	);

	if (files.isEmpty())
		return;

	QString lastDir = QFileInfo(files.first()).path();
	m_settings->setValue("LastDirectory", lastDir);

	bool isEmpty = (m_playlistWidget->count() == 0);
	m_playlistWidget->addFiles(files);
	if (isEmpty)
		m_playlistWidget->playFirst();
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
	m_playlistWidget->addFiles(NCore::dirListRecursive(dir));
	if (isEmpty)
		m_playlistWidget->playFirst();
}

void NPlayer::showSavePlaylistDialog()
{
	QString file = QFileDialog::getSaveFileName(
		m_mainWindow, qobject_cast<QAction *>(QObject::sender())->text().remove("..."),
		m_settings->value("LastDirectory").toString(),
		"M3U Playlist (*.m3u)"
	);

	if (file.isEmpty())
		return;

	QString lastDir = QFileInfo(file).path();
	m_settings->setValue("LastDirectory", lastDir);

	if (!file.endsWith(".m3u"))
		file.append(".m3u");

	writePlaylist(file);
}

void NPlayer::showContextMenu(QPoint pos)
{
	m_contextMenu->exec(m_mainWindow->mapToGlobal(pos));
}

