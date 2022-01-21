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

#include "action.h"

void NAction::init()
{
    m_customizable = false;
}

void NAction::setEnabled(bool enable)
{
    foreach (QxtGlobalShortcut *shortcut, m_globalShortcuts)
        shortcut->setEnabled(enable);
    QAction::setEnabled(enable);
}

QList<QKeySequence> NAction::globalShortcuts()
{
    QList<QKeySequence> list;
    foreach (QxtGlobalShortcut *shortcut, m_globalShortcuts)
        list << shortcut->shortcut();
    return list;
}

void NAction::setGlobalShortcut(const QKeySequence &shortcut)
{
    QList<QKeySequence> list;
    if (!shortcut.isEmpty()) {
        list << shortcut;
    }
    setGlobalShortcuts(list);
}

void NAction::setGlobalShortcuts(QKeySequence::StandardKey key)
{
    setGlobalShortcuts(QList<QKeySequence>() << QKeySequence(key));
}

void NAction::setGlobalShortcuts(const QList<QKeySequence> &shortcuts)
{
    foreach (QxtGlobalShortcut *shortcut, m_globalShortcuts)
        delete shortcut;
    m_globalShortcuts.clear();

    foreach (QKeySequence seq, shortcuts) {
        QxtGlobalShortcut *s = new QxtGlobalShortcut(this);
        connect(s, SIGNAL(activated()), this, SLOT(trigger()));
        s->setShortcut(seq);
        m_globalShortcuts << s;
    }
}
