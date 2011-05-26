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

#include "settings.h"
#include "rcDir.h"
#include "pluginInterface.h"
#include "pluginLoader.h"
#include "skinLoader.h"
#include "skinFileSystem.h"
#include "widgetPrototype.h"
#include "playlist.h"
#include "arguments.h"
#include "action.h"

#include <QFileInfo>
#include <QPluginLoader>
#include <QFileDialog>
#include <QDesktopServices>
#include <QMetaObject>
#include <QNetworkRequest>

#include <QDebug>

Q_DECLARE_METATYPE(QWidget *)
Q_DECLARE_METATYPE(QPushButton *)
NWidgetPrototype wProto;

struct QtMetaObject : private QObject
{
public:
	static const QMetaObject *get() { return &static_cast<QtMetaObject *>(0)->staticQtMetaObject; }
};

NPlayer::NPlayer()
{
	setObjectName("NPlayer");

	m_networkManager = new QNetworkAccessManager(this);
	m_networkManager->setObjectName("networkManager");

	m_localPlaylist = rcDir() + "/" + applicationBinaryName() + ".m3u";

	settings()->setParent(this);
	m_playbackEngine = playbackPlugin(settings());
	m_playbackEngine->setParent(this);
	m_playbackEngine->setObjectName("playbackEngine");

	m_mainWindow = new NMainWindow();
	connect(m_mainWindow, SIGNAL(closed()), this, SLOT(mainWindowClosed()));
	m_mainWindow->init(skinUiFormFile(settings()));

	m_preferencesDialog = new NPreferencesDialog(m_mainWindow);
	connect(m_preferencesDialog, SIGNAL(settingsChanged()), this, SLOT(preferencesDialogSettingsChanged()));
	connect(m_preferencesDialog, SIGNAL(versionCheckOnline()), this, SLOT(versionCheckOnline()));

	m_playlistWidget = qFindChild<NPlaylistWidget *>(m_mainWindow, "playlistWidget");

	// loading script
	m_scriptEngine = new QScriptEngine(this);
	QString scriptFileName(skinScriptFile(settings()));
	QFile scriptFile(scriptFileName);
	scriptFile.open(QIODevice::ReadOnly);
	m_scriptEngine->evaluate(scriptFile.readAll(), scriptFileName);
	scriptFile.close();

	QScriptValue Qt = m_scriptEngine->newQMetaObject(QtMetaObject::get());
	m_scriptEngine->globalObject().setProperty("Qt", Qt);

	m_scriptEngine->setDefaultPrototype(qMetaTypeId<QWidget *>(), m_scriptEngine->newQObject(&wProto));

	QScriptValue constructor = m_scriptEngine->evaluate("Program");
	QScriptValue playbackEngineObject = m_scriptEngine->newQObject(m_playbackEngine, QScriptEngine::QtOwnership);
	QScriptValue windowScriptObject = m_scriptEngine->newQObject(m_mainWindow, QScriptEngine::QtOwnership);
	QScriptValue programmObject = constructor.construct(QScriptValueList() << windowScriptObject << playbackEngineObject);

	// actions
	NAction *playAction = new NAction(style()->standardIcon(QStyle::SP_MediaPlay), tr("Play / Pause"), this);
	playAction->setObjectName("playAction");
	playAction->setStatusTip(tr("Toggle playback"));
	playAction->setGlobal(TRUE);
	connect(playAction, SIGNAL(triggered()), m_playbackEngine, SLOT(play()));

	NAction *stopAction = new NAction(style()->standardIcon(QStyle::SP_MediaStop), tr("Stop"), this);
	stopAction->setObjectName("stopAction");
	stopAction->setStatusTip(tr("Stop playback"));
	stopAction->setGlobal(TRUE);
	connect(stopAction, SIGNAL(triggered()), m_playbackEngine, SLOT(stop()));

	NAction *prevAction = new NAction(style()->standardIcon(QStyle::SP_MediaSkipBackward), tr("Previous"), this);
	prevAction->setObjectName("prevAction");
	prevAction->setStatusTip(tr("Play previous track in playlist"));
	prevAction->setGlobal(TRUE);
	connect(prevAction, SIGNAL(triggered()), m_playlistWidget, SLOT(activatePrev()));

	NAction *nextAction = new NAction(style()->standardIcon(QStyle::SP_MediaSkipBackward), tr("Next"), this);
	nextAction->setObjectName("nextAction");
	nextAction->setStatusTip(tr("Play next track in playlist"));
	nextAction->setGlobal(TRUE);
	connect(nextAction, SIGNAL(triggered()), m_playlistWidget, SLOT(activateNext()));

	NAction *preferencesAction = new NAction(QIcon::fromTheme("preferences-desktop",
											style()->standardIcon(QStyle::SP_MessageBoxInformation)),
											tr("Preferences..."), this);
	connect(preferencesAction, SIGNAL(triggered()), this, SLOT(showPreferencesDialog()));

	NAction *exitAction = new NAction(QIcon::fromTheme("exit",
										style()->standardIcon(QStyle::SP_DialogCloseButton)),
										tr("Exit"), this);
	connect(exitAction, SIGNAL(triggered()), this, SLOT(quit()));

	NAction *fileDialogAction = new NAction(style()->standardIcon(QStyle::SP_DialogOpenButton), tr("Open Files..."), this);
	connect(fileDialogAction, SIGNAL(triggered()), this, SLOT(showFileDialog()));

	NAction *aboutAction = new NAction(QIcon::fromTheme("help-about",
										style()->standardIcon(QStyle::SP_MessageBoxQuestion)),
										tr("About") + " " + QCoreApplication::applicationName(), this);
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(showAboutMessageBox()));

	NAction *alwaysOnTopAction = new NAction(tr("Always On Top"), this);
	alwaysOnTopAction->setCheckable(TRUE);
	alwaysOnTopAction->setObjectName("alwaysOnTopAction");

	// tray icon
	QMenu *trayIconMenu = new QMenu(this);
	trayIconMenu->addAction(playAction);
	trayIconMenu->addAction(stopAction);
	trayIconMenu->addAction(preferencesAction);
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(exitAction);
	m_trayIcon = new QSystemTrayIcon(this);
	m_trayIcon->setObjectName("trayIcon");
	m_trayIcon->setContextMenu(trayIconMenu);
	m_trayIcon->setIcon(m_mainWindow->windowIcon());

	// context menu
	m_contextMenu = new QMenu(m_mainWindow);
	m_mainWindow->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_mainWindow, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
	m_contextMenu->addAction(fileDialogAction);
	m_contextMenu->addAction(alwaysOnTopAction);
	m_contextMenu->addAction(preferencesAction);
	m_contextMenu->addSeparator();
	m_contextMenu->addAction(aboutAction);
	m_contextMenu->addSeparator();
	m_contextMenu->addAction(exitAction);

	if (QCoreApplication::arguments().size() > 1) {
		QStringList pathList;
		for (int i = 1; i < QCoreApplication::arguments().size(); ++i) {
			QString file = QCoreApplication::arguments().at(i);
			if (QFile(file).exists())
			pathList << file;
		}
		m_playlistWidget->activateMediaList(pathList);
	}

	m_preferencesDialog->initShortcuts();

	loadSettings();
	saveSettings();

	QMetaObject::connectSlotsByName(this);

	restorePlaylist();

	m_mainWindow->setTitle("");
	m_mainWindow->show();
	QResizeEvent e(m_mainWindow->size(), m_mainWindow->size());
	QCoreApplication::sendEvent(m_mainWindow, &e);
}

