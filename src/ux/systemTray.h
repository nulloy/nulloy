/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2013 Sergey Vlasov <sergey@vlasov.me>
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

#ifndef N_SYSTEM_TRAY_H
#define N_SYSTEM_TRAY_H

#include <QSystemTrayIcon>

namespace NSystemTray
{
	void init(QObject *parent = 0);

	void setContextMenu(QMenu *menu);
	void setEnabled(bool enabled);
	void setIcon(const QIcon &icon);
	void setToolTip(const QString &str);
}

#endif

/* vim: set ts=4 sw=4: */
