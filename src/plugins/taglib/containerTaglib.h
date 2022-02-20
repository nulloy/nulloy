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

#ifndef N_CONTAINER_TAGLIB_H
#define N_CONTAINER_TAGLIB_H

#include "pluginContainer.h"

class NContainerTaglib : public QObject, public NPluginContainer
{
    Q_OBJECT
    Q_INTERFACES(NPluginContainer)
    Q_PLUGIN_METADATA(IID "com.nulloy.NContainerTaglib")

private:
    QList<NPlugin *> m_plugins;

public:
    NContainerTaglib(QObject *parent = NULL);
    ~NContainerTaglib();
    QList<NPlugin *> plugins() const;
    QString name() const { return "TagLib"; }
    QString version() const { return "0.9"; }
};

#endif
