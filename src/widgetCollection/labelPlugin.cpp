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

#include "label.h"
#include "labelPlugin.h"
#include <QtPlugin>

NLabelPlugin::NLabelPlugin(const QString &group, QObject *parent) : QObject(parent)
{
	m_initialized = FALSE;
	m_group = group;
}

void NLabelPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
	if (m_initialized)
		return;

	m_initialized = TRUE;
}

bool NLabelPlugin::isInitialized() const
{
	return m_initialized;
}

QWidget *NLabelPlugin::createWidget(QWidget *parent)
{
	return new NLabel(parent);
}

QString NLabelPlugin::name() const
{
	return "NLabel";
}

QString NLabelPlugin::group() const
{
	return m_group;
}

QIcon NLabelPlugin::icon() const
{
	return QIcon();
}

QString NLabelPlugin::toolTip() const
{
	return "";
}

QString NLabelPlugin::whatsThis() const
{
	return "";
}

bool NLabelPlugin::isContainer() const
{
	return FALSE;
}

QString NLabelPlugin::domXml() const
{
	return "<ui language=\"c++\">\n"
		" <widget class=\"NLabel\" name=\"label\">\n"
		"  <property name=\"geometry\">\n"
		"   <rect>\n"
		"    <x>0</x>\n"
		"    <y>0</y>\n"
		"    <width>130</width>\n"
		"    <height>90</height>\n"
		"   </rect>\n"
		"  </property>\n"
		" </widget>\n"
		"</ui>";
}

QString NLabelPlugin::includeFile() const
{
	return "label.h";
}

/* vim: set ts=4 sw=4: */
