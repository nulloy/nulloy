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

#ifndef N_CONTAINER_VLC_H
#define N_CONTAINER_VLC_H

#include "pluginContainer.h"

class NContainerVlc : public QObject, public NPluginContainer
{
    Q_OBJECT
    Q_INTERFACES(NPluginContainer)
    Q_PLUGIN_METADATA(IID "com.nulloy.NContainerVlc")

private:
    QList<NPlugin *> m_plugins;

public:
    NContainerVlc(QObject *parent = NULL);
    ~NContainerVlc();
    QList<NPlugin *> plugins() const;
    QString name() const { return "VLC"; }
    QString version() const { return "0.8"; }
};

#endif
