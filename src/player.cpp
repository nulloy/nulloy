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

#include "player.h"

#include "scriptPrototypes.h"
#include "core.h"
#include "action.h"
#include "systemTray.h"
#include "m3uPlaylist.h"

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

#include <QFileInfo>
#include <QPluginLoader>
#include <QFileDialog>
#include <QMetaObject>
#include <QNetworkRequest>
#include <QTextBrowser>

#include <QDebug>

NWidgetPrototype widgetPrototype;
NLayoutPrototype layoutPrototype;
Q_DECLARE_METATYPE(QWidget *)
Q_DECLARE_METATYPE(QLayout *)
Q_DECLARE_METATYPE(NMainWindow *)
Q_DECLARE_METATYPE(NPlaybackEngineInterface *)
Q_DECLARE_METATYPE(QList<QWidget *>)
Q_DECLARE_METATYPE(QMargins)


struct QtMetaObject : private QObject
{
public:
	static const QMetaObject *get() { return &static_cast<QtMetaObject *>(0)->staticQtMetaObject; }
};

template <typename T>
QScriptValue qScriptValueFromQObject(QScriptEngine *engine, T const &obj)
{
	return engine->newQObject(obj);
}

template <typename T>
void qScriptValueToQObject(const QScriptValue &value, T &obj)
{
	obj = qobject_cast<T>(value.toQObject());
}

template <typename T>
int qScriptRegisterQObjectMetaType(QScriptEngine *engine, const QScriptValue &prototype = QScriptValue(), T *obj = 0)
{
	Q_UNUSED(obj);
	return qScriptRegisterMetaType<T>(engine, qScriptValueFromQObject, qScriptValueToQObject, prototype);
}

