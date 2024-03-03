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

#include "player.h"

#include "action.h"
#include "actionManager.h"
#include "common.h"
#include "coverWidget.h"
#include "cursorOverride.h"
#include "i18nLoader.h"
#include "image.h"
#include "logDialog.h"
#include "mainWindow.h"
#include "playbackEngineInterface.h"
#include "playlistController.h"
#include "playlistDataItem.h"
#include "playlistModel.h"
#include "playlistStorage.h"
#include "playlistWidget.h"
#include "playlistWidgetItem.h"
#include "pluginLoader.h"
#include "preferencesDialogHandler.h"
#include "scriptEngine.h"
#include "settings.h"
#include "svgImage.h"
#include "tagEditorDialog.h"
#include "trackInfoModel.h"
#include "trackInfoReader.h"
#include "trackInfoWidget.h"
#ifndef _N_NO_UPDATE_CHECK_
#include "updateChecker.h"
#endif
#include "utils.h"
#include "volumeSlider.h"
#include "waveformBuilderInterface.h"
#include "waveformSlider.h"
#include "waveformView.h"
#ifndef _N_NO_SKINS_
#include "skinFileSystem.h"
#include "skinLoader.h"
#endif

#ifdef Q_OS_WIN
#include "w7TaskBar.h"
#include "winIcon.h"
#endif

#ifdef Q_OS_MAC
#include "macDock.h"
#endif

#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QMetaObject>
#include <QQmlApplicationEngine>
//#include <QQuickWindow>
#include <QResizeEvent>
#include <QToolTip>

static const qreal kLog10over20 = qLn(10) / 20;