NPlayer::~NPlayer()
{
	delete m_mainWindow;
}

void NPlayer::message(const QString &str)
{
	QString prefix = "files:";
	if (str.startsWith(prefix))
		m_playlistWidget->activateMediaList(str.mid(prefix.size()).split("<|>"));
}

void NPlayer::restorePlaylist()
{
	QStringList files = playlistParse(m_localPlaylist);
	if (files.isEmpty())
		return;
	m_playlistWidget->setMediaList(files);

	QStringList playlistRowValues = settings()->value("PlaylistRow", QStringList()).toStringList();
	if (!playlistRowValues.isEmpty()) {
		if (settings()->value("RestorePlayback").toBool()) {
			m_playlistWidget->activateRow(playlistRowValues.at(0).toInt());
			m_playbackEngine->setPosition(playlistRowValues.at(1).toFloat());
		} else {
			m_playlistWidget->setCurrentRow(playlistRowValues.at(0).toInt());
		}
	}
}

void NPlayer::savePlaylist()
{
	playlistWrite(m_localPlaylist, m_playlistWidget->mediaList());

	int row = m_playlistWidget->currentRow();
	qreal pos = m_playbackEngine->position();
	settings()->setValue("PlaylistRow", QStringList() << QString::number(row) << QString::number(pos));
}

