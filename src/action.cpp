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

void NAction::init()
{
	m_global = FALSE;
	m_customizable = FALSE;
}

void NAction::setEnabled(bool enable)
{
	foreach (QxtGlobalShortcut *shortcut, m_globalShortcuts)
		shortcut->setEnabled((enable == TRUE) ? m_global : FALSE);
	QAction::setEnabled(enable);
}

bool NAction::isEnabled()
{
	return (QAction::isEnabled() && m_global);
}

void NAction::setGlobal(bool enable)
{
	m_global = enable;
	foreach (QxtGlobalShortcut *shortcut, m_globalShortcuts)
		shortcut->setEnabled(m_global);
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
	setGlobalShortcuts(QList<QKeySequence>() << shortcut);
}

void NAction::setGlobalShortcuts(QKeySequence::StandardKey key)
{
	setGlobalShortcuts(QList<QKeySequence>() << QKeySequence(key));
}

void NAction::setGlobalShortcuts(const QList<QKeySequence> &shortcuts)
{
	setGlobal(FALSE);
	foreach (QKeySequence seq, shortcuts) {
		if (!seq.isEmpty()) {
			setGlobal(TRUE);
			break;
		}
	}

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

/* vim: set ts=4 sw=4: */
