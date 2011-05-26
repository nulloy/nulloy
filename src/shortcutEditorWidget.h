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

#ifndef N_SHORTCUT_EDITOR_WIDGET_H
#define N_SHORTCUT_EDITOR_WIDGET_H

#include <QtGui>
#include "action.h"

class NShortcutEditorWidget : public QTableWidget
{
public:
	NShortcutEditorWidget(QWidget *parent = 0);
	~NShortcutEditorWidget(void);
	void applyShortcuts();
	void init(const QList<NAction *> &actionList);

private:
	static QString keyEventToString(QKeyEvent *e);
	void keyPressEvent(QKeyEvent *e);
	QList<NAction *> m_actionList;
};

#endif

/* vim: set ts=4 sw=4: */
