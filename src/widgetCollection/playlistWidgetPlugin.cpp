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

#include "playlistWidget.h"
#include "playlistWidgetPlugin.h"
#include <QtPlugin>

NPlaylistWidgetPlugin::NPlaylistWidgetPlugin(const QString &group, QObject *parent) : QObject(parent)
{
	m_initialized = FALSE;
	m_group = group;
}

void NPlaylistWidgetPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
	if (m_initialized)
		return;

	m_initialized = TRUE;
}

bool NPlaylistWidgetPlugin::isInitialized() const
{
	return m_initialized;
}

QWidget *NPlaylistWidgetPlugin::createWidget(QWidget *parent)
{
	return new NPlaylistWidget(parent);
}

QString NPlaylistWidgetPlugin::name() const
{
	return "NPlaylistWidget";
}

QString NPlaylistWidgetPlugin::group() const
{
	return m_group;
}

QIcon NPlaylistWidgetPlugin::icon() const
{
	return QIcon();
}

QString NPlaylistWidgetPlugin::toolTip() const
{
	return "";
}

QString NPlaylistWidgetPlugin::whatsThis() const
{
	return "";
}

bool NPlaylistWidgetPlugin::isContainer() const
{
	return FALSE;
}

QString NPlaylistWidgetPlugin::domXml() const
{
	return "<ui language=\"c++\">\n"
		" <widget class=\"NPlaylistWidget\" name=\"playlistWidget\">\n"
		"  <property name=\"geometry\">\n"
		"   <rect>\n"
		"    <x>0</x>\n"
		"    <y>0</y>\n"
		"    <width>200</width>\n"
		"    <height>300</height>\n"
		"   </rect>\n"
		"  </property>\n"
		" </widget>\n"
		"</ui>";
}

QString NPlaylistWidgetPlugin::includeFile() const
{
	return "playlistWidget.h";
}

/* vim: set ts=4 sw=4: */
