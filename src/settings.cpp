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

#include "settings.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QVariant>

#include "action.h"
#include "common.h"

#define MIN_VERSION "0.8"

NSettings *NSettings::m_instance = NULL;

NSettings::NSettings(QObject *parent)
    : QSettings(NCore::settingsPath(), QSettings::IniFormat, parent)
{
    Q_ASSERT_X(!m_instance, "NSettings", "NSettings instance already exists.");
    m_instance = this;

    QString version = value("SettingsVersion").toString();
    if (version.isEmpty() || version < MIN_VERSION) {
        foreach (QString key, allKeys())
            remove(key);
        setValue("SettingsVersion", MIN_VERSION);
    }

    initValue("Shortcuts/PlayAction", QStringList() << "X"
                                                    << "C"
                                                    << "Space");
    initValue("Shortcuts/StopAction", "V");
    initValue("Shortcuts/PrevAction", "Z");
    initValue("Shortcuts/NextAction", "B");

    initValue("Shortcuts/Jump1ForwardAction", "Shift+Right");
    initValue("Shortcuts/Jump1BackwardsAction", "Shift+Left");
    initValue("Shortcuts/Jump2ForwardAction", "Right");
    initValue("Shortcuts/Jump2BackwardsAction", "Left");
    initValue("Shortcuts/Jump3ForwardAction", "Ctrl+Right");
    initValue("Shortcuts/Jump3BackwardsAction", "Ctrl+Left");
    initValue("Jump1", 5);
    initValue("Jump2", 30);
    initValue("Jump3", 180);

    {
        QStringList fullScreenKeys;
        foreach (QKeySequence seq, QKeySequence::keyBindings(QKeySequence::FullScreen)) {
            fullScreenKeys << seq.toString();
        }
        initValue("Shortcuts/FullScreenAction", fullScreenKeys);
    }

    initValue("PlaylistTrackInfo", "%F (%d)");
    initValue("WindowTitleTrackInfo",
              "\"{%a - %t|%F}\" - " + QCoreApplication::applicationName() + " %v");
    initValue("EncodingTrackInfo", "UTF-8");
    initValue("TooltipTrackInfo", "%C");
    initValue("TooltipOffset", QStringList() << QString::number(0) << QString::number(0));

    initValue("Shuffle", false);
    initValue("Repeat", false);

    initValue("Maximized", false);
    initValue("TrayIcon", false);
    initValue("AlwaysOnTop", false);
    initValue("WhilePlayingOnTop", false);
    initValue("StartPaused", false);
    initValue("RestorePlaylist", true);
    initValue("SingleInstance", true);
    initValue("EnqueueFiles", true);
    initValue("PlayEnqueued", true);
    initValue("AutoCheckUpdates", true);
    initValue("DisplayLogDialog", true);
    initValue("LastDirectory", QStandardPaths::standardLocations(QStandardPaths::MusicLocation));
    initValue("LoopPlaylist", false);
    initValue("LoadNext", false);
    initValue("ShowCoverArt", true);
    initValue("ShowPlaybackControls", true);
    initValue("LoadNextSort", QDir::Name);
    initValue("Volume", 0.8);
    initValue("ShowDecibelsVolume", false);

#ifdef Q_OS_WIN
    initValue("TaskbarProgress", true);
#endif

#ifdef Q_OS_MAC
    initValue("QuitOnClose", false);
#else
    initValue("MinimizeToTray", false);
#endif

    initValue("CustomTrash", false);
    initValue("CustomFileManager", false);
    initValue("FileFilters", QString("*.m3u *.m3u8 \
        *.mp3 *.ogg *.mp4 *.wma \
        *.flac *.ape *.wav *.wv *.tta \
        *.mpc *.spx *.opus \
        *.m4a *.aac *.aiff \
        *.xm *.s3m *.it *.mod")
                                 .simplified());

    initValue("TrackInfo/TopLeft", "{%B kbps/|}{%s kHz|}");
    initValue("TrackInfo/MiddleCenter", "{%a - %t|%F}");
    initValue("TrackInfo/BottomRight", "%T/%d");
}

NSettings::~NSettings()
{
    m_instance = NULL;
}

NSettings *NSettings::instance()
{
    if (!m_instance) {
        m_instance = new NSettings();
    }

    return m_instance;
}

void NSettings::initShortcuts(QObject *instance)
{
    foreach (NAction *action, instance->findChildren<NAction *>()) {
        if (action->isCustomizable()) {
            m_actionList << action;
        }
    }
}

void NSettings::loadShortcuts()
{
    foreach (NAction *action, m_actionList) {
        QList<QKeySequence> localShortcuts;
        QStringList localsList = value("Shortcuts/" + action->objectName()).toStringList();
        if (!localsList.isEmpty()) {
            foreach (QString str, localsList)
                localShortcuts << QKeySequence(str);
            action->setShortcuts(localShortcuts);
        }

        QList<QKeySequence> globalShortcuts;
        QStringList globalsList = value("GlobalShortcuts/" + action->objectName()).toStringList();
        if (!globalsList.isEmpty()) {
            foreach (QString str, globalsList)
                globalShortcuts << QKeySequence(str);
            action->setGlobalShortcuts(globalShortcuts);
        }
    }
}

void NSettings::saveShortcuts()
{
    foreach (NAction *action, m_actionList) {
        if (action->objectName().isEmpty() || !action->isCustomizable()) {
            continue;
        }

        QList<QKeySequence> localKeys = action->shortcuts();
        if (!localKeys.isEmpty()) {
            QStringList localsList;
            foreach (QKeySequence seq, localKeys) {
                if (!seq.isEmpty()) {
                    localsList << seq.toString();
                }
            }
            setValue("Shortcuts/" + action->objectName(), localsList);
        } else {
            remove("Shortcuts/" + action->objectName());
        }

        QList<QKeySequence> globalKeys = action->globalShortcuts();
        if (!globalKeys.isEmpty()) {
            QStringList globalsList;
            foreach (QKeySequence seq, globalKeys) {
                if (!seq.isEmpty()) {
                    globalsList << seq.toString();
                }
            }
            setValue("GlobalShortcuts/" + action->objectName(), globalsList);
        } else {
            remove("GlobalShortcuts/" + action->objectName());
        }
    }
}

QList<NAction *> NSettings::shortcuts() const
{
    return m_actionList;
}

QVariant NSettings::value(const QString &key, const QVariant &defaultValue) const
{
    QVariant value = QSettings::value(key, defaultValue);
    return value;
}

void NSettings::setValue(const QString &key, const QVariant &value)
{
    QSettings::setValue(key, value);
    emit valueChanged(key, value);
}

void NSettings::initValue(const QString &key, const QVariant &defaultValue)
{
    setValue(key, value(key, defaultValue));
}

void NSettings::remove(const QString &key)
{
    QSettings::remove(key);
    emit valueChanged(key, QString());
}
