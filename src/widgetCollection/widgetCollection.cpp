/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2014 Sergey Vlasov <sergey@vlasov.me>
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

#include "dropArea.h"
#include "label.h"
#include "playlistWidget.h"
#include "slider.h"
#include "volumeSlider.h"
#include "waveformSlider.h"
#include "coverWidget.h"

#include <QtPlugin>
#include <QSizeGrip>

#define _N_WIDGET_PLUGIN_(CLASS_NAME, IS_CONTAINER) \
class CLASS_NAME##Plugin : public QObject, public NWidgetPlugin \
{ \
public: \
	CLASS_NAME##Plugin(QObject *parent = 0) : QObject(parent), NWidgetPlugin(#CLASS_NAME) {} \
	virtual QWidget* createWidget(QWidget *parent) { return new CLASS_NAME(parent); } \
	virtual bool isContainer() const { return IS_CONTAINER; } \
}; \

#define N_WIDGET_PLUGIN(CLASS_NAME) _N_WIDGET_PLUGIN_(CLASS_NAME, FALSE)
#define N_WIDGET_CONTAINER_PLUGIN(CLASS_NAME) _N_WIDGET_PLUGIN_(CLASS_NAME, TRUE)

N_WIDGET_PLUGIN(NLabel)
N_WIDGET_CONTAINER_PLUGIN(NDropArea)
N_WIDGET_PLUGIN(NPlaylistWidget)
N_WIDGET_PLUGIN(NSlider)
N_WIDGET_PLUGIN(NVolumeSlider)
N_WIDGET_PLUGIN(NWaveformSlider)
N_WIDGET_PLUGIN(NCoverWidget)
N_WIDGET_PLUGIN(QSizeGrip)

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
	m_plugins.push_back(new QSizeGripPlugin(this));
	m_plugins.push_back(new NSliderPlugin(this));
	m_plugins.push_back(new NVolumeSliderPlugin(this));
	m_plugins.push_back(new NPlaylistWidgetPlugin(this));
	m_plugins.push_back(new NLabelPlugin(this));
	m_plugins.push_back(new NCoverWidgetPlugin(this));
}

Q_EXPORT_PLUGIN2(widget_collection, NWidgetCollection)

