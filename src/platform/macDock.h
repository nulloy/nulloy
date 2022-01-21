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

#ifndef N_MAC_DOCK_H
#define N_MAC_DOCK_H

#include <QObject>

class NMacDock : public QObject
{
    Q_OBJECT

private:
    NMacDock() {}
    ~NMacDock() {}
    NMacDock(NMacDock const &copy);
    NMacDock operator=(NMacDock const &copy);

public:
    static NMacDock *instance();
    void registerClickHandler();

    void _emitClicked();

signals:
    void clicked();
};

#endif
