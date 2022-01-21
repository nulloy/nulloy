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

#include "containerPhonon.h"

#include "playbackEnginePhonon.h"
#include "waveformBuilderPhonon.h"

NContainerPhonon::NContainerPhonon(QObject *parent) : QObject(parent)
{
    m_plugins << new NPlaybackEnginePhonon() << new NWaveformBuilderPhonon();
}

NContainerPhonon::~NContainerPhonon()
{
    foreach (NPlugin *plugin, m_plugins)
        delete plugin;
}

QList<NPlugin *> NContainerPhonon::plugins() const
{
    return m_plugins;
}
