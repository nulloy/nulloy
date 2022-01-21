/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2022 Sergey Vlasov <sergey@vlasov.me>
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

#include "scriptQtPrototypes.h"

#include <QMouseEvent>

#include "global.h"

NWidgetPrototype::NWidgetPrototype(QObject *parent) : QObject(parent) {}

void NWidgetPrototype::enableDoubleClick()
{
    QWidget *widget = qscriptvalue_cast<QWidget *>(thisObject());
    if (widget) {
        widget->installEventFilter(this);
    }
}

bool NWidgetPrototype::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj)
    if (event->type() == QEvent::MouseButtonDblClick) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        if (mouseEvent->button() == Qt::LeftButton) {
            emit doubleClicked();
        }
    }

    return false;
}

void NWidgetPrototype::setParent(QWidget *parent)
{
    QWidget *widget = qscriptvalue_cast<QWidget *>(thisObject());
    if (widget) {
        widget->setParent(parent);
    }
}

int NWidgetPrototype::windowFlags() const
{
    QWidget *widget = qscriptvalue_cast<QWidget *>(thisObject());
    if (widget) {
        return (int)widget->windowFlags();
    } else {
        return 0;
    }
}

void NWidgetPrototype::setWindowFlags(int flags)
{
    QWidget *widget = qscriptvalue_cast<QWidget *>(thisObject());
    if (widget) {
        widget->setWindowFlags((Qt::WindowFlags)flags);
    }
}

void NWidgetPrototype::setAttribute(int attribute, bool enable)
{
    QWidget *widget = qscriptvalue_cast<QWidget *>(thisObject());
    if (widget) {
        widget->setAttribute((Qt::WidgetAttribute)attribute, enable);
    }
}

QWidget *NWidgetPrototype::parentWidget() const
{
    QWidget *widget = qscriptvalue_cast<QWidget *>(thisObject());
    if (widget) {
        return widget->parentWidget();
    } else {
        return NULL;
    }
}

void NWidgetPrototype::move(int x, int y)
{
    QWidget *widget = qscriptvalue_cast<QWidget *>(thisObject());
    if (widget) {
        widget->move(x, y);
    }
}

void NWidgetPrototype::resize(int w, int h)
{
    QWidget *widget = qscriptvalue_cast<QWidget *>(thisObject());
    if (widget) {
        widget->resize(w, h);
    }
}

void NWidgetPrototype::setStandardIcon(QString name, QString fallback)
{
    QAbstractButton *button = qscriptvalue_cast<QAbstractButton *>(thisObject());
    if (button) {
        button->setIcon(QIcon::fromTheme(name, QIcon(fallback)));
    }
}

void NWidgetPrototype::setSizeGripEnabled(bool enabled)
{
    QDialog *dialog = qscriptvalue_cast<QDialog *>(thisObject());
    if (dialog) {
        dialog->setSizeGripEnabled(enabled);
    }
}

void NWidgetPrototype::setFontSize(int size)
{
    QWidget *widget = qscriptvalue_cast<QWidget *>(thisObject());
    if (widget) {
        QFont font = widget->font();
        font.setPixelSize(size);
        widget->setFont(font);
    }
}

QLayout *NWidgetPrototype::layout() const
{
    QWidget *widget = qscriptvalue_cast<QWidget *>(thisObject());
    if (widget) {
        return widget->layout();
    } else {
        return NULL;
    }
}

NLayoutPrototype::NLayoutPrototype(QObject *parent) : QObject(parent) {}

QMargins NLayoutPrototype::contentsMargins() const
{
    QLayout *layout = qscriptvalue_cast<QLayout *>(thisObject());
    if (layout) {
        return layout->contentsMargins();
    } else {
        return QMargins();
    }
}

void NLayoutPrototype::setContentsMargins(int left, int top, int right, int bottom)
{
    QLayout *layout = qscriptvalue_cast<QLayout *>(thisObject());
    if (layout) {
        layout->setContentsMargins(left, top, right, bottom);
    }
}

void NLayoutPrototype::setContentsMargins(QMargins margins)
{
    QLayout *layout = qscriptvalue_cast<QLayout *>(thisObject());
    if (layout) {
        layout->setContentsMargins(margins);
    }
}

void NLayoutPrototype::setSpacing(int spacing)
{
    QLayout *layout = qscriptvalue_cast<QLayout *>(thisObject());
    if (layout) {
        layout->setSpacing(spacing);
    }
}

void NLayoutPrototype::setSpacingAt(int index, int spacing)
{
    QLayout *layout = qscriptvalue_cast<QLayout *>(thisObject());
    if (layout) {
        layout->removeItem(layout->itemAt(index));
        qobject_cast<QBoxLayout *>(layout)->insertSpacing(index, spacing);
    }
}

void NLayoutPrototype::insertSpacing(int index, int size)
{
    QLayout *layout = qscriptvalue_cast<QLayout *>(thisObject());
    if (layout) {
        qobject_cast<QBoxLayout *>(layout)->insertSpacing(index, size);
    }
}

void NLayoutPrototype::insertWidget(int index, QWidget *widget)
{
    QLayout *layout = qscriptvalue_cast<QLayout *>(thisObject());
    if (layout) {
        qobject_cast<QBoxLayout *>(layout)->insertWidget(index, widget);
    }
}

NSplitterPrototype::NSplitterPrototype(QObject *parent) : QObject(parent) {}

QList<int> NSplitterPrototype::sizes() const
{
    QSplitter *splitter = qscriptvalue_cast<QSplitter *>(thisObject());
    if (splitter) {
        return splitter->sizes();
    } else {
        return QList<int>();
    }
}

void NSplitterPrototype::setSizes(const QList<int> &list)
{
    QSplitter *splitter = qscriptvalue_cast<QSplitter *>(thisObject());
    if (splitter) {
        splitter->setSizes(list);
    }
}

QScriptValue NMarginsPrototype::toScriptValue(QScriptEngine *engine, const QMargins &m)
{
    QScriptValue obj = engine->newObject();
    obj.setProperty("bottom", m.bottom());
    obj.setProperty("left", m.left());
    obj.setProperty("right", m.right());
    obj.setProperty("top", m.top());
    return obj;
}

void NMarginsPrototype::fromScriptValue(const QScriptValue &obj, QMargins &m)
{
    m.setBottom(obj.property("bottom").toInt32());
    m.setLeft(obj.property("left").toInt32());
    m.setRight(obj.property("right").toInt32());
    m.setTop(obj.property("top").toInt32());
}

QScriptValue NPointPrototype::toScriptValue(QScriptEngine *engine, const QPoint &p)
{
    QScriptValue obj = engine->newObject();
    obj.setProperty("x", p.x());
    obj.setProperty("y", p.y());
    return obj;
}

void NPointPrototype::fromScriptValue(const QScriptValue &obj, QPoint &p)
{
    p.setX(obj.property("x").toInt32());
    p.setY(obj.property("y").toInt32());
}
