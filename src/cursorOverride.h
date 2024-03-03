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

#ifndef N_CURSOR_OVERRIDE_H
#define N_CURSOR_OVERRIDE_H

#include <QObject>
#include <QtQml/qqml.h>

class NCursorOverride : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE void setOverrideCursor(Qt::CursorShape shape);
    Q_INVOKABLE void restoreOverrideCursor();
};

#endif // N_CURSOR_OVERRIDE_H