NPlayer::NPlayer()
{
    m_settings = NSettings::instance();

    QString styleName = m_settings->value("Style").toString();
    if (!styleName.isEmpty()) {
        QApplication::setStyle(styleName);
    }

    NI18NLoader::init();
    NPluginLoader::init();

    m_trackInfoReader = new NTrackInfoReader(tagReader(), this);

    m_coverReader = dynamic_cast<NCoverReaderInterface *>(NPluginLoader::getPlugin(N::CoverReader));

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

    m_logDialog = new NLogDialog(m_mainWindow);
    m_volumeSlider = m_mainWindow->findChild<NVolumeSlider *>("volumeSlider");
    m_coverWidget = m_mainWindow->findChild<NCoverWidget *>("coverWidget");

    m_playlistWidget = m_mainWindow->findChild<NPlaylistWidget *>("playlistWidget");
    if (QAbstractButton *repeatButton = m_mainWindow->findChild<QAbstractButton *>(
            "repeatButton")) {
        repeatButton->setChecked(m_playlistWidget->repeatMode());
    }
    m_playlistWidget->setTrackInfoReader(m_trackInfoReader);

    m_trackInfoWidget = new NTrackInfoWidget();
    m_trackInfoWidget->setTrackInfoReader(m_trackInfoReader);
    connect(m_trackInfoWidget, SIGNAL(showToolTip(const QString &)), this,
            SLOT(showToolTip(const QString &)));
    QVBoxLayout *trackInfoLayout = new QVBoxLayout;
    trackInfoLayout->setContentsMargins(0, 0, 0, 0);
    trackInfoLayout->addWidget(m_trackInfoWidget);
    m_waveformSlider = m_mainWindow->findChild<NWaveformSlider *>("waveformSlider");
    m_waveformSlider->setLayout(trackInfoLayout);
    m_trackInfoModel = new NTrackInfoModel(*m_trackInfoReader, this);

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

    m_utils = new NUtils(this);

    m_mainWindow->setTitle(QCoreApplication::applicationName() + " " +
                           QCoreApplication::applicationVersion());
    m_mainWindow->show();
    m_mainWindow->loadSettings();

    m_qmlEngine = new QQmlApplicationEngine();
    QQmlContext *context = m_qmlEngine->rootContext();
    context->setContextProperty("NPlaybackEngine", m_playbackEngine);
    context->setContextProperty("NPlayer", this);

    m_playlistController = new NPlaylistController(*m_playbackEngine, *m_trackInfoReader,
                                                   *m_settings, this);
    auto syncToPlaylist = [&]() {
        QList<NPlaylistDataItem> dataItems;
        NPlaylistModel *model = m_playlistController->model();
        for (size_t row = 0; row < model->size(); ++row) {
            NPlaylistDataItem dataItem(model->data(row, NPlaylistModel::FilePathRole).toString());
            dataItem.id = model->data(row, NPlaylistModel::IdRole).toInt();
            dataItems << dataItem;
        }
        m_playlistWidget->setItems(dataItems);
        ssize_t playingRow = model->playingRow();
        if (playingRow >= 0) {
            m_playlistWidget->setPlayingItem(m_playlistWidget->itemAtRow(playingRow));
        }
    };
    connect(m_playlistController->model(), &QAbstractItemModel::rowsMoved, syncToPlaylist);
    connect(m_playlistController->model(), &NPlaylistModel::countChanged, syncToPlaylist);
    connect(m_playlistController, &NPlaylistController::contextMenuRequested, this,
            &NPlayer::showPlaylistContextMenu);
    qmlRegisterType<NPlaylistModel>("NPlaylistModel", 1, 0, "NPlaylistModel");
    context->setContextProperty("NPlaylistController", m_playlistController);
    context->setContextProperty("NUtils", m_utils);

    QResizeEvent e(m_mainWindow->size(), m_mainWindow->size());
    QCoreApplication::sendEvent(m_mainWindow, &e);

    m_actionManager = new NActionManager(this);
    // keyboard shortcuts
    foreach (NAction *action, findChildren<NAction *>()) {
        if (!action->shortcuts().isEmpty()) {
            m_mainWindow->addAction(action);
        }
    }

    m_systemTray = new QSystemTrayIcon(this);
    m_systemTray->setContextMenu(m_actionManager->trayIconMenu());
    m_systemTray->setIcon(m_mainWindow->windowIcon());
    m_trayClickTimer = new QTimer(this);
    m_trayClickTimer->setSingleShot(true);

    loadSettings();

    context->setContextProperty("NSettings", NSettings::instance());
    context->setContextProperty("NSkinFileSystem", NSkinFileSystem::instance());
    context->setContextProperty("_oldMainWindow", m_mainWindow);
    context->setContextProperty("_actionsList", QVariant::fromValue(findChildren<NAction *>()));
    context->setContextProperty("NTrackInfoModel", m_trackInfoModel);
    qmlRegisterType<NWaveformView>("NWaveformView", 1, 0, "NWaveformView");
    qmlRegisterType<NSvgImage>("NSvgImage", 1, 0, "NSvgImage");
    qmlRegisterType<NImage>("NImage", 1, 0, "NImage");

    qmlRegisterSingletonType<NCursorOverride>("NCursorOverride", 1, 0, "NCursorOverride",
                                              [](QQmlEngine *, QJSEngine *) {
                                                  NCursorOverride *instance = new NCursorOverride();
                                                  return instance;
                                              });

    m_qmlEngine->load(QUrl::fromLocalFile("src/mainWindow.qml"));

    m_qmlMainWindow = m_qmlEngine->rootObjects().first();
    m_qmlMainWindow->setProperty("width", m_mainWindow->width());
    m_qmlMainWindow->setProperty("height", m_mainWindow->height());
    m_qmlMainWindow->setProperty("x", m_mainWindow->x() + m_mainWindow->width() + 20);
    //m_qmlMainWindow->setProperty("x", m_mainWindow->x());
    m_qmlMainWindow->setProperty("y", m_mainWindow->y());
    //qobject_cast<QQuickWindow *>(m_qmlMainWindow->property("window").value<QObject *>())
    //    ->setTextRenderType(QQuickWindow::NativeTextRendering);
    QObject::connect(m_qmlMainWindow, SIGNAL(closing(QQuickCloseEvent *)), this,
                     SLOT(on_mainWindow_closed()));
#ifndef _N_NO_UPDATE_CHECK_
    NUpdateChecker::instance().setParentWindow(m_qmlMainWindow);
#endif

    m_coverImage = m_qmlEngine->rootObjects().first()->findChild<NImage *>("coverImage");

    skinProgram.property("afterShow").call(skinProgram);

    if (NSettings::instance()->value("RestorePlaylist").toBool()) {
        loadDefaultPlaylist();
    }

    connectSignals();

    m_settingsSaveTimer = new QTimer(this);
    connect(m_settingsSaveTimer, &QTimer::timeout, this, &NPlayer::saveSettings);
    m_settingsSaveTimer->start(5000); // 5 seconds

    m_writeDefaultPlaylistTimer = new QTimer(this);
    m_writeDefaultPlaylistTimer->setSingleShot(true);
    connect(m_writeDefaultPlaylistTimer, &QTimer::timeout,
            [this]() { writePlaylist(NCore::defaultPlaylistPath(), N::NulloyM3u); });
}

