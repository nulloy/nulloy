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

#include "player.h"
#include <qtsingleapplication.h>

#ifndef _N_NO_SKINS_
#include "skinFileSystem.h"
Q_IMPORT_PLUGIN(widget_collection)
#endif

int main(int argc, char *argv[])
{
	QtSingleApplication instance(argc, argv);

	QString msg;
	if (QCoreApplication::arguments().size() > 1) {
		QStringList argList = QCoreApplication::arguments();
		argList.takeFirst();
		QStringList pathList;
		foreach (QString arg, argList) {
			if (QFile(arg).exists())
				pathList << arg;
		}
		msg = "files:" + pathList.join("<|>");
	}
	if (instance.sendMessage(msg))
		return 0;

	QApplication::setQuitOnLastWindowClosed(FALSE);

	QCoreApplication::setApplicationName("Nulloy");
	QCoreApplication::setApplicationVersion(QString(_N_VERSION_) + " Alpha");
	QCoreApplication::setOrganizationDomain("nulloy.com");

#if !defined WIN32 && !defined _WINDOWS && !defined Q_WS_WIN
	int res;
	res = chdir(QCoreApplication::applicationDirPath().toAscii().data());
#endif

#ifndef _N_NO_SKINS_
	NSkinFileSystem::init();
#endif

	NPlayer p;
	QObject::connect(&instance, SIGNAL(messageReceived(const QString &)),
					&p, SLOT(message(const QString &)));

	return instance.exec();
}

/* vim: set ts=4 sw=4: */
