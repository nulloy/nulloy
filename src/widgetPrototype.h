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

#ifndef N_WIDGET_PROTOTYPE_H
#define N_WIDGET_PROTOTYPE_H

#include <QObject>
#include <QScriptable>

class NWidgetPrototype : public QObject, public QScriptable
{
	Q_OBJECT
	Q_PROPERTY(int windowFlags READ windowFlags WRITE setWindowFlags)

public:
	NWidgetPrototype(QObject *parent = 0);
	int windowFlags();
	void setWindowFlags(int flags);
	Q_INVOKABLE QWidget* parentWidget();
	Q_INVOKABLE void move(int x, int y);
	Q_INVOKABLE void resize(int w, int h);
	Q_INVOKABLE void setStandardIcon(QString name, QString fallback = "");

public slots:
	void setParent(QWidget *parent);
};

#endif

/* vim: set ts=4 sw=4: */

