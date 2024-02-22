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

#include "settings.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QStyle>
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

    setIniCodec("UTF-8");

    QString version = value("SettingsVersion").toString();
    if (version.isEmpty() || version < MIN_VERSION) {
        foreach (QString key, allKeys())
            remove(key);
        setValue("SettingsVersion", MIN_VERSION);
    }

    initValue("Style", "Fusion");

    initValue("Shortcuts/RevealInFileManagerAction", QStringList() << "Ctrl+Return");
    initValue("Shortcuts/RemoveFromPlaylistAction", QStringList() << "Delete");
    initValue("Shortcuts/MoveToTrashAction", QStringList() << "Ctrl+Delete");
    initValue("Shortcuts/TagEditorAction", QStringList() << "F4");
    initValue("Shortcuts/PrevAction", QStringList() << "Z");
    initValue("Shortcuts/PlayAction", QStringList() << "X");
    initValue("Shortcuts/PauseAction", QStringList() << "C");
    initValue("Shortcuts/StopAction", QStringList() << "V");
    initValue("Shortcuts/NextAction", QStringList() << "B");
    initValue("Shortcuts/PlayPauseAction", QStringList() << "Space");

    initValue("Shortcuts/Jump1ForwardAction", QStringList() << "Shift+Right");
    initValue("Shortcuts/Jump1BackwardsAction", QStringList() << "Shift+Left");
    initValue("Shortcuts/Jump2ForwardAction", QStringList() << "Right");
    initValue("Shortcuts/Jump2BackwardsAction", QStringList() << "Left");
    initValue("Shortcuts/Jump3ForwardAction", QStringList() << "Ctrl+Right");
    initValue("Shortcuts/Jump3BackwardsAction", QStringList() << "Ctrl+Left");
    initValue("Jump1", 5);
    initValue("Jump2", 30);
    initValue("Jump3", 180);

    initValue("Shortcuts/SpeedIncreaseAction", QStringList() << "]");
    initValue("Shortcuts/SpeedDecreaseAction", QStringList() << "[");
    initValue("Shortcuts/SpeedResetAction", QStringList() << "Backspace");
    initValue("SpeedStep", 0.01);

    /*
    initValue("Shortcuts/PitchIncreaseAction", QStringList() << "}");
    initValue("Shortcuts/PitchDecreaseAction", QStringList() << "{");
    initValue("Shortcuts/PitchResetAction", QStringList() << "Shift+Backspace");
    initValue("PitchStep", 0.01);
    */

    {
        QStringList fullScreenKeys;
        foreach (QKeySequence seq, QKeySequence::keyBindings(QKeySequence::FullScreen)) {
            fullScreenKeys << seq.toString();
        }
        initValue("Shortcuts/FullScreenAction", QStringList() << fullScreenKeys);
    }

    initValue("PlaylistTrackInfo", "%F{ (%d)}");
    initValue("WindowTitleTrackInfo",
              "{\"%a - %t\" — |\"%F\" — }" + QCoreApplication::applicationName() + " %v");
    initValue("EncodingTrackInfo", "UTF-8");
    initValue("TooltipTrackInfo", "%C");
    initValue("TooltipOffset", QStringList() << QString::number(0) << QString::number(0));

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
    initValue("DisplayTagEditorConfirmDialog", true);
    initValue("DisplayMoveToTrashConfirmDialog", true);
    initValue("LastDirectory", QStandardPaths::standardLocations(QStandardPaths::MusicLocation));
    initValue("ScrollToItem", true);
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

    initValue("QuitWhenFinished", false);

    initValue("CustomTrash", false);
    initValue("CustomTrashCommand", "");
    initValue("CustomFileManager", false);
    initValue("CustomFileManagerCommand", "");
    initValue("FileFilters", QString("*.m3u *.m3u8 \
        *.mp3 *.ogg *.mp4 *.wma \
        *.flac *.ape *.wav *.wv *.tta \
        *.mpc *.spx *.opus \
        *.m4a *.aac *.aiff \
        *.xm *.s3m *.it *.mod")
                                 .simplified());

    initValue("TrackInfo/TopLeft", "{%B kbps/%s kHz|{%B kbps}{%s kHz}}");
    initValue("TrackInfo/MiddleCenter", "{%a - %t|%F}");
    initValue("TrackInfo/BottomRight", "%T{/%d}");
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
    QVariant val = value(key, defaultValue);
    val.convert(defaultValue.type());
    setValue(key, val);
}

void NSettings::remove(const QString &key)
{
    QSettings::remove(key);
    emit valueChanged(key, QString());
}

NSettingsOverlay::NSettingsOverlay(QObject *parent) : QObject(parent)
{
    for (const QString &key : NSettings::instance()->allKeys()) {
        m_map[key] = NSettings::instance()->value(key);
    }
}

QVariant NSettingsOverlay::value(const QString &key) const
{
    if (!m_map.contains(key)) {
        return "";
    }

    QVariant value = m_map.value(key);
    if (QString(value.typeName()) == "QString" &&
        (value.toString() == "false" || value.toString() == "true")) {
        return QVariant(value.toBool());
    }

    return value;
}

void NSettingsOverlay::setValue(const QString &key, const QVariant &value)
{
    m_map[key] = value;
}

void NSettingsOverlay::commit()
{
    QMapIterator<QString, QVariant> iter(m_map);
    while (iter.hasNext()) {
        iter.next();
        NSettings::instance()->setValue(iter.key(), iter.value());
    }
}
