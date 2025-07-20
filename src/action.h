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

#ifndef N_ACTION_H
#define N_ACTION_H

#include <QAction>

class QxtGlobalShortcut;

class NAction : public QAction
{
    Q_OBJECT

public:
    using QAction::QAction;

    bool isEnabled() const;
    void setEnabled(bool enable);

    bool isCustomizable() const;
    void setCustomizable(bool enable);

    QList<QKeySequence> sequences() const;
    void setSequences(const QList<QKeySequence> &sequences);
    QStringList shortcuts() const;
    void setShortcuts(const QStringList &shortcuts);

    QList<QKeySequence> globalSequences() const;
    void setGlobalSequences(const QList<QKeySequence> &sequences);
    QStringList globalShortcuts() const;
    void setGlobalShortcuts(const QStringList &shortcuts);

private:
    bool m_isCustomizable = false;
    QList<QxtGlobalShortcut *> m_globalShortcuts;
};

#endif
