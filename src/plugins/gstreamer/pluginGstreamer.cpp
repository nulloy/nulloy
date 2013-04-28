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

#include "pluginGstreamer.h"
#include "playbackEngineGstreamer.h"
#include "waveformBuilderGstreamer.h"
#ifdef _N_GSTREAMER_TAGREADER_PLUGIN_
#include "tagReaderGstreamer.h"
#endif

NPluginGstreamer::NPluginGstreamer(QObject *parent) : QObject(parent)
{
	m_elements << new NPlaybackEngineGStreamer()
#ifdef _N_GSTREAMER_TAGREADER_PLUGIN_
				<< new NTagReaderGstreamer()
#endif
				<< new NWaveformBuilderGstreamer();
}

NPluginGstreamer::~NPluginGstreamer()
{
	foreach (QObject *obj, m_elements)
		delete obj;
}

QObjectList NPluginGstreamer::elements()
{
	return m_elements;
}

#if !defined _N_GSTREAMER_PLUGINS_BUILTIN_ && !defined _N_NO_PLUGINS_
Q_EXPORT_PLUGIN2(plugin_gstreamer, NPluginGstreamer)
#endif

/* vim: set ts=4 sw=4: */