NPlayer::NPlayer()
{
	setObjectName("NPlayer");
	m_settings = new NSettings(this);


	// construct playbackEngine
#ifndef _N_NO_PLUGINS_
	m_playbackEngine = NPluginLoader::playbackPlugin();
#else
	m_playbackEngine = dynamic_cast<NPlaybackEngineInterface *>(new NPlaybackEngineGStreamer());
	dynamic_cast<NPluginInterface *>(m_playbackEngine)->init();
#endif
	m_playbackEngine->setParent(this);
	m_playbackEngine->setObjectName("playbackEngine");
	//


	// construct mainWindow
	m_mainWindow = new NMainWindow();
	m_mainWindow->setObjectName("mainWindow");
	connect(m_mainWindow, SIGNAL(closed()), this, SLOT(mainWindowClosed()));
#ifndef _N_NO_SKINS_
	m_mainWindow->init(NSkinLoader::skinUiFormFile());
#else
	m_mainWindow->init(QString());
#endif
	//


	// loading script
	m_scriptEngine = new QScriptEngine(this);
#ifndef _N_NO_SKINS_
	QString scriptFileName(NSkinLoader::skinScriptFile());
#else
	QString scriptFileName(":skins/native/script.js");
#endif
	QFile scriptFile(scriptFileName);
	scriptFile.open(QIODevice::ReadOnly);
	m_scriptEngine->evaluate(scriptFile.readAll(), scriptFileName);
	scriptFile.close();

	QScriptValue Qt = m_scriptEngine->newQMetaObject(QtMetaObject::get());
	m_scriptEngine->globalObject().setProperty("Qt", Qt);

	QString ws;
#if defined Q_WS_MAC
	ws = "mac";
#elif defined Q_WS_WIN
	ws = "win";
#endif
	m_scriptEngine->globalObject().setProperty("Q_WS", ws);

	qScriptRegisterQObjectMetaType<NMainWindow *>(m_scriptEngine);
	qScriptRegisterQObjectMetaType<NPlaybackEngineInterface *>(m_scriptEngine);
	qScriptRegisterMetaType(m_scriptEngine, NMarginsPrototype::toScriptValue, NMarginsPrototype::fromScriptValue);
	m_scriptEngine->setDefaultPrototype(qMetaTypeId<QWidget *>(), m_scriptEngine->newQObject(&widgetPrototype));
	m_scriptEngine->setDefaultPrototype(qMetaTypeId<QLayout *>(), m_scriptEngine->newQObject(&layoutPrototype));
	qScriptRegisterSequenceMetaType< QList<QWidget *> >(m_scriptEngine);

	QScriptValue constructor = m_scriptEngine->evaluate("Program");
	QScriptValue playerEngineObject = m_scriptEngine->newQObject(this, QScriptEngine::QtOwnership);
	constructor.construct(QScriptValueList() << playerEngineObject);
	//


	m_preferencesDialog = new NPreferencesDialog(m_mainWindow);
	connect(m_preferencesDialog, SIGNAL(settingsChanged()), this, SLOT(preferencesDialogSettingsChanged()));
	connect(m_preferencesDialog, SIGNAL(versionOnlineRequested()), this, SLOT(versionOnlineFetch()));

	m_playlistWidget = qFindChild<NPlaylistWidget *>(m_mainWindow, "playlistWidget");


	// actions
	NAction *playAction = new NAction(style()->standardIcon(QStyle::SP_MediaPlay), tr("Play / Pause"), this);
	playAction->setObjectName("playAction");
	playAction->setStatusTip(tr("Toggle playback"));
	playAction->setGlobal(TRUE);
	playAction->setCustomizable(TRUE);
	connect(playAction, SIGNAL(triggered()), m_playbackEngine, SLOT(play()));

	NAction *stopAction = new NAction(style()->standardIcon(QStyle::SP_MediaStop), tr("Stop"), this);
	stopAction->setObjectName("stopAction");
	stopAction->setStatusTip(tr("Stop playback"));
	stopAction->setGlobal(TRUE);
	stopAction->setCustomizable(TRUE);
	connect(stopAction, SIGNAL(triggered()), m_playbackEngine, SLOT(stop()));

	NAction *prevAction = new NAction(style()->standardIcon(QStyle::SP_MediaSkipBackward), tr("Previous"), this);
	prevAction->setObjectName("prevAction");
	prevAction->setStatusTip(tr("Play previous track in playlist"));
	prevAction->setGlobal(TRUE);
	prevAction->setCustomizable(TRUE);
	connect(prevAction, SIGNAL(triggered()), m_playlistWidget, SLOT(activatePrev()));

	NAction *nextAction = new NAction(style()->standardIcon(QStyle::SP_MediaSkipForward), tr("Next"), this);
	nextAction->setObjectName("nextAction");
	nextAction->setStatusTip(tr("Play next track in playlist"));
	nextAction->setGlobal(TRUE);
	nextAction->setCustomizable(TRUE);
	connect(nextAction, SIGNAL(triggered()), m_playlistWidget, SLOT(activateNext()));

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

	NAction *savePlaylistDialogAction = new NAction(style()->standardIcon(QStyle::SP_DialogSaveButton), tr("Save Playlist..."), this);
	connect(savePlaylistDialogAction, SIGNAL(triggered()), this, SLOT(showSavePlaylistDialog()));

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

	QActionGroup* group = new QActionGroup(this);
	loadNextNameDownAction->setActionGroup(group);
	loadNextNameUpAction->setActionGroup(group);
	loadNextDateDownAction->setActionGroup(group);
	loadNextDateUpAction->setActionGroup(group);
	//


	// tray icon
	QMenu *trayIconMenu = new QMenu(this);
	trayIconMenu->addAction(playAction);
	trayIconMenu->addAction(stopAction);
	trayIconMenu->addAction(prevAction);
	trayIconMenu->addAction(nextAction);
	trayIconMenu->addAction(preferencesAction);
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(exitAction);
	NSystemTray::init(this);
	NSystemTray::setContextMenu(trayIconMenu);
#ifdef Q_WS_MAC
	NSystemTray::setIcon(QIcon(":mac-systray.png"));
#else
	NSystemTray::setIcon(m_mainWindow->windowIcon());
#endif
	//


	// context menu
	m_contextMenu = new QMenu(m_mainWindow);
	m_mainWindow->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_mainWindow, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
	m_contextMenu->addAction(openFileDialogAction);
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

	m_networkManager = new QNetworkAccessManager(this);
	m_networkManager->setObjectName("networkManager");

	m_localPlaylist = NCore::rcDir() + "/" + NCore::applicationBinaryName() + ".m3u";

	m_logDialog = new NLogDialog(m_mainWindow);
	connect(m_playbackEngine, SIGNAL(message(QMessageBox::Icon, const QString &, const QString &)),
			m_logDialog, SLOT(showMessage(QMessageBox::Icon, const QString &, const QString &)));

	QMetaObject::connectSlotsByName(this);

	m_settings->initShortcuts(this);
	m_settings->loadShortcuts();
	foreach (NAction *action, m_settings->shortcuts())
		m_mainWindow->addAction(action);

	loadSettings();

	m_mainWindow->setTitle("");

	QStringList pathList;
	if (QCoreApplication::arguments().size() > 1) {
		for (int i = 1; i < QCoreApplication::arguments().size(); ++i) {
			QString file = QCoreApplication::arguments().at(i);
			if (QFile(file).exists())
				pathList << file;
		}
	}
	if (!pathList.isEmpty())
		m_playlistWidget->activateMediaList(pathList);
	else
		restorePlaylist();

	m_mainWindow->show();
	QResizeEvent e(m_mainWindow->size(), m_mainWindow->size());
	QCoreApplication::sendEvent(m_mainWindow, &e);
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

void NPlayer::message(const QString &str)
{
	qDebug() << str;
	QStringList argList = str.split("<|>");
	QStringList pathList;
	QStringList notPathArgList;
	foreach (QString arg, argList) {
		if (QFile(arg).exists())
			pathList << arg;
		else
			notPathArgList << arg;
	}

	foreach (QString arg, notPathArgList) {
		if (arg == "--next")
			m_playlistWidget->activateNext();
		else if (arg == "--prev")
			m_playlistWidget->activatePrev();
		else if (arg == "--stop")
			m_playbackEngine->stop();
		else if (arg == "--pause")
			m_playlistWidget->activateCurrent();
	}

	if (!pathList.isEmpty())
		m_playlistWidget->activateMediaList(pathList);
}

void NPlayer::restorePlaylist()
{
	m_playlistWidget->setMediaListFromPlaylist(m_localPlaylist);

	QStringList playlistRowValues = m_settings->value("PlaylistRow").toStringList();
	if (!playlistRowValues.isEmpty()) {
		m_playlistWidget->setCurrentRow(playlistRowValues.at(0).toInt());
		qreal pos = playlistRowValues.at(1).toFloat();
		if (m_settings->value("RestorePlayback").toBool() && pos != 0 && pos != 1) {
			m_playbackEngine->play();
			m_playbackEngine->setPosition(pos);
		}
	}
}

void NPlayer::savePlaylist()
{
	m_playlistWidget->writePlaylist(m_localPlaylist);

	int row = m_playlistWidget->currentRow();
	qreal pos = m_playbackEngine->position();
	m_settings->setValue("PlaylistRow", QStringList() << QString::number(row) << QString::number(pos));
}

void NPlayer::loadSettings()
{
	NSystemTray::setEnabled(m_settings->value("GUI/TrayIcon").toBool());

	if (m_settings->value("AutoCheckUpdates").toBool())
		versionOnlineFetch();

	bool alwaysOnTop = m_settings->value("GUI/AlwaysOnTop").toBool();
	if (alwaysOnTop) {
		NAction *alwaysOnTopAction = qFindChild<NAction *>(this, "alwaysOnTopAction");
		alwaysOnTopAction->setChecked(TRUE);
		on_alwaysOnTopAction_toggled(TRUE);
	}

	bool whilePlaying = m_settings->value("GUI/WhilePlayingOnTop").toBool();
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

void NPlayer::preferencesDialogSettingsChanged()
{
	NSystemTray::setEnabled(m_settings->value("GUI/TrayIcon").toBool());
}

void NPlayer::versionOnlineFetch()
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
		m_networkManager->get(QNetworkRequest(QUrl("http://" +
								QCoreApplication::organizationDomain() + "/version_" + suffix)));
}

void NPlayer::on_networkManager_finished(QNetworkReply *reply)
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

void NPlayer::mainWindowClosed()
{
	if (m_settings->value("GUI/MinimizeToTray").toBool()) {
		NSystemTray::setEnabled(TRUE);
	} else {
		quit();
	}
}

void NPlayer::on_trayIcon_activated(QSystemTrayIcon::ActivationReason reason)
{
	bool visible = m_mainWindow->isVisible();
	bool minimized = (bool)(m_mainWindow->windowState() & Qt::WindowMinimized);
	if (reason == QSystemTrayIcon::Trigger) {
		if (!minimized && visible) {
			m_mainWindow->showNormal();
			m_mainWindow->activateWindow();
		}
	} else if (reason == QSystemTrayIcon::DoubleClick) {
		if (m_settings->value("GUI/MinimizeToTray").toBool()) {
			if (minimized && visible) {
				m_mainWindow->showNormal();
				m_mainWindow->activateWindow();
			} else {
				m_mainWindow->setVisible(!visible);
			}
			if (!m_settings->value("GUI/TrayIcon").toBool())
				NSystemTray::setEnabled(!visible);
		} else {
			if (!minimized) {
				m_mainWindow->showMinimized();
			} else {
				m_mainWindow->showNormal();
				m_mainWindow->activateWindow();
			}
		}
	}
}

void NPlayer::quit()
{
	savePlaylist();
	saveSettings();
	QCoreApplication::quit();
}

void NPlayer::on_playbackEngine_mediaChanged(const QString &path)
{
	if (path.isEmpty())
		return;

	QString title;
	if (QFile(path).exists())
		title = QFileInfo(path).fileName();
	else
		title = "";
	m_mainWindow->setTitle(title);
	NSystemTray::setToolTip(title);
}

void NPlayer::on_playbackEngine_stateChanged(int state)
{
	bool whilePlaying = m_settings->value("GUI/WhilePlayingOnTop").toBool();
	bool alwaysOnTop = m_settings->value("GUI/AlwaysOnTop").toBool();
	if (!alwaysOnTop)
		m_mainWindow->setOnTop(whilePlaying && state == NPlaybackEngineInterface::Playing);
#ifdef Q_WS_WIN
	if (state == NPlaybackEngineInterface::Playing) {
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
	m_settings->setValue("GUI/AlwaysOnTop", checked);

	bool whilePlaying = m_settings->value("GUI/WhilePlayingOnTop").toBool();
	if (!whilePlaying || m_playbackEngine->state() != NPlaybackEngineInterface::Playing)
		m_mainWindow->setOnTop(checked);
}

void NPlayer::on_whilePlayingOnTopAction_toggled(bool checked)
{
	m_settings->setValue("GUI/WhilePlayingOnTop", checked);

	bool alwaysOnTop = m_settings->value("GUI/AlwaysOnTop").toBool();
	if (!alwaysOnTop)
		m_mainWindow->setOnTop(checked && m_playbackEngine->state() == NPlaybackEngineInterface::Playing);
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

void NPlayer::showAboutMessageBox()
{
	QString html = QString() +
#ifdef Q_WS_MAC
					"<span style=\" font-size:14pt;\">" +
#else
					"<span style=\" font-size:9pt;\">" +
#endif
						"<b>" +  QCoreApplication::applicationName() + "</b> Music Player " +
							"<a href='http://" + QCoreApplication::organizationDomain() + "'>http://" +
								QCoreApplication::organizationDomain() + "</a>" +
					"</span><br>" +
#ifdef Q_WS_MAC
					"<span style=\" font-size:10pt;\">" +
#else
					"<span style=\" font-size:8pt;\">" +
#endif
						"Version: " + QCoreApplication::applicationVersion() +
							(QString(_N_TIME_STAMP_).isEmpty() ? "" : " (Build " + QString(_N_TIME_STAMP_) + ")") + "<br><br>" +
						"Copyright (C) 2010-2011  Sergey Vlasov &lt;<a href='mailto:Sergey Vlasov <sergey@vlasov.me>" +
							"?subject=" + QCoreApplication::applicationName() + " " +
							QCoreApplication::applicationVersion() + "'>sergey@vlasov.me</a>&gt;" +
					"</span>";

	QDialog *dialog = new QDialog(m_mainWindow);
	dialog->setWindowTitle(QObject::tr("About ") + QCoreApplication::applicationName());
	dialog->setMaximumSize(0, 0);
	QVBoxLayout *layout = new QVBoxLayout;
	dialog->setLayout(layout);

	QLabel *iconLabel = new QLabel;
	QPixmap pixmap(":icon-96.png");
	iconLabel->setPixmap(pixmap);
#ifdef Q_WS_MAC
	iconLabel->setMask(pixmap.mask());
#endif

	QHBoxLayout* iconLayout = new QHBoxLayout();
	iconLayout->addStretch();
	iconLayout->addWidget(iconLabel);
	iconLayout->addStretch();
	layout->addLayout(iconLayout);

	QTextBrowser *textBrowser = new QTextBrowser(this);
	textBrowser->setStyleSheet("background: transparent");
	textBrowser->setFrameShape(QFrame::NoFrame);
	textBrowser->setHtml("<center>" + html + "</center>");

	textBrowser->setMinimumWidth(350);
	layout->addWidget(textBrowser);

	QPushButton *closeButton = new QPushButton("Close");
	connect(closeButton, SIGNAL(clicked()), dialog, SLOT(accept())); \
	QHBoxLayout* buttonLayout = new QHBoxLayout();
	buttonLayout->addStretch();
	buttonLayout->addWidget(closeButton);
	buttonLayout->addStretch();
	layout->addLayout(buttonLayout);

	dialog->exec();
}

void NPlayer::showOpenFileDialog()
{
	QString music = "*.mp3 *.ogg *.flac *.wma *.wav "
					"*.aac *.m4a *.spx *.mp4 "
					"*.xm *.s3m *.it *.mod";
	QString playlist = "*.m3u *.m3u8";

	QStringList files = QFileDialog::getOpenFileNames(m_mainWindow, tr("Open Files"),
														m_settings->value("LastDirectory").toString(),
														"All supported (" + music + " " + playlist + ");;"
														"Music files (" + music + ");;"
														"Playlist files (" + playlist + ");;"
														"All files (*)");

	if (files.isEmpty())
		return;

	QString lastDir = QFileInfo(files.first()).path();
	m_settings->setValue("LastDirectory", lastDir);

	bool isEmpty = (m_playlistWidget->count() == 0);
	m_playlistWidget->appendMediaList(files);
	if (isEmpty)
		m_playlistWidget->activateFirst();
}

void NPlayer::showSavePlaylistDialog()
{
	QString file = QFileDialog::getSaveFileName(this, tr("Save File"),
												m_settings->value("LastDirectory").toString(),
												"M3U Playlist (*.m3u)");

	if (file.isEmpty())
		return;

	QString lastDir = QFileInfo(file).path();
	m_settings->setValue("LastDirectory", lastDir);

	if (!file.endsWith(".m3u"))
		file.append(".m3u");

	m_playlistWidget->writePlaylist(file);
}

void NPlayer::showContextMenu(QPoint pos)
{
	m_contextMenu->exec(m_mainWindow->mapToGlobal(pos));
}

/* vim: set ts=4 sw=4: */
