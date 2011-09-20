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

#include "core.h"

#include <QtCore>
#include <QCoreApplication>
#include <QStringList>
#include <QDir>
#include <QFileInfo>

namespace NCore
{
	QList<QByteArray> _argList;
	QVector<const char *> _argVector;
	bool _cArgs_init = FALSE;

	bool _rcDir_init = FALSE;
	QString _rcDir;

	QStringList _processPath(const QString &path);
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

/* vim: set ts=4 sw=4: */

