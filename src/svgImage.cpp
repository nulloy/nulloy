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

#include "svgImage.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QUrl>

NSvgImage::NSvgImage(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    m_svgRenderer = new QSvgRenderer(this);
}

void NSvgImage::setSource(const QString &source)
{
    m_source = source;
    QUrl url = QQmlEngine::contextForObject(this)->baseUrl().resolved(QUrl(m_source));
    if (!m_svgRenderer->load(url.toLocalFile())) {
        qCritical() << "Failed to load SVG file:" << m_source;
        m_source = "";
    }
}

void NSvgImage::setElementId(const QString &id)
{
    m_elementId = id;
    update();
}

void NSvgImage::paint(QPainter *painter)
{
    if (m_source.isEmpty() || m_elementId.isEmpty()) {
        return;
    }

    QRectF elementBounds = m_svgRenderer->boundsOnElement(m_elementId);
    if (!elementBounds.isValid()) {
        qCritical() << "SVG element ID not found:" << m_elementId;
        return;
    }

    qreal dpr = window()->devicePixelRatio();
    qreal scaleX = elementBounds.width() / width();
    qreal scaleY = elementBounds.height() / height();
    int offsetX = (width() - elementBounds.width()) / 2;
    int offsetY = (height() - elementBounds.height()) / 2;

    painter->translate(offsetX, offsetY);
    painter->scale(scaleX / dpr, scaleY / dpr);

    m_svgRenderer->render(painter, m_elementId);
}
