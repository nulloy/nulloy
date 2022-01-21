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

#include "coverWidgetPopup.h"

#include <QBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QPropertyAnimation>

#define MARGIN 50

NCoverWidgetPopup::NCoverWidgetPopup(QWidget *parent) : QWidget(parent)
{
    m_pixmapLabel = new QLabel;
    m_pixmapLabel->setStyleSheet("border: 1px solid white;");

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
    hLayout->addWidget(m_pixmapLabel);
    hLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(MARGIN, MARGIN, MARGIN, MARGIN);
    vLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
    vLayout->addLayout(hLayout);
    vLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));

    QWidget *container = new QWidget();
    QVBoxLayout *cLayout = new QVBoxLayout;
    cLayout->addWidget(container);
    cLayout->setContentsMargins(0, 0, 0, 0);
    container->setLayout(vLayout);
    container->setStyleSheet("background-color: rgba(0, 0, 0, 150);");

    this->setLayout(cLayout);

    m_effect = new QGraphicsOpacityEffect(this);
    m_effect->setOpacity(1.0);
    setGraphicsEffect(m_effect);

    m_animation = new QPropertyAnimation(m_effect, "opacity", this);
    connect(m_animation, SIGNAL(finished()), this, SLOT(on_animation_finished()));
    m_animation->setDuration(150);
    m_animation->setEasingCurve(QEasingCurve::OutQuad);
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);

    QWidget::window()->installEventFilter(this);
}

void NCoverWidgetPopup::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }

#ifndef Q_OS_MAC // QTBUG-15367
    m_animation->setDirection(QAbstractAnimation::Backward);
    if (m_animation->state() == QAbstractAnimation::Stopped) {
        m_animation->start();
    }
#else
    hide();
#endif
}

void NCoverWidgetPopup::setPixmap(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    setToolTip(QString("%1 x %2").arg(m_pixmap.width()).arg(m_pixmap.height()));
    resize(QWidget::window()->size());
}

void NCoverWidgetPopup::resize(const QSize &size)
{
    QSize margin = QSize(MARGIN * 2, MARGIN * 2);
    QSize pixmapMaxSize = size - margin;
    QPixmap pixmap = m_pixmap;
    if (m_pixmap.height() > pixmapMaxSize.height() || m_pixmap.width() > pixmapMaxSize.width()) {
        pixmap = m_pixmap.scaled(pixmapMaxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    m_pixmapLabel->setPixmap(pixmap);
    setMinimumSize(size);
    setMaximumSize(size);
}

void NCoverWidgetPopup::on_animation_finished()
{
    if (m_animation->direction() == QAbstractAnimation::Backward) {
        hide();
    }
}

void NCoverWidgetPopup::show()
{
    QWidget::show();
    m_animation->setDirection(QAbstractAnimation::Forward);
    if (m_animation->state() == QAbstractAnimation::Stopped) {
        m_animation->start();
    }
}

bool NCoverWidgetPopup::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::Resize) {
        resize(dynamic_cast<QResizeEvent *>(event)->size());
    }

    return false;
}
