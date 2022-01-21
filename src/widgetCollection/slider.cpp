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

#include "slider.h"

#include <QMouseEvent>
#include <QStyleOptionSlider>
#include <QWheelEvent>

NSlider::NSlider(QWidget *parent) : QSlider(parent) {}

qreal NSlider::valueAtPos(int pos)
{
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QRect gr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
    QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

    int pxMin;
    int pxMax;
    if (orientation() == Qt::Horizontal) {
        pxMin = gr.x() + sr.width() / 2;
        pxMax = gr.right() - sr.width() / 2 + 1;
    } else {
        pxMin = gr.y() + sr.height() / 2;
        pxMax = gr.bottom() - sr.height() / 2 + 1;
    }

    return QStyle::sliderValueFromPosition(minimum(), maximum(), pos - pxMin, pxMax - pxMin,
                                           opt.upsideDown) /
           (qreal)maximum();
}

void NSlider::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::RightButton) {
        emit sliderPressed();
        int pos = (orientation() == Qt::Horizontal) ? event->x() : event->y();
        qreal val = valueAtPos(pos);
        setValue(val);
        emit sliderMoved(val);
    }

    QSlider::mousePressEvent(event);
}

void NSlider::mouseMoveEvent(QMouseEvent *event)
{
    int pos = (orientation() == Qt::Horizontal) ? event->x() : event->y();
    qreal val = valueAtPos(pos);
    setValue(val);
    emit sliderMoved(val);

    QSlider::mouseMoveEvent(event);
}

void NSlider::wheelEvent(QWheelEvent *event)
{
    QSlider::wheelEvent(event);

    qreal val = value() / (qreal)maximum();
    emit sliderMoved(val);
}

void NSlider::setValue(qreal value)
{
    QSlider::setValue(qRound(value * maximum()));
}
