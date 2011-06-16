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

#include "label.h"
#include <QPen>
#include <QDebug>

NLabel::NLabel(QWidget* parent) : QLabel(parent)
{
	mShadowOffset = QPoint(0, 0);
	mEnabled = FALSE;
	mShadowColor = Qt::gray;
}

bool NLabel::shadowEnabled() const
{
	return mEnabled;
}

void NLabel::setShadowEnabled(bool enabled)
{
	if (mEnabled != enabled) {
		mEnabled = enabled;
		update();
	}
}

QPoint NLabel::shadowOffset() const
{
	return mShadowOffset;
}

void NLabel::setShadowOffset(const QPoint &offset)
{
	if (mShadowOffset != offset) {
		mShadowOffset = offset;
		update();
	}
}

QColor NLabel::shadowColor() const
{
	return mShadowColor;
}

void NLabel::setShadowColor(QColor color)
{
	if (mShadowColor != color) {
		mShadowColor = color;
		update();
	}
}

void NLabel::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	QPainter painter;
	painter.setFont(font());
	if (mEnabled && mShadowOffset != QPoint(0, 0)) {
		painter.begin(this);
		painter.setPen(QPen(mShadowColor));
		painter.drawText(rect().translated(mShadowOffset), alignment(), text());
		painter.end();
	}

	painter.begin(this);
	painter.drawText(rect(), alignment(), text());
	painter.end();
}

/* vim: set ts=4 sw=4: */
