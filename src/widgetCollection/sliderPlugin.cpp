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

#include "slider.h"
#include "sliderPlugin.h"
#include <QtPlugin>

NSliderPlugin::NSliderPlugin(const QString &group, QObject *parent) : QObject(parent)
{
	m_initialized = FALSE;
	m_group = group;
}

void NSliderPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
	if (m_initialized)
		return;

	m_initialized = TRUE;
}

bool NSliderPlugin::isInitialized() const
{
	return m_initialized;
}

QWidget *NSliderPlugin::createWidget(QWidget *parent)
{
	return new NSlider(parent);
}

QString NSliderPlugin::name() const
{
	return "NSlider";
}

QString NSliderPlugin::group() const
{
	return m_group;
}

QIcon NSliderPlugin::icon() const
{
	return QIcon();
}

QString NSliderPlugin::toolTip() const
{
	return "";
}

QString NSliderPlugin::whatsThis() const
{
	return "";
}

bool NSliderPlugin::isContainer() const
{
	return FALSE;
}

QString NSliderPlugin::domXml() const
{
	return "<ui language=\"c++\">\n"
		" <widget class=\"NSlider\" name=\"slider\">\n"
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

QString NSliderPlugin::includeFile() const
{
	return "sliderPlugin.h";
}

/* vim: set ts=4 sw=4: */
