/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2022 Sergey Vlasov <sergey@vlasov.me>
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

#include "containerGstreamer.h"

#include "playbackEngineGstreamer.h"
#include "waveformBuilderGstreamer.h"
#ifdef _N_GSTREAMER_TAGREADER_PLUGIN_
#include "tagReaderGstreamer.h"
#endif

NContainerGstreamer::NContainerGstreamer(QObject *parent) : QObject(parent)
{
#ifdef Q_OS_WIN
    _putenv(QString("GST_REGISTRY=NUL").toUtf8());
    _putenv(QString("GST_REGISTRY_UPDATE=no").toUtf8());
#endif

    m_plugins << new NPlaybackEngineGStreamer()
#ifdef _N_GSTREAMER_TAGREADER_PLUGIN_
              << new NTagReaderGstreamer()
#endif
              << new NWaveformBuilderGstreamer();
}

NContainerGstreamer::~NContainerGstreamer()
{
    foreach (NPlugin *plugin, m_plugins)
        delete plugin;
}

QList<NPlugin *> NContainerGstreamer::plugins() const
{
    return m_plugins;
}
