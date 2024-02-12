/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2024 Sergey Vlasov <sergey@vlasov.me>
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

#include "coverWidget.h"

#include <QResizeEvent>

#include "coverWidgetPopup.h"
#include "pluginLoader.h"
#include "settings.h"

NCoverWidget::NCoverWidget(QWidget *parent) : QLabel(parent)
{
    m_popup = NULL;

    setScaledContents(true);
    hide();
}

NCoverWidget::~NCoverWidget() {}

void NCoverWidget::setPixmap(const QPixmap &pixmap)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);

    setMinimumSize(10, 10);
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    m_pixmap = pixmap;
    QLabel::setPixmap(m_pixmap);
    show();
}

void NCoverWidget::resizeEvent(QResizeEvent *event)
{
    fitToHeight(event->size().height());
}

void NCoverWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }

    if (!m_popup) {
        m_popup = new NCoverWidgetPopup(QWidget::window());
    }
    m_popup->setPixmap(m_pixmap);
    m_popup->show();
}

void NCoverWidget::fitToHeight(int height)
{
    if (height == 0 || m_pixmap.height() == 0) {
        return;
    }
    double scale = (double)height / m_pixmap.height();
    setFixedWidth(m_pixmap.width() * scale);
}