void NPlayer::loadSettings()
{
	m_mainWindow->loadSettings();
	m_preferencesDialog->loadSettings();
	m_playbackEngine->setVolume(settings()->value("Volume", 0.8).toFloat());
	m_trayIcon->setVisible(settings()->value("GUI/TrayIcon").toBool());

	if (settings()->value("AutoCheckUpdates", FALSE).toBool())
		versionCheckOnline();

	bool onTop = settings()->value("GUI/AlwaysOnTop", FALSE).toBool();
	if (onTop) {
		NAction *alwaysOnTopAction = qFindChild<NAction *>(this, "alwaysOnTopAction");
		alwaysOnTopAction->setChecked(TRUE);
		on_alwaysOnTopAction_toggled(TRUE);
	}
}

void NPlayer::saveSettings()
{
	settings()->setValue("Volume", QString::number(m_playbackEngine->volume()));
	m_mainWindow->saveSettings();
	m_preferencesDialog->saveSettings();

	NAction *alwaysOnTopAction = qFindChild<NAction *>(this, "alwaysOnTopAction");
	settings()->setValue("GUI/AlwaysOnTop", alwaysOnTopAction->isChecked());
}

void NPlayer::preferencesDialogSettingsChanged()
{
	m_trayIcon->setVisible(settings()->value("GUI/TrayIcon").toBool());
}

QString NPlayer::about()
{
	return QString() +
			"<b>" +  QCoreApplication::applicationName() + "</b> Music Player, " +
				"<a href='http://" + QCoreApplication::organizationDomain() + "'>http://" +
					QCoreApplication::organizationDomain() + "</a><br>" +
			"Version: " + QCoreApplication::applicationVersion() +
				(QString(_N_TIME_STAMP_).isEmpty() ? "" : " (Build " + QString(_N_TIME_STAMP_) + ")") + "<br><br>" +
			"Copyright (C) 2010-2011  Sergey Vlasov &lt;<a href='mailto:Sergey Vlasov <sergey@vlasov.me>" +
				"?subject=" + QCoreApplication::applicationName() + " " +
				QCoreApplication::applicationVersion() + "'>sergey@vlasov.me</a>&gt;";
}

void NPlayer::versionCheckOnline()
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
		m_networkManager->get(QNetworkRequest(QUrl("http://nulloy.com/version_" + suffix)));
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
					QCoreApplication::organizationDomain() + "</a>");
		}
	}

	reply->deleteLater();
}

void NPlayer::mainWindowClosed()
{
	if (settings()->value("GUI/MinimizeToTray", FALSE).toBool()) {
		m_trayIcon->setVisible(TRUE);
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
		if (settings()->value("GUI/MinimizeToTray").toBool()) {
			if (minimized && visible) {
				m_mainWindow->showNormal();
				m_mainWindow->activateWindow();
			} else {
				m_mainWindow->setVisible(!visible);
			}
			if (!settings()->value("GUI/TrayIcon").toBool())
				m_trayIcon->setVisible(!visible);
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

void NPlayer::on_playbackEngine_message(QMessageBox::Icon icon, const QString &title, const QString &msg)
{
	QMessageBox box(icon, title, msg, QMessageBox::Close, m_mainWindow);
	box.exec();
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
	m_trayIcon->setToolTip(title);
}

void NPlayer::on_alwaysOnTopAction_toggled(bool checked)
{
	Qt::WindowFlags flags = m_mainWindow->windowFlags();
	if (checked)
		flags |= Qt::WindowStaysOnTopHint;
	else
		flags ^= Qt::WindowStaysOnTopHint;
	m_mainWindow->setWindowFlags(flags);
	m_mainWindow->show();
}

void NPlayer::showPreferencesDialog()
{
	m_preferencesDialog->show();
}

void NPlayer::showAboutMessageBox()
{
	QMessageBox::about(m_mainWindow, QObject::tr("About ") + QCoreApplication::applicationName(), about());
}

void NPlayer::showFileDialog()
{
	QStringList files = QFileDialog::getOpenFileNames(m_mainWindow, tr("Open Files"),
						QDesktopServices::storageLocation(QDesktopServices::MusicLocation));

	m_playlistWidget->appendMediaList(files);
}

void NPlayer::showContextMenu(QPoint pos)
{
	m_contextMenu->exec(m_mainWindow->mapToGlobal(pos));
}

/* vim: set ts=4 sw=4: */