NPlayer::~NPlayer()
{
    NPluginLoader::deinit();
    delete m_mainWindow;
    delete m_settings;
}

NMainWindow *NPlayer::mainWindow()
{
    return m_mainWindow;
}

void NPlayer::connectSignals()
{
    connect(m_playbackEngine, SIGNAL(mediaChanged(const QString &, int)), this,
            SLOT(on_playbackEngine_mediaChanged(const QString &, int)));
    connect(m_playbackEngine, SIGNAL(mediaFailed(const QString &, int)), this,
            SLOT(on_playbackEngine_mediaFailed(const QString &, int)));
    connect(m_playbackEngine, SIGNAL(stateChanged(N::PlaybackState)), this,
            SLOT(on_playbackEngine_stateChanged(N::PlaybackState)));
    connect(m_playbackEngine, SIGNAL(positionChanged(qreal)), m_waveformSlider,
            SLOT(setValue(qreal)));
    connect(m_playbackEngine, SIGNAL(tick(qint64)), m_trackInfoWidget,
            SLOT(updatePlaybackLabels(qint64)));
    connect(m_playbackEngine, SIGNAL(tick(qint64)), m_trackInfoModel, SLOT(updatePlayback(qint64)));
    connect(m_playbackEngine, SIGNAL(message(N::MessageIcon, const QString &, const QString &)),
            m_logDialog, SLOT(showMessage(N::MessageIcon, const QString &, const QString &)));

    connect(m_mainWindow, SIGNAL(closed()), this, SLOT(on_mainWindow_closed()));

    if (QAbstractButton *playButton = m_mainWindow->findChild<QAbstractButton *>("playButton")) {
        connect(playButton, SIGNAL(clicked()), this, SLOT(playPause()));
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
        connect(shuffleButton, SIGNAL(clicked()), m_playlistWidget, SLOT(shufflePlaylist()));
    }

    //connect(m_playlistWidget, SIGNAL(addMoreRequested()), this,
    //        SLOT(on_playlist_addMoreRequested()));
    connect(m_playlistController, &NPlaylistController::addMoreRequested, this,
            &NPlayer::on_playlist_addMoreRequested);
    //connect(m_playlistWidget, &NPlaylistWidget::durationChanged, [this](int durationSec) {
    connect(m_playlistController->model(), &NPlaylistModel::durationChanged, [this](size_t seconds) {
        m_trackInfoReader->updatePlaylistDuration(seconds);
        m_trackInfoWidget->updatePlaylistLabels();
        m_trackInfoModel->updatePlaylistLabels();

        QString format = NSettings::instance()->value("WindowTitleTrackInfo").toString();
        if (!format.isEmpty()) {
            QString title = m_trackInfoReader->toString(format);
            m_mainWindow->setTitle(title);
        }
    });
    connect(m_playlistWidget, &NPlaylistWidget::itemsChanged, [this]() {
        m_writeDefaultPlaylistTimer->start(100); //
    });
    connect(m_playlistWidget, &NPlaylistWidget::playingItemChanged,
            [this]() { savePlaybackState(); });
    //connect(m_playlistWidget, &NPlaylistWidget::playlistFinished, [this]() {
    connect(m_playlistController, &NPlaylistController::playlistFinished, [this]() {
        if (NSettings::instance()->value("QuitWhenFinished").toBool()) {
            QCoreApplication::quit();
        }
    });

    connect(m_waveformSlider, &NWaveformSlider::filesDropped,
            [this](const QList<NPlaylistDataItem> &dataItems) {
                m_playlistWidget->setItems(dataItems);
                m_playlistWidget->playRow(0);
            });
    connect(m_waveformSlider, SIGNAL(sliderMoved(qreal)), m_playbackEngine,
            SLOT(setPosition(qreal)));

    connect(m_mainWindow, SIGNAL(customContextMenuRequested(const QPoint &)), this,
            SLOT(showContextMenu(const QPoint &)));
    connect(m_playlistWidget, &NPlaylistWidget::contextMenuRequested, this,
            &NPlayer::showPlaylistContextMenu);
    connect(m_systemTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this,
            SLOT(on_trayIcon_activated(QSystemTrayIcon::ActivationReason)));
    connect(m_trayClickTimer, SIGNAL(timeout()), this, SLOT(on_trayClickTimer_timeout()));
}

