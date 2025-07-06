/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2025 Sergey Vlasov <sergey@vlasov.me>
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

#ifndef N_ACTION_MANAGER_H
#define N_ACTION_MANAGER_H

#include "action.h"

class NPlayer;

class NActionManager : public QObject
{
    Q_OBJECT

public:
    NActionManager(NPlayer *player);

    QMenu *contextMenu();
    QMenu *playlistContextMenu();
    QMenu *trayIconMenu();

private:
    QMenu *m_contextMenu;
    QMenu *m_playlistContextMenu;
    QMenu *m_trayIconMenu;
};

#endif
