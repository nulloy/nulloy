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

#include "label.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QPoint>

NLabel::NLabel(QWidget *parent) : QLabel(parent)
{
    m_shadowOffset = QPoint(0, 0);
    m_enabled = false;
    m_shadowColor = Qt::gray;
    m_elideMode = Qt::ElideRight;
}

void NLabel::setText(const QString &text)
{
    QLabel::setText(text);
    updateElidedText();
}

void NLabel::setElideMode(Qt::TextElideMode mode)
{
    m_elideMode = mode;
}

bool NLabel::shadowEnabled() const
{
    return m_enabled;
}

void NLabel::setShadowEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        update();
    }
}

QPoint NLabel::shadowOffset() const
{
    return m_shadowOffset;
}

void NLabel::setShadowOffset(const QPoint &offset)
{
    if (m_shadowOffset != offset) {
        m_shadowOffset = offset;
        update();
    }
}

QColor NLabel::shadowColor() const
{
    return m_shadowColor;
}

void NLabel::setShadowColor(QColor color)
{
    if (m_shadowColor != color) {
        m_shadowColor = color;
        update();
    }
}

void NLabel::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    updateElidedText();
}

void NLabel::updateElidedText()
{
    m_elidedText = fontMetrics().elidedText(text(), m_elideMode, contentsRect().width());
}

void NLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter;
    QRect rect = contentsRect();

    if (painter.isActive()) {
        painter.setFont(font());
    }

    if (m_enabled && m_shadowOffset != QPoint(0, 0)) {
        painter.begin(this);
        painter.setPen(QPen(m_shadowColor));
        painter.drawText(rect.translated(m_shadowOffset), alignment(), m_elidedText);
        painter.end();
    }

    painter.begin(this);
    painter.drawText(rect, alignment(), m_elidedText);
    painter.end();
}