NPlaybackEngineInterface *NPlayer::playbackEngine()
{
    return m_playbackEngine;
}

NPlaylistWidget *NPlayer::playlistWidget()
{
    return m_playlistWidget;
}

NPlaylistController *NPlayer::playlistController()
{
    return m_playlistController;
}

NTagReaderInterface *NPlayer::tagReader()
{
    return dynamic_cast<NTagReaderInterface *>(NPluginLoader::getPlugin(N::TagReader));
}

NCoverWidget *NPlayer::coverWidget()
{
    return m_coverWidget;
}

NSettings *NPlayer::settings()
{
    return m_settings;
}

QString NPlayer::volumeTooltipText(qreal value) const
{
    if (NSettings::instance()->value("ShowDecibelsVolume").toBool()) {
        qreal decibel = 0.67 * log(value) / kLog10over20;
        QString decibelStr;
        decibelStr.setNum(decibel, 'g', 2);
        return QString("%1 %2 dB").arg(tr("Volume")).arg(decibelStr);
    } else {
        return QString("%1 %2\%").arg(tr("Volume")).arg(QString::number(int(value * 100)));
    }
}

bool NPlayer::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FileOpen) {
        QFileOpenEvent *fileEvent = static_cast<QFileOpenEvent *>(event);

        if (!fileEvent->file().isEmpty()) {
            m_playlistWidget->setFiles(QStringList() << fileEvent->file());
            m_playlistWidget->playRow(0);
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
            const ssize_t lastRow = m_playlistController->model()->size() - 1;
            m_playlistController->appendFiles(files);
            if (m_playbackEngine->state() == N::PlaybackStopped ||
                NSettings::instance()->value("PlayEnqueued").toBool()) {
                m_playlistController->playRow(lastRow + 1);
                m_playbackEngine->setPosition(0); // overrides setPosition() in loadDefaultPlaylist()
            }
        } else {
            m_playlistController->setFiles(files);
            m_playlistController->playRow(0);
        }
    }
}

