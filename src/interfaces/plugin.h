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

#ifndef N_PLUGIN_H
#define N_PLUGIN_H

#include "global.h"
#include <QtCore>

class NPlugin
{
protected:
	bool m_init;

public:
	NPlugin() { m_init = FALSE; }
	virtual ~NPlugin() {}
	virtual QString name()
	{
		QObject *obj = dynamic_cast<QObject *>(this);
		if (obj)
			return obj->metaObject()->className();
		else
			return "";
	}
	virtual QString interfaceString() = 0;
	virtual N::PluginType type() { return N::OtherPlugin; }
	virtual void init() = 0;
};

Q_DECLARE_INTERFACE(NPlugin, "Nulloy/NPlugin/0.7")

#endif

