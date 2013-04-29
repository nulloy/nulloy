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

#ifndef N_PLUGIN_GSTREAMER_H
#define N_PLUGIN_GSTREAMER_H

#include "pluginInterface.h"

class NPluginGstreamer : public QObject, public NPluginInterface
{
	Q_OBJECT
	Q_INTERFACES(NPluginInterface)

private:
	QObjectList m_elements;

public:
	NPluginGstreamer(QObject *parent = NULL);
	~NPluginGstreamer();
	QObjectList elements();
	QString name() { return "GStreamer"; }
	QString version() { return "0.5"; }
};

#endif