void NPlayer::loadDefaultPlaylist()
{
    QString defaultPlaylistPath = NCore::defaultPlaylistPath();
    if (!QFileInfo(defaultPlaylistPath).exists()) {
        return;
    }

    m_playlistController->setFiles({defaultPlaylistPath});

    QStringList playlistRowValues = m_settings->value("PlaylistRow").toStringList();
    if (!playlistRowValues.isEmpty()) {
        int row = playlistRowValues.at(0).toInt();
        qreal pos = playlistRowValues.at(1).toFloat();
        NPlaylistModel *model = m_playlistController->model();

        if (row < 0 || row > model->size() - 1) {
            return;
        }

        model->setData(row, NPlaylistModel::IsFocusedRole, true);
        model->setData(row, NPlaylistModel::IsSelectedRole, true);

        if (!m_settings->value("StartPaused").toBool()) {
            m_playlistController->playRow(row);
            m_playbackEngine->setPosition(pos);
        } else { // do the work that would have been done upon m_playbackEngine::mediaChanged()
            model->setData(row, NPlaylistModel::IsPlayingtRole, true);
            const QString file = model->data(row, NPlaylistModel::FilePathRole).toString();
            const int id = model->data(row, NPlaylistModel::IdRole).toInt();

            model->countChanged(); // to trigger syncToPlaylist
            m_playbackEngine->setMedia(file, id);
            m_playbackEngine->setPosition(pos);
            m_playbackEngine->positionChanged(pos);

            loadCoverArt(file);

            m_waveformSlider->setMedia(file);
            m_waveformSlider->setValue(pos);
            m_waveformSlider->setPausedState(true);
            m_trackInfoWidget->updateFileLabels(file);
            m_trackInfoModel->updateFileLabels(file);
        }
    }
}

void NPlayer::writePlaylist(const QString &file, N::M3uExtention ext)
{
    QList<NPlaylistDataItem> dataItemsList;
    for (int i = 0; i < m_playlistWidget->count(); ++i) {
        NPlaylistDataItem dataItem = m_playlistWidget->itemAtRow(i)->dataItem();
        dataItemsList << dataItem;
    }
    NPlaylistStorage::writeM3u(file, dataItemsList, ext);
}

void NPlayer::loadSettings()
{
    m_systemTray->setVisible(m_settings->value("TrayIcon").toBool());

#ifndef _N_NO_UPDATE_CHECK_
    if (m_settings->value("AutoCheckUpdates").toBool()) {
        NUpdateChecker::instance().checkOnline();
    }
#endif

    m_playbackEngine->setVolume(m_settings->value("Volume").toFloat());
    m_volumeSlider->setValue(m_settings->value("Volume").toFloat());
    m_playlistController->loadSettings();
}

void NPlayer::saveSettings()
{
    m_settings->setValue("Volume", QString::number(m_playbackEngine->volume()));
    m_mainWindow->saveSettings();
    savePlaybackState();
    m_actionManager->saveSettings();
}

void NPlayer::savePlaybackState()
{
    const int row = m_playlistController->model()->playingRow();
    const qreal pos = m_playbackEngine->position();
    m_settings->setValue("PlaylistRow", QStringList()
                                            << QString::number(row) << QString::number(pos));
}

void NPlayer::applySettings()
{
    m_systemTray->setVisible(m_settings->value("TrayIcon").toBool());
#ifdef Q_OS_WIN
    NW7TaskBar::instance()->setEnabled(NSettings::instance()->value("TaskbarProgress").toBool());
#endif
    m_trackInfoWidget->loadSettings();
    m_trackInfoWidget->updateFileLabels(m_playbackEngine->currentMedia());
    m_trackInfoModel->loadSettings();
    m_trackInfoModel->updateFileLabels(m_playbackEngine->currentMedia());
    m_playlistController->loadSettings();
    m_playlistWidget->processVisibleItems();
    m_actionManager->saveSettings();
}

