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

#include "player.h"
#include "settings.h"
#include <qtsingleapplication.h>

#ifndef _N_NO_SKINS_
#include "skinFileSystem.h"
Q_IMPORT_PLUGIN(widget_collection)
#endif

int main(int argc, char *argv[])
{
	QtSingleApplication instance(argc, argv);

	// construct a message
	QString msg;
	if (QCoreApplication::arguments().size() > 1) {
		QStringList argList = QCoreApplication::arguments();
		argList.takeFirst();
		msg = argList.join("<|>");
	}

	if (NSettings::instance()->value("SingleInstance").toBool()) {
		// try to send it to an already running instrance
		if (instance.sendMessage(msg))
			return 0; // return if delivered
	}

	QApplication::setQuitOnLastWindowClosed(FALSE);
	QCoreApplication::setApplicationName("Nulloy");
	QCoreApplication::setApplicationVersion(QString(_N_VERSION_) + " Alpha");
	QCoreApplication::setOrganizationDomain("nulloy.com");

	// for Qt core plugins
#ifdef Q_WS_WIN
	QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() + "/Plugins/");
#endif

#ifndef _N_NO_SKINS_
	NSkinFileSystem::init();
#endif

	NPlayer p;
	QObject::connect(&instance, SIGNAL(messageReceived(const QString &)),
	                 &p, SLOT(readMessage(const QString &)));

	if (NSettings::instance()->value("RestorePlaylist").toBool()) {
		// try to load default playlist (will fail if msg contained files)
		p.loadDefaultPlaylist();
	}

	// manually read the message
	if (!msg.isEmpty())
		p.readMessage(msg);

	instance.installEventFilter(&p);

	return instance.exec();
}

