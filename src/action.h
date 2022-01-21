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

#ifndef N_ACTION_H
#define N_ACTION_H

#include <QAction>

#include "qxtglobalshortcut.h"

class NAction : public QAction
{
    Q_OBJECT
    Q_PROPERTY(bool customizable READ isCustomizable WRITE setCustomizable)

public:
    NAction(QObject *parent) : QAction(parent) { init(); }
    NAction(const QString &text, QObject *parent) : QAction(text, parent) { init(); }
    NAction(const QIcon &icon, const QString &text, QObject *parent) : QAction(icon, text, parent)
    {
        init();
    }

    void setEnabled(bool enable);
    void setCustomizable(bool enable) { m_customizable = enable; }
    bool isCustomizable() const { return m_customizable; }

    QList<QKeySequence> globalShortcuts();
    void setGlobalShortcut(const QKeySequence &shortcut);
    void setGlobalShortcuts(QKeySequence::StandardKey key);
    void setGlobalShortcuts(const QList<QKeySequence> &shortcuts);

private:
    void init();
    bool m_customizable;
    QList<QxtGlobalShortcut *> m_globalShortcuts;
};

#endif
