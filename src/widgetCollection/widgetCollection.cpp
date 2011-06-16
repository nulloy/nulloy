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

#include "widgetCollection.h"
#include "dropAreaPlugin.h"
#include "waveformSliderPlugin.h"
#include "sliderPlugin.h"
#include "playlistWidgetPlugin.h"
#include "labelPlugin.h"
#include <QtPlugin>

NWidgetCollection::NWidgetCollection(QObject *parent) : QObject(parent)
{
	const QString group = QLatin1String("Nulloy");
	m_plugins.push_back(new NDropAreaPlugin(group, this));
	m_plugins.push_back(new NWaveformSliderPlugin(group, this));
	m_plugins.push_back(new NSliderPlugin(group, this));
	m_plugins.push_back(new NPlaylistWidgetPlugin(group, this));
	m_plugins.push_back(new NLabelPlugin(group, this));
}

QList<QDesignerCustomWidgetInterface *> NWidgetCollection::customWidgets() const
{
	return m_plugins;
}

Q_EXPORT_PLUGIN2(widget_collection, NWidgetCollection)

/* vim: set ts=4 sw=4: */
