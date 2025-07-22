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

#include "waveformView.h"

#include "pluginLoader.h"
#include "waveformBuilderInterface.h"

#define IDLE_INTERVAL 60
#define FAST_INTERVAL 10

NWaveformView::NWaveformView(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    m_color = Qt::black;
    m_borderColor = Qt::gray;
    m_borderWidth = 1;

    setAntialiasing(true);

    m_waveBuilder = dynamic_cast<NWaveformBuilderInterface *>(
        NPluginLoader::getPlugin(N::WaveformBuilder));
    Q_ASSERT(m_waveBuilder);

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(checkForUpdate()));
    m_timer->setInterval(IDLE_INTERVAL);
    m_timer->start();

    init();
}

void NWaveformView::setColor(const QColor &color)
{
    m_color = color;
    update();
}

void NWaveformView::setBorderColor(const QColor &color)
{
    m_borderColor = color;
    update();
}

void NWaveformView::setBorderWidth(qreal width)
{
    m_borderWidth = width;
    update();
}

void NWaveformView::setGradientStops(const QVariantList &stops)
{
    m_gradientStops = stops;
    update();
}

void NWaveformView::paint(QPainter *painter)
{
    float builderPos;
    int builderIndex;
    m_waveBuilder->positionAndIndex(builderPos, builderIndex);

    QPainterPath pathPos;
    QPainterPath pathNeg;
    pathPos.moveTo(0, height() / 2.0);
    pathNeg.moveTo(0, height() / 2.0);
    const NWaveformPeaks &peaks = m_waveBuilder->peaks();
    for (int i = 0; i < builderIndex; ++i) {
        pathPos.lineTo((i / (qreal)m_oldBuilderIndex) * m_oldBuilderPos * width(),
                       (1 + peaks.positive(i)) * (height() / 2.0));
        pathNeg.lineTo((i / (qreal)m_oldBuilderIndex) * m_oldBuilderPos * width(),
                       (1 + peaks.negative(i)) * (height() / 2.0));
    }

    pathPos.connectPath(pathNeg.toReversed());
    painter->setPen(QPen(m_borderColor, m_borderWidth));

    QBrush brush;
    if (!m_gradientStops.isEmpty()) {
        QLinearGradient gradient(0, 0, 0, height());
        for (int i = 0; i < m_gradientStops.size(); ++i) {
            QVariantMap stop = m_gradientStops.at(i).toMap();
            qreal position = stop["position"].toFloat();
            QColor color = QColor(stop["color"].toString());
            gradient.setColorAt(position, color);
        }
        brush = gradient;
    } else {
        brush = QBrush(m_color);
    }
    painter->setBrush(brush);
    painter->drawPath(pathPos);
}

void NWaveformView::init()
{
    m_oldBuilderIndex = -1;
    m_oldBuilderPos = -1;
    m_needsUpdate = false;
}

void NWaveformView::checkForUpdate()
{
    if (!m_waveBuilder) {
        return;
    }

    float builderPos;
    int builderIndex;
    m_waveBuilder->positionAndIndex(builderPos, builderIndex);

    if (m_oldBuilderIndex != builderIndex) {
        m_needsUpdate = true;
    }

    if ((builderPos != 0.0 && builderPos != 1.0) && m_timer->interval() != FAST_INTERVAL) {
        m_timer->setInterval(FAST_INTERVAL);
    } else if ((builderPos == 0.0 || builderPos == 1.0) && m_timer->interval() != IDLE_INTERVAL) {
        m_timer->setInterval(IDLE_INTERVAL);
    }

    if (m_needsUpdate) {
        m_oldBuilderPos = builderPos;
        m_oldBuilderIndex = builderIndex;

        update();
        m_needsUpdate = false;
    }
}
