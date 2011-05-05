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

#include "rcDir.h"
#include <QCoreApplication>
#include <QStringList>
#include <QDir>

static bool _rcDir_init = FALSE;
static QString _rcDir;

QString rcDir()
{
	if (!_rcDir_init) {
#if !defined WIN32 && !defined _WINDOWS && !defined Q_WS_WIN
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

/* vim: set ts=4 sw=4: */
