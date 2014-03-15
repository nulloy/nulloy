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

#include "systemTray.h"

namespace NSystemTray
{
	QSystemTrayIcon *_trayIcon = NULL;
}

void NSystemTray::init(QObject *parent)
{
	_trayIcon = new QSystemTrayIcon(parent);
	_trayIcon->setObjectName("trayIcon");
}

void NSystemTray::setContextMenu(QMenu *menu)
{
	_trayIcon->setContextMenu(menu);
}

void NSystemTray::setEnabled(bool enabled)
{
	_trayIcon->setVisible(enabled);
}

void NSystemTray::setIcon(const QIcon &icon)
{
	_trayIcon->setIcon(icon);
}

void NSystemTray::setToolTip(const QString &str)
{
	_trayIcon->setToolTip(str);
}

