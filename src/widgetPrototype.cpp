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

#include "widgetPrototype.h"

#include <QWidget>
#include <QPushButton>

#include <QStyle>
#include <QScriptValue>
#include <QScriptEngine>
#include <QDebug>

Q_DECLARE_METATYPE(QWidget *)
Q_DECLARE_METATYPE(QPushButton *)

NWidgetPrototype::NWidgetPrototype(QObject *parent) : QObject(parent) {}

void NWidgetPrototype::setParent(QWidget *parent)
{
	QWidget *widget = qscriptvalue_cast<QWidget *>(thisObject());
	if (widget)
		widget->setParent(parent);
}

int NWidgetPrototype::windowFlags()
{
	QWidget *widget = qscriptvalue_cast<QWidget *>(thisObject());
	if (widget)
		return (int)widget->windowFlags();
	else
		return 0;
}

void NWidgetPrototype::setWindowFlags(int flags)
{
	QWidget *widget = qscriptvalue_cast<QWidget *>(thisObject());
	if (widget)
		widget->setWindowFlags((Qt::WindowFlags)flags);
}

QWidget* NWidgetPrototype::parentWidget()
{
	QWidget *widget = qscriptvalue_cast<QWidget *>(thisObject());
	if (widget)
		return widget->parentWidget();
	else
		return NULL;
}

void NWidgetPrototype::move(int x, int y)
{
	QWidget *widget = qscriptvalue_cast<QWidget *>(thisObject());
	if (widget)
		widget->move(x, y);
}

void NWidgetPrototype::resize(int w, int h)
{
	QWidget *widget = qscriptvalue_cast<QWidget *>(thisObject());
	if (widget)
		widget->resize(w, h);
}

void NWidgetPrototype::setStandardIcon(QString name, QString fallback)
{
	QPushButton *button = qscriptvalue_cast<QPushButton *>(thisObject());
	if (button)
		button->setIcon(QIcon::fromTheme(name, QIcon(fallback)));
}

/* vim: set ts=4 sw=4: */
