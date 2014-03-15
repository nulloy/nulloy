/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2014 Sergey Vlasov <sergey@vlasov.me>
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

#include <QStyleOptionSlider>
#include <QMouseEvent>
#include <QWheelEvent>

NSlider::NSlider(QWidget *parent) : QSlider(parent) {}

void NSlider::mousePressEvent(QMouseEvent *event)
{
	if (event->button() != Qt::RightButton) {
		emit sliderPressed();

		QStyleOptionSlider opt;
		initStyleOption(&opt);
		QRect gr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
		QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

		int pxMin;
		int pxMax;
		if (orientation() == Qt::Horizontal) {
			pxMin = gr.x() +  sr.width() / 2;
			pxMax = gr.right() -  sr.width() / 2 + 1;
		} else {
			pxMin = gr.y() + sr.height() / 2;
			pxMax = gr.bottom() - sr.height() / 2 + 1;
		}

		setValue(QStyle::sliderValueFromPosition(minimum(), maximum(), event->x() - pxMin, pxMax - pxMin, opt.upsideDown));

		emit sliderMoved(value());
	}

	QSlider::mousePressEvent(event);
}

void NSlider::wheelEvent(QWheelEvent *event)
{
	QSlider::wheelEvent(event);
	emit sliderMoved(value());
}