void NPlayer::on_mainWindow_closed()
{
    if (m_settings->value("MinimizeToTray").toBool()) {
        m_systemTray->setVisible(true);
    } else {
#ifdef Q_OS_MAC
        if (m_settings->value("QuitOnClose").toBool()) {
            QCoreApplication::quit();
        }
#else
        QCoreApplication::quit();
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
    delete m_qmlEngine;
    saveSettings();
}

void NPlayer::on_playbackEngine_mediaChanged(const QString &file, int)
{
    QString title;
    QString format = NSettings::instance()->value("WindowTitleTrackInfo").toString();
    if (!format.isEmpty()) {
        m_trackInfoReader->setSource(file);
        title = m_trackInfoReader->toString(format);
    }
    m_mainWindow->setTitle(title);
    m_systemTray->setToolTip(title);

    m_waveformSlider->setMedia(file);
    m_trackInfoWidget->updateFileLabels(file);
    m_trackInfoModel->updateFileLabels(file);
    loadCoverArt(file);
}

void NPlayer::loadCoverArt(const QString &file)
{
    if (!m_settings->value("ShowCoverArt").toBool()) {
        return;
    }
    QImage image;
    if (m_coverReader) {
        m_coverReader->setSource(file);
        QList<QImage> images = m_coverReader->getImages();

        if (!images.isEmpty()) {
            image = images.first();
        }
    }

    if (image.isNull()) {
        QFileInfo fileInfo(file);
        QDir dir = fileInfo.absoluteDir();
        QStringList images = dir.entryList(QStringList() << "*.jpg"
                                                         << "*.jpeg"
                                                         << "*.png",
                                           QDir::Files);

        // search for image which file name starts same as source file:
        QString baseName = fileInfo.completeBaseName();
        QString imageFile;
        foreach (QString image, images) {
            if (baseName.startsWith(QFileInfo(image).completeBaseName())) {
                imageFile = dir.absolutePath() + "/" + image;
                break;
            }
        }

        // search for cover.* or folder.* or front.*:
        if (imageFile.isEmpty()) {
            QStringList matchedImages = images.filter(
                QRegExp("^(cover|folder|front)\\..*$", Qt::CaseInsensitive));
            if (!matchedImages.isEmpty()) {
                imageFile = dir.absolutePath() + "/" + matchedImages.first();
            }
        }
        if (!imageFile.isEmpty()) {
            image = QImage(imageFile);
        }
    }

    if (image.isNull()) {
        m_coverWidget->hide();
        if (m_coverImage) {
            m_coverImage->setVisible(false);
        }
    } else {
        m_coverWidget->show();
        m_coverWidget->setPixmap(QPixmap::fromImage(image));
        if (m_coverImage) {
            m_coverImage->setVisible(true);
            m_coverImage->setImage(image);
        }
    }
}

void NPlayer::on_playbackEngine_mediaFailed(const QString &, int)
{
    on_playbackEngine_mediaChanged("", 0);
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

void NPlayer::showTagEditor(const QString &path)
{
    NTagEditorDialog(path, m_mainWindow);
}

void NPlayer::on_playlist_addMoreRequested()
{
    if (!NSettings::instance()->value("LoadNext").toBool()) {
        return;
    }
    QDir::SortFlag flag = (QDir::SortFlag)NSettings::instance()->value("LoadNextSort").toInt();
    QString file = m_playlistWidget->playingItem()->data(N::PathRole).toString();
    QString path = QFileInfo(file).path();
    QStringList entryList =
        QDir(path).entryList(NSettings::instance()->value("FileFilters").toString().split(' '),
                             QDir::Files | QDir::NoDotAndDotDot, flag);
    int index = entryList.indexOf(QFileInfo(file).fileName());
    if (index != -1 && entryList.size() > index + 1) {
        m_playlistWidget->addFiles(QStringList() << path + "/" + entryList.at(index + 1));
    }
}

void NPlayer::playPause()
{
    if (!m_playlistWidget->hasPlayingItem()) {
        if (m_playlistWidget->count() > 0) {
            m_playlistWidget->playRow(0);
        } else {
            showOpenFileDialog();
        }
    } else {
        if (m_playbackEngine->state() == N::PlaybackPlaying) {
            m_playbackEngine->pause();
        } else {
            m_playbackEngine->play();
        }
    }
}

void NPlayer::showAboutDialog()
{
    NDialogHandler *dialogHandler = new NDialogHandler(QUrl::fromLocalFile(":src/aboutDialog.qml"),
                                                       m_qmlMainWindow);
    connect(dialogHandler, &NDialogHandler::beforeShown,
            [&](QQmlContext *context) { context->setContextProperty("NUtils", m_utils); });
    dialogHandler->showDialog();
}

void NPlayer::showPreferencesDialog()
{
    NPreferencesDialogHandler *dialogHandler = new NPreferencesDialogHandler(this, m_qmlMainWindow);
    connect(dialogHandler, &NDialogHandler::beforeShown, [&](QQmlContext *context) {
        context->setContextProperty("NUpdateChecker", &NUpdateChecker::instance());
    });

#ifndef _N_NO_UPDATE_CHECK_
    connect(dialogHandler, &NDialogHandler::afterShown,
            [](QObject *root) { root->setProperty("checkUpdate", true); });
#endif

    connect(dialogHandler, &NPreferencesDialogHandler::settingsApplied, this,
            &NPlayer::applySettings);
    dialogHandler->showDialog();
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
    m_playlistWidget->addItems(m_utils->dirListRecursive(dir));
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

void NPlayer::showContextMenu(const QPoint &pos)
{
    // (1, 1) offset to avoid accidental item activation
    m_actionManager->contextMenu()->exec(m_mainWindow->mapToGlobal(pos) + QPoint(1, 1));
}

void NPlayer::showPlaylistContextMenu(const QPoint &pos)
{
    m_actionManager->playlistContextMenu()->exec(pos);
}

void NPlayer::showToolTip(const QString &text)
{
    if (text.isEmpty()) {
        QToolTip::hideText();
        return;
    }

    QStringList offsetList = NSettings::instance()->value("TooltipOffset").toStringList();
    QToolTip::showText(mapToGlobal(QCursor::pos() +
                                   QPoint(offsetList.at(0).toInt(), offsetList.at(1).toInt())),
                       text);
}

bool NPlayer::revealInFileManager(const QString &file, QString *error) const
{
    QFileInfo fileInfo(file);

    if (!fileInfo.exists()) {
        *error = tr("File doesn't exist: <b>%1</b>").arg(QFileInfo(file).fileName());
        return false;
    }

    QString cmd;
    int res = 0;

    bool customFileManager = NSettings::instance()->value("CustomFileManager").toBool();
    if (customFileManager) {
        cmd = NSettings::instance()->value("CustomFileManagerCommand").toString();
        if (cmd.isEmpty()) {
            *error = tr("Custom File Manager is enabled but not configured.");
            return false;
        }
        QString filePath = file;
        QString fileName = fileInfo.fileName();
        QString canonicalPath = fileInfo.canonicalPath();
#if defined Q_OS_WIN
        filePath.replace('/', '\\');
        fileName.replace('/', '\\');
        canonicalPath.replace('/', '\\');
#else
        // escape single quote
        filePath.replace("'", "'\\''");
        fileName.replace("'", "'\\''");
        canonicalPath.replace("'", "'\\''");
#endif
        cmd.replace("%p", filePath);
        cmd.replace("%F", fileName);
        cmd.replace("%P", canonicalPath);

#if !defined Q_OS_WIN
        res = QProcess::execute("sh", QStringList{"-c", cmd});
#else
        res = QProcess::execute(cmd);
#endif
    } else {
        QString path = fileInfo.canonicalFilePath();
        QStringList args;
#if defined Q_OS_WIN
        cmd = "explorer.exe";
        args = QStringList{"/n", ",", "/select", ",", path.replace('/', '\\')};
#elif defined Q_OS_LINUX
        cmd = "xdg-open";
        args = QStringList{fileInfo.canonicalPath().replace("'", "'\\''")};
#elif defined Q_OS_MAC
        cmd = "open";
        args = QStringList{"-R", path.replace("'", "'\\''")};
#endif
        res = QProcess::execute(cmd, args);
        cmd += " " + args.join(' ');
    }

#ifndef Q_OS_WIN
    if (res != 0) {
        *error = tr("File manager command failed with exit code <b>%1</b>:").arg(res) +
                 QString("<br><br><span style=\"font-family: 'Lucida Console', Monaco, "
                         "monospace\">%1</span>")
                     .arg(cmd);
        return false;
    }
#endif

    return true;
}
