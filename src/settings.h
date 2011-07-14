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

#ifndef N_SETTINGS_H
#define N_SETTINGS_H

#include <QAction>
#include <QVariant>

namespace NSettings
{
	void init(QObject *parent = 0);

	void initShortcuts(QObject *instance);
	void saveShortcuts();
	void loadShortcuts();
	QList<QAction *> shortcuts();

	QVariant value(const QString &key);
	void setValue(const QString &key, const QVariant &value);
	void remove(const QString &key);
}

#endif

/* vim: set ts=4 sw=4: */
