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
#include <QStringList>
#include <QDir>
#include <QFileInfo>

namespace NCore
{
	static QList<QByteArray> _argList;
	static QVector<const char *> _argVector;
	static bool _cArgs_init = FALSE;

	static bool _rcDir_init = FALSE;
	static QString _rcDir = "./";

	static QStringList _processPath(const QString &path);
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
		_rcDir = QCoreApplication::applicationDirPath();
#endif
		QDir dir(_rcDir);
		if (!dir.exists())
			dir.mkdir(_rcDir);

		_rcDir_init = TRUE;
	}
	return _rcDir;
}


QStringList NCore::_processPath(const QString &path)
{
	QStringList list;
	if (QFileInfo(path).isDir()) {
		QStringList entryList = QDir(path).entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
		foreach (QString f, entryList)
			list << _processPath(path + "/" + f);
	} else {
		list << path;
	}
	return list;
}

QStringList NCore::dirListRecursive(const QString &path)
{
	return _processPath(path);
}

bool NCore::revealInFileManager(const QString &file)
{
	QFileInfo fileInfo(file);

	if (!fileInfo.exists())
		return FALSE;

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

