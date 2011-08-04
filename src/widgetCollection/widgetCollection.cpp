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
#include <QtPlugin>

static inline QString _domXml(const QString &className, const QString &name)
{
	return QString("<ui language=\"c++\">\n"
					" <widget class=\"%1\" name=\"%2\">\n"
					" </widget>\n"
					"</ui>").arg(className).arg(name);
}

NWidgetPlugin::NWidgetPlugin(const QString &className)
{
	m_initialized = FALSE;

	m_className = className;
	m_className.remove("Plugin");
	QString name = m_className;
	name.remove(0, 1);
	name[0] = name.at(0).toLower();

	m_domXml = _domXml(m_className, name);
	m_header = name + ".h";
}

void NWidgetPlugin::initialize(QDesignerFormEditorInterface *core)
{
	Q_UNUSED(core);

	if (m_initialized)
		return;

	m_initialized = TRUE;
}

NWidgetCollection::NWidgetCollection(QObject *parent) : QObject(parent)
{
	m_plugins.push_back(new NDropAreaPlugin(this));
	m_plugins.push_back(new NWaveformSliderPlugin(this));
	m_plugins.push_back(new NSliderPlugin(this));
	m_plugins.push_back(new NPlaylistWidgetPlugin(this));
	m_plugins.push_back(new NLabelPlugin(this));
}

Q_EXPORT_PLUGIN2(widget_collection, NWidgetCollection)

/* vim: set ts=4 sw=4: */
