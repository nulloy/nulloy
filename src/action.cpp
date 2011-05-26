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

#include "action.h"

void NAction::setEnabled(bool enable)
{
	if (enable) {
		foreach (QxtGlobalShortcut *shortcut, m_globalShortcuts)
			shortcut->setEnabled(m_global);
	} else {
		foreach (QxtGlobalShortcut *shortcut, m_globalShortcuts)
			shortcut->setEnabled(FALSE);
	}

}

void NAction::setGlobal(bool global)
{
	m_global = global;

	foreach (QxtGlobalShortcut *shortcut, m_globalShortcuts)
		shortcut->setEnabled(m_global);
}

void NAction::setShortcut(const QKeySequence &shortcut)
{
	foreach (QxtGlobalShortcut *shortcut, m_globalShortcuts)
		delete shortcut;
	m_globalShortcuts.clear();

	QxtGlobalShortcut *s = new QxtGlobalShortcut(this);
	connect(s, SIGNAL(activated()), this, SLOT(trigger()));
	s->setShortcut(shortcut);
	m_globalShortcuts << s;

	QAction::setShortcut(shortcut);
}

void NAction::setShortcuts(const QList<QKeySequence> &shortcuts)
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

	QAction::setShortcuts(shortcuts);
}

void NAction::setShortcuts(QKeySequence::StandardKey key)
{
	setShortcut(key);
}

/* vim: set ts=4 sw=4: */
