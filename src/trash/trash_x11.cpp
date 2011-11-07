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

#include  "trash.h"
#include <QProcess>

int NTrash(const QString &path, QString *error)
{
	if (QProcess::execute("which trash") != 0) {
		*error = "'trash-cli' is not available on your system.";
		return -1;
	}

	QProcess trash;
	trash.start("trash \"" +  path + "\"");
	trash.waitForStarted();
	trash.waitForFinished();
	if ( trash.readAll().startsWith("trash: cannot trash"))
		return -1;
	else
		return 0;
}

/* vim: set ts=4 sw=4: */

