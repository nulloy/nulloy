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

#include "dirProcessor.h"
#include <QFileInfo>
#include <QDir>

#include <QDebug>

static QStringList _processPath(QString path)
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

QStringList dirListRecursive(QString path)
{
	return _processPath(path);
}

/* vim: set ts=4 sw=4: */
