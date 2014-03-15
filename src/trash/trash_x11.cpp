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

#include <QProcess>

int _trash(const QString &path, QString *error)
{
	QString cmd = "trash";
	int res;

	res = QProcess::execute("which " + cmd);
	if (res != 0) {
		cmd = "trash-put";
		res = QProcess::execute("which " + cmd);
	}
	if (res != 0) {
		*error = "Package <b>trash-cli</b> is not installed.";
		return -1;
	}

	QProcess trash;
	trash.start(cmd + " \"" +  path + "\"");
	trash.waitForStarted();
	trash.waitForFinished();
	if (trash.readAll().startsWith("trash: cannot trash"))
		return -1;
	else
		return 0;
}

