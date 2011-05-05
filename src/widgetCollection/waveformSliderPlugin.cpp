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

#include "waveformSlider.h"
#include "waveformSliderPlugin.h"
#include <QtPlugin>

NWaveformSliderPlugin::NWaveformSliderPlugin(const QString &group, QObject *parent) : QObject(parent)
{
	m_initialized = FALSE;
	m_group = group;
}

void NWaveformSliderPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
	if (m_initialized)
		return;

	m_initialized = TRUE;
}

bool NWaveformSliderPlugin::isInitialized() const
{
	return m_initialized;
}

QWidget *NWaveformSliderPlugin::createWidget(QWidget *parent)
{
	return new NWaveformSlider(parent);
}

QString NWaveformSliderPlugin::name() const
{
	return "NWaveformSlider";
}

QString NWaveformSliderPlugin::group() const
{
	return m_group;
}

QIcon NWaveformSliderPlugin::icon() const
{
	return QIcon();
}

QString NWaveformSliderPlugin::toolTip() const
{
	return "";
}

QString NWaveformSliderPlugin::whatsThis() const
{
	return "";
}

bool NWaveformSliderPlugin::isContainer() const
{
	return FALSE;
}

QString NWaveformSliderPlugin::domXml() const
{
	return "<ui language=\"c++\">\n"
		" <widget class=\"NWaveformSlider\" name=\"waveformSlider\">\n"
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

QString NWaveformSliderPlugin::includeFile() const
{
	return "waveformSliderPlugin.h";
}

/* vim: set ts=4 sw=4: */
