/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2015 Sergey Vlasov <sergey@vlasov.me>
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

#include "settings.h"
#include <QProcess>

int _trash(const QString &path, QString *error)
{
	bool customTrash = NSettings::instance()->value("CustomTrash").toBool();
	if (!customTrash) {
		*error = QString(QObject::tr("Custom Trash Command is not configured."));
		return -1;
	}

	QString cmd = NSettings::instance()->value("CustomTrashCommand").toString();
	if (cmd.isEmpty()) {
		*error = QString(QObject::tr("Custom Trash Command is enabled but not configured."));
		return -1;
	}

	cmd.replace("%f", path);

	int res = QProcess::execute(cmd);
	if (res != 0) {
		*error = QString(QObject::tr("Custom Trash Command failed with exit code <b>%1</b>.")).arg(res);
		return -1;
	}

	return 0;
}

