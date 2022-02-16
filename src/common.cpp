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

#include "common.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QtCore>

#ifdef Q_OS_WIN
#include <QSettings>
#endif

namespace NCore
{
    static QList<QByteArray> _argList;
    static QVector<const char *> _argVector;
    static bool _cArgs_init = false;

    static bool _rcDir_init = false;
    static QString _rcDir = "./";
} // namespace NCore

void NCore::cArgs(int *argc, const char ***argv)
{
    if (!_cArgs_init) {
        foreach (const QString &s, QCoreApplication::arguments()) {
            _argList << s.toLatin1();
            _argVector << _argList.last().constData();
        }
        _cArgs_init = true;
    }

    *argv = &_argVector[0];
    *argc = QCoreApplication::arguments().size();
}

QString NCore::applicationBinaryName()
{
    return QFileInfo(QCoreApplication::arguments().first()).completeBaseName();
}

QString NCore::applicationBasenameName()
{
    return QFileInfo(QCoreApplication::arguments().first()).fileName();
}

QString NCore::defaultPlaylistPath()
{
    return NCore::rcDir() + "/" + NCore::applicationBinaryName() + ".m3u";
}

QString NCore::settingsPath()
{
    return NCore::rcDir() + "/" + NCore::applicationBinaryName() + ".cfg";
}

QString NCore::rcDir()
{
    if (!_rcDir_init) {
#ifndef Q_OS_WIN
        QDir parentDir(QCoreApplication::applicationDirPath());
        if (parentDir.dirName() == "bin") {
            _rcDir = QDir::homePath() + "/.nulloy";
        } else {
            _rcDir = QCoreApplication::applicationDirPath();
        }
#else
        QDir parentDir(QCoreApplication::applicationDirPath());
        parentDir.cdUp();

        QSettings
            registryMachine("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion",
                            QSettings::NativeFormat);
        QDir programFilesDir(
            registryMachine.value("ProgramFilesDir", "C:/Program Files").toString());
        QDir programFilesDirX86(
            registryMachine.value("ProgramFilesDir (x86)", "C:/Program Files (x86)").toString());

        if (parentDir == programFilesDir || parentDir == programFilesDirX86) {
            QString appData = QProcessEnvironment::systemEnvironment().value("AppData");
            if (appData != "")
                _rcDir = appData + "/Nulloy";
            else
                _rcDir = QCoreApplication::applicationDirPath();
        } else {
            _rcDir = QCoreApplication::applicationDirPath();
        }
#endif
        QDir dir(_rcDir);
        if (!dir.exists()) {
            dir.mkdir(_rcDir);
        }

        _rcDir_init = true;
    }
    return _rcDir;
}
