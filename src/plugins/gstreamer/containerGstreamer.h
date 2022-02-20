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

#ifndef N_CONTAINER_GSTREAMER_H
#define N_CONTAINER_GSTREAMER_H

#include "pluginContainer.h"

class NContainerGstreamer : public QObject, public NPluginContainer
{
    Q_OBJECT
    Q_INTERFACES(NPluginContainer)
    Q_PLUGIN_METADATA(IID "com.nulloy.NContainerGstreamer")

private:
    QList<NPlugin *> m_plugins;

public:
    NContainerGstreamer(QObject *parent = NULL);
    ~NContainerGstreamer();
    QList<NPlugin *> plugins() const;
    QString name() const { return "GStreamer"; }
    QString version() const { return "0.9"; }
};

#endif
