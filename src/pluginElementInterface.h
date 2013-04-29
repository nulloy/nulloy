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

#ifndef N_PLUGIN_ELEMENT_INTERFACE_H
#define N_PLUGIN_ELEMENT_INTERFACE_H

#include <QtCore>

enum PluginType {
	Other = 0x0,
	PlaybackEngine = 0x1,
	WaveformBuilder = 0x2,
	TagReader = 0x3
};
Q_DECLARE_FLAGS(PluginFlags, PluginType)
Q_DECLARE_OPERATORS_FOR_FLAGS(PluginFlags)

class NPluginElementInterface
{
protected:
	bool m_init;

public:
	NPluginElementInterface() { m_init = FALSE; }
	virtual ~NPluginElementInterface() {}
	virtual QString name()
	{	QObject *obj = dynamic_cast<QObject *>(this);
		if (obj)
			return obj->metaObject()->className();
		else
			return "";
	}
	virtual QString interface() = 0;
	virtual PluginType type() { return Other; }
	virtual void init() = 0;
};

Q_DECLARE_INTERFACE(NPluginElementInterface, "Nulloy/NPluginElementInterface/0.5")

#endif
