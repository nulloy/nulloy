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

#include "arguments.h"
#include <QtCore>

static QList<QByteArray> _argList;
static QVector<const char *> _argVector;
static bool _init = FALSE;

void c_args(int *argc, const char ***argv)
{
	if (!_init) {
		foreach (const QString &s, QCoreApplication::arguments()) {
			_argList << s.toLatin1();
			_argVector << _argList.last().constData();
		}
		_init = TRUE;
	}

	*argv = &_argVector[0];
	*argc = QCoreApplication::arguments().size();
}

QString applicationBinaryName()
{
	return QFileInfo(QCoreApplication::arguments().first()).completeBaseName();
}

/* vim: set ts=4 sw=4: */

