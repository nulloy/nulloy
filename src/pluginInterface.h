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

#ifndef N_PLUGIN_INTERFACE_H
#define N_PLUGIN_INTERFACE_H

#include <QtCore>

class NPluginInterface
{
public:
	NPluginInterface() {}
	virtual ~NPluginInterface() {}
	virtual QObjectList elements() = 0;
	virtual QString name() = 0;
	virtual QString version() = 0;
};

Q_DECLARE_INTERFACE(NPluginInterface, "Nulloy/NPluginInterface/0.5")

#endif
