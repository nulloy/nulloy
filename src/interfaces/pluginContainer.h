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

#ifndef N_PLUGIN_CONTAINER_H
#define N_PLUGIN_CONTAINER_H

#include <QtCore>

class NPlugin;

class NPluginContainer
{
public:
    NPluginContainer() {}
    virtual ~NPluginContainer() {}
    virtual QList<NPlugin *> plugins() const = 0;
    virtual QString name() const = 0;
    virtual QString version() const = 0;
};

Q_DECLARE_INTERFACE(NPluginContainer, "Nulloy/NPluginContainer/0.7")

#endif
