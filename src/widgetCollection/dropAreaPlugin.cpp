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

#include "dropArea.h"
#include "dropAreaPlugin.h"
#include <QtPlugin>

NDropAreaPlugin::NDropAreaPlugin(const QString &group, QObject *parent) : QObject(parent)
{
	m_initialized = FALSE;
	m_group = group;
}

void NDropAreaPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
	if (m_initialized)
		return;

	m_initialized = TRUE;
}

bool NDropAreaPlugin::isInitialized() const
{
	return m_initialized;
}

QWidget *NDropAreaPlugin::createWidget(QWidget *parent)
{
	return new NDropArea(parent);
}

QString NDropAreaPlugin::name() const
{
	return "NDropArea";
}

QString NDropAreaPlugin::group() const
{
	return m_group;
}

QIcon NDropAreaPlugin::icon() const
{
	return QIcon();
}

QString NDropAreaPlugin::toolTip() const
{
	return "";
}

QString NDropAreaPlugin::whatsThis() const
{
	return "";
}

bool NDropAreaPlugin::isContainer() const
{
	return TRUE;
}

QString NDropAreaPlugin::domXml() const
{
	return "<ui language=\"c++\">\n"
		" <widget class=\"NDropArea\" name=\"dropArea\">\n"
		"  <property name=\"geometry\">\n"
		"   <rect>\n"
		"    <x>0</x>\n"
		"    <y>0</y>\n"
		"    <width>100</width>\n"
		"    <height>100</height>\n"
		"   </rect>\n"
		"  </property>\n"
		" </widget>\n"
		"</ui>";
}

QString NDropAreaPlugin::includeFile() const
{
	return "dropAreaPlugin.h";
}

/* vim: set ts=4 sw=4: */
