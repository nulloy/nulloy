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

#include "action.h"
#include "qxtglobalshortcut.h"

bool NAction::isEnabled() const
{
    return QAction::isEnabled();
}

void NAction::setEnabled(bool enable)
{
    for (QxtGlobalShortcut *shortcut : m_globalShortcuts) {
        shortcut->setEnabled(enable);
    }
    QAction::setEnabled(enable);
}

void NAction::setCustomizable(bool enable)
{
    m_isCustomizable = enable;
}

bool NAction::isCustomizable() const
{
    return m_isCustomizable;
}

QList<QKeySequence> NAction::sequences() const
{
    return QAction::shortcuts();
}

QStringList NAction::shortcuts() const
{
    QStringList shortcuts;
    for (const QKeySequence &seq : sequences()) {
        shortcuts << seq.toString();
    }
    return shortcuts;
}

void NAction::setSequences(const QList<QKeySequence> &sequences)
{
    QAction::setShortcuts(sequences);
}

void NAction::setShortcuts(const QStringList &shortcuts)
{
    QList<QKeySequence> sequences;
    for (const QString &str : shortcuts) {
        sequences << QKeySequence(str);
    }
    setSequences(sequences);
}

QList<QKeySequence> NAction::globalSequences() const
{
    QList<QKeySequence> sequences;
    for (QxtGlobalShortcut *shortcut : m_globalShortcuts) {
        sequences << shortcut->shortcut();
    }
    return sequences;
}

QStringList NAction::globalShortcuts() const
{
    QStringList shortcuts;
    for (const QKeySequence &seq : globalSequences()) {
        shortcuts << seq.toString();
    }
    return shortcuts;
}

void NAction::setGlobalSequences(const QList<QKeySequence> &sequences)
{
    for (QxtGlobalShortcut *shortcut : m_globalShortcuts) {
        delete shortcut;
    }
    m_globalShortcuts.clear();

    for (const QKeySequence &seq : sequences) {
        QxtGlobalShortcut *s = new QxtGlobalShortcut(this);
        connect(s, &QxtGlobalShortcut::activated, this, &QAction::trigger);
        s->setShortcut(seq);
        m_globalShortcuts << s;
    }
}

void NAction::setGlobalShortcuts(const QStringList &shortcuts)
{
    QList<QKeySequence> sequences;
    for (const QString &str : shortcuts) {
        sequences << QKeySequence(str);
    }
    setGlobalSequences(sequences);
}
