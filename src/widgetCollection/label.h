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

#ifndef N_LABEL_H
#define N_LABEL_H

#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QPoint>

class NLabel : public QLabel
{
	Q_OBJECT
	Q_PROPERTY(int xOffset READ xOffset WRITE setXOffset)
	Q_PROPERTY(int yOffset READ yOffset WRITE setYOffset)
	Q_PROPERTY(QPoint shadowOffset READ shadowOffset WRITE setShadowOffset)
	Q_PROPERTY(QColor shadowColor READ shadowColor WRITE setShadowColor)
	Q_PROPERTY(bool shadowEnabled READ shadowEnabled WRITE setShadowEnabled)

private:
	QPoint mShadowOffset;
	QColor mShadowColor;
	bool mEnabled;

	void paintEvent(QPaintEvent *event);

public:
	NLabel(QWidget* parent = 0);

	bool shadowEnabled() const;
	void setShadowEnabled(bool enabled);

	QPoint shadowOffset() const;
	inline int xOffset() const { return shadowOffset().x(); }
	inline int yOffset() const { return shadowOffset().y(); }

	void setShadowOffset(const QPoint &offset);
	Q_INVOKABLE inline void setShadowOffset(int dx, int dy) { setShadowOffset(QPoint(dx, dy)); }
	inline void setShadowOffset(int d) { setShadowOffset(QPoint(d, d)); }
	inline void setXOffset(int dx) { setShadowOffset(QPoint(dx, yOffset())); }
	inline void setYOffset(int dy) { setShadowOffset(QPoint(xOffset(), dy)); }

	QColor shadowColor() const;
	void setShadowColor(QColor color);
	Q_INVOKABLE inline void setShadowColor(QString color) { setShadowColor(QColor(color)); }
};

#endif

/* vim: set ts=4 sw=4: */
