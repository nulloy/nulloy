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

#ifndef N_SCRIPT_PROTOTYPES_H
#define N_SCRIPT_PROTOTYPES_H

#include <QObject>
#include <QScriptable>
#include <QScriptValue>
#include <QScriptEngine>
#include <QWidget>
#include <QLayout>

class NWidgetPrototype : public QObject, public QScriptable
{
	Q_OBJECT
	Q_PROPERTY(int windowFlags READ windowFlags WRITE setWindowFlags)

public:
	NWidgetPrototype(QObject *parent = 0);
	int windowFlags();
	void setWindowFlags(int flags);
	Q_INVOKABLE void setAttribute(int attribute, bool enable = TRUE);
	Q_INVOKABLE QWidget* parentWidget();
	Q_INVOKABLE void move(int x, int y);
	Q_INVOKABLE void resize(int w, int h);
	Q_INVOKABLE void setSizeGripEnabled(bool enabled);
	Q_INVOKABLE void setStandardIcon(QString name, QString fallback = "");
	Q_INVOKABLE QLayout* layout();
	Q_INVOKABLE void setFontSize(int size);

public slots:
	void setParent(QWidget *parent);
};

class NLayoutPrototype : public QObject, public QScriptable
{
	Q_OBJECT

public:
	NLayoutPrototype(QObject *parent = 0);
	Q_INVOKABLE QMargins contentsMargins();
	Q_INVOKABLE void setContentsMargins(int left, int top, int right, int bottom);
	Q_INVOKABLE void setContentsMargins(QMargins margins);
	Q_INVOKABLE void setSpacing(int spacing);
	Q_INVOKABLE void setSpacingAt(int index, int spacing);
	Q_INVOKABLE void insertWidget(int index, QWidget *widget);
};

namespace NMarginsPrototype
{
	QScriptValue toScriptValue(QScriptEngine *engine, const QMargins &m);
	void fromScriptValue(const QScriptValue &obj, QMargins &m);
}

#endif

/* vim: set ts=4 sw=4: */

