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

#ifndef N_PLUGIN_TAGLIB_H
#define N_PLUGIN_TAGLIB_H

#include "pluginContainer.h"

class NPluginTaglib : public QObject, public NPluginContainer
{
	Q_OBJECT
	Q_INTERFACES(NPluginContainer)

private:
	QObjectList m_plugins;

public:
	NPluginTaglib(QObject *parent = NULL);
	~NPluginTaglib();
	QObjectList plugins();
	QString name() { return "TagLib"; }
	QString version() { return "0.5"; }
};

#endif

