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

#ifndef N_PLUGIN_INTERFACE_H
#define N_PLUGIN_INTERFACE_H

#include <QtCore>

class NPluginInterface
{
protected:
	bool m_init;

public:
	NPluginInterface() { m_init = FALSE; }
	virtual ~NPluginInterface() {}
	virtual QString identifier() = 0;
	virtual QString interface() = 0;
	virtual void init() = 0;
};

Q_DECLARE_INTERFACE(NPluginInterface, "Nulloy/PluginInterface/0.1")

#endif
