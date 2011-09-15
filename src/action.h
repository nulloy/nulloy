/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2011 Sergey Vlasov <sergey@vlasov.me>
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
	Q_PROPERTY(bool global READ isGlobal WRITE setGlobal)

public:
	NAction(QObject *parent) : QAction(parent), m_global(FALSE) {}
	NAction(const QString &text, QObject *parent) : QAction(text, parent), m_global(FALSE) {}
	NAction(const QIcon &icon, const QString &text, QObject *parent) : QAction(icon, text, parent), m_global(FALSE) {}

	void setEnabled(bool enable);
	bool isEnabled();
	void setGlobal(bool global);
	bool isGlobal() { return m_global; }

	void setShortcut(const QKeySequence &shortcut);
	void setShortcuts(const QList<QKeySequence> &shortcuts);
	void setShortcuts(QKeySequence::StandardKey key);

private:
	bool m_global;
	QList<QxtGlobalShortcut *> m_globalShortcuts;
};

#endif

/* vim: set ts=4 sw=4: */
