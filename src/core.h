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

#ifndef N_CORE_H
#define N_CORE_H

#include <QStringList>

namespace NCore
{
	void cArgs(int *argc, const char ***argv);
	QString applicationBinaryName();
	QString rcDir();
	QStringList dirListRecursive(const QString &path);
}

#endif

/* vim: set ts=4 sw=4: */
