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

#include "common.h"

#include <QtCore>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

#ifdef Q_WS_WIN
#include <QSettings>
#endif

namespace NCore
{
	static QList<QByteArray> _argList;
	static QVector<const char *> _argVector;
	static bool _cArgs_init = FALSE;

	static bool _rcDir_init = FALSE;
	static QString _rcDir = "./";

	static QStringList _processPath(const QString &path, const QStringList &nameFilters);
}

void NCore::cArgs(int *argc, const char ***argv)
{
	if (!_cArgs_init) {
		foreach (const QString &s, QCoreApplication::arguments()) {
			_argList << s.toLatin1();
			_argVector << _argList.last().constData();
		}
		_cArgs_init = TRUE;
	}

	*argv = &_argVector[0];
	*argc = QCoreApplication::arguments().size();
}

QString NCore::applicationBinaryName()
{
	return QFileInfo(QCoreApplication::arguments().first()).completeBaseName();
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
#ifndef Q_WS_WIN
		QDir parentDir(QCoreApplication::applicationDirPath());
		if (parentDir.dirName() == "bin")
			_rcDir = QDir::homePath() + "/.nulloy";
		else
			_rcDir = QCoreApplication::applicationDirPath();
#else
		QDir parentDir(QCoreApplication::applicationDirPath());
		parentDir.cdUp();

		QSettings registryMachine("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion", QSettings::NativeFormat);
		QDir programFilesDir(registryMachine.value("ProgramFilesDir", "C:/Program Files").toString());
		QDir programFilesDirX86(registryMachine.value("ProgramFilesDir (x86)", "C:/Program Files (x86)").toString());

		if (parentDir == programFilesDir || parentDir == programFilesDirX86) {
			QString appData = QProcessEnvironment::systemEnvironment ().value("AppData");
			if (appData != "")
				_rcDir = appData + "/Nulloy";
			else
				_rcDir = QCoreApplication::applicationDirPath();
		} else {
			_rcDir = QCoreApplication::applicationDirPath();
		}
#endif
		QDir dir(_rcDir);
		if (!dir.exists())
			dir.mkdir(_rcDir);

		_rcDir_init = TRUE;
	}
	return _rcDir;
}


QStringList NCore::_processPath(const QString &path, const QStringList &nameFilters)
{
	QStringList list;
	if (QFileInfo(path).isDir()) {
		QStringList entryList;
		if (!nameFilters.isEmpty())
			entryList = QDir(path).entryList(nameFilters, QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
		else
			entryList = QDir(path).entryList(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);

		foreach (QString f, entryList)
			list << _processPath(path + "/" + f, nameFilters);
	} else {
		list << path;
	}
	return list;
}

QStringList NCore::dirListRecursive(const QString &path, const QStringList &nameFilters)
{
	return _processPath(path, nameFilters);
}

bool NCore::revealInFileManager(const QString &file, QString *error)
{
	QFileInfo fileInfo(file);

	if (!fileInfo.exists()) {
		*error = QString(QObject::tr("File doesn't exist: <b>%1</b>")).arg(QFileInfo(file).fileName());
		return FALSE;
	}

	QString fileManagerCommand;
	QStringList arguments;
	QString path = fileInfo.canonicalFilePath();
#if defined Q_WS_WIN
	fileManagerCommand = "explorer.exe";
	arguments << "/n,/select,";
	path = path.replace('/', '\\');
#elif defined Q_WS_X11
	QProcess xdg;
	xdg.start("xdg-mime query default inode/directory");
	xdg.waitForStarted();
	xdg.waitForFinished();

	fileManagerCommand = QString::fromUtf8(xdg.readAll()).simplified().remove(".desktop");
	if (QProcess::execute("which " + fileManagerCommand) != 0) {
		*error = QString(QObject::tr("Default file manager is set to <b>%1</b> but it's not available.")).arg(fileManagerCommand);
		return FALSE;
	}

	if (fileManagerCommand == "dolphin") {
		arguments << "--select";
	} else if (fileManagerCommand != "nautilus") {
		path = fileInfo.canonicalPath();
	}
#elif defined Q_WS_MAC
	fileManagerCommand = "open";
	arguments << "-R";
#endif

	arguments << path;

	QProcess reveal;
	reveal.start(fileManagerCommand, arguments);
	reveal.waitForStarted();
	reveal.waitForFinished();

	return TRUE;
}

