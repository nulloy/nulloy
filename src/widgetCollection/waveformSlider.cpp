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

#include "waveformSlider.h"

#include <QFile>
#include <QMouseEvent>
#include <QPainterPath>
#include <QStyleOptionFocusRect>
#include <QStylePainter>

#include "playlistDataItem.h"
#include "pluginLoader.h"
#include "settings.h"
#include "utils.h"
#include "waveformBuilderInterface.h"

#define IDLE_INTERVAL 60
#define FAST_INTERVAL 10

NWaveformSlider::NWaveformSlider(QWidget *parent) : QAbstractSlider(parent)
{
    m_radius = 0;
    m_fileDrop = false;
    m_background = QBrush(Qt::darkBlue);
    m_waveBackground = QBrush(Qt::darkGreen);
    m_waveBorderColor = QColor(Qt::green);
    m_progressPlayingBackground = QBrush(Qt::darkCyan);
    m_progressPausedBackground = QBrush(Qt::darkYellow);
    m_remainingPlayingBackground = QBrush(Qt::NoBrush);
    m_remainingPausedBackground = QBrush(Qt::NoBrush);
    m_groovePlayingColor = QColor(Qt::transparent);
    m_groovePausedColor = QColor(Qt::transparent);
    m_playingComposition = QPainter::CompositionMode_Screen;
    m_pausedComposition = QPainter::CompositionMode_Screen;
    m_fileDropBorderColor = QColor(Qt::transparent);
    m_fileDropBackground = QBrush(Qt::NoBrush);

    setMinimum(0);
    setMaximum(10000);

    m_waveBuilder = dynamic_cast<NWaveformBuilderInterface *>(
        NPluginLoader::getPlugin(N::WaveformBuilder));
    Q_ASSERT(m_waveBuilder);

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(checkForUpdate()));
    m_timer->setInterval(IDLE_INTERVAL);
    m_timer->start();

    setAcceptDrops(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    setMinimumHeight(50);
    setMinimumWidth(150);

    m_oldSize = QSize(0, 0);
    init();
}

void NWaveformSlider::setPausedState(bool state)
{
    m_pausedState = state;
    update();
}

void NWaveformSlider::init()
{
    m_oldBuilderIndex = -1;
    m_oldBuilderPos = -1;
    m_pausedState = false;
    m_needsUpdate = false;
    m_hasMedia = false;
}

QSize NWaveformSlider::sizeHint() const
{
    return QSize(200, 80);
}

NWaveformSlider::~NWaveformSlider() {}

void NWaveformSlider::resizeEvent(QResizeEvent *event)
{
    QAbstractSlider::resizeEvent(event);
    checkForUpdate();
}

void NWaveformSlider::checkForUpdate()
{
    if (!m_waveBuilder) {
        return;
    }

    float builderPos;
    int builderIndex;
    m_waveBuilder->positionAndIndex(builderPos, builderIndex);

    if (m_oldSize != size() || m_oldBuilderIndex != builderIndex) {
        m_needsUpdate = true;
    }

    if ((builderPos != 0.0 && builderPos != 1.0) && m_timer->interval() != FAST_INTERVAL) {
        m_timer->setInterval(FAST_INTERVAL);
    } else if ((builderPos == 0.0 || builderPos == 1.0) && m_timer->interval() != IDLE_INTERVAL) {
        m_timer->setInterval(IDLE_INTERVAL);
    }

    if (m_needsUpdate) {
        QPainter painter;
        qreal dpr = devicePixelRatioF();
        QImage image(width() * dpr, height() * dpr, QImage::Format_ARGB32_Premultiplied);
        image.setDevicePixelRatio(dpr);
        QImage waveImage;
        waveImage = m_backgroundImage = m_progressPlayingImage = m_progressPausedImage =
            m_remainingPlayingImage = m_remainingPausedImage = image;

        m_oldBuilderPos = builderPos;
        m_oldBuilderIndex = builderIndex;
        m_oldSize = size();

        // waveform >>
        waveImage.fill(0);
        painter.begin(&waveImage);
        painter.setBrush(m_waveBackground);
        QPen wavePen;
        wavePen.setWidth(0);
        wavePen.setColor(m_waveBorderColor);
        painter.setPen(wavePen);

        QPainterPath pathPos;
        QPainterPath pathNeg;
        NWaveformPeaks *peaks = m_waveBuilder->peaks();
        for (int i = 0; i < m_oldBuilderIndex; ++i) {
            pathPos.lineTo((i / (qreal)m_oldBuilderIndex) * m_oldBuilderPos * width(),
                           (1 + peaks->positive(i)) * (height() / 2.0));
            pathNeg.lineTo((i / (qreal)m_oldBuilderIndex) * m_oldBuilderPos * width(),
                           (1 + peaks->negative(i)) * (height() / 2.0));
        }
        QPainterPath fullPath(pathNeg);
        fullPath.connectPath(pathPos.toReversed());
        fullPath.closeSubpath();
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawPath(fullPath);
        painter.end();
        // << waveform

        // main background >>
        painter.begin(&m_backgroundImage);
        m_backgroundImage.fill(0);
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_background);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.drawRoundedRect(rect(), m_radius, m_radius);
        painter.end();
        // << main background

        QList<QImage *> images;
        images << &m_progressPlayingImage << &m_progressPausedImage << &m_remainingPlayingImage
               << &m_remainingPausedImage;
        QList<QPainter::CompositionMode> modes;
        modes << m_playingComposition << m_pausedComposition << m_playingComposition
              << m_pausedComposition;
        QList<QBrush> brushes;
        brushes << m_progressPlayingBackground << m_progressPausedBackground
                << m_remainingPlayingBackground << m_remainingPausedBackground;
        for (int i = 0; i < images.size(); ++i) {
            painter.begin(images[i]);
            images[i]->fill(0);
            painter.setPen(Qt::NoPen);
            painter.setBrush(brushes[i]);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.drawRect(rect());
            painter.setCompositionMode(modes[i]);
            painter.drawImage(0, 0, waveImage);
            painter.setCompositionMode(QPainter::CompositionMode_DestinationAtop);
            painter.drawImage(0, 0, m_backgroundImage);
            painter.end();
        }

        update();
        m_needsUpdate = false;
    }
}

void NWaveformSlider::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    if (m_hasMedia) {
        qreal x = (qreal)value() / maximum() * width();
        qreal dpr = devicePixelRatioF();
        QTransform transform;
        transform.scale(dpr, dpr);

        QRectF left = QRectF(0, 0, x, height());
        painter.drawImage(left, m_pausedState ? m_progressPausedImage : m_progressPlayingImage,
                          transform.mapRect(left));

        QRectF right = QRectF(x, 0, width() - x, height());
        painter.drawImage(right, m_pausedState ? m_remainingPausedImage : m_remainingPlayingImage,
                          transform.mapRect(right));

        QColor grooveColor = m_pausedState ? m_groovePausedColor : m_groovePlayingColor;
        if (grooveColor != Qt::transparent) {
            painter.setPen(grooveColor);
            painter.drawLine(QLineF(x, 0, x, height()));
        }
    } else {
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_background);
        painter.drawRoundedRect(rect(), m_radius, m_radius);
    }

    if (m_fileDrop) {
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(m_fileDropBorderColor);
        painter.setBrush(m_fileDropBackground);
        painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), m_radius - 1,
                                m_radius - 1);
    }
}

void NWaveformSlider::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        return;
    }

    qreal value = (qreal)event->x() / width();

    emit sliderMoved(value);
    setValue(value);
}

void NWaveformSlider::wheelEvent(QWheelEvent *event)
{
    event->ignore();
}

void NWaveformSlider::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::StyleChange) {
        m_needsUpdate = true;
    }
    QWidget::changeEvent(event);
}

void NWaveformSlider::setValue(qreal value)
{
    QAbstractSlider::setValue(value * maximum());
}

void NWaveformSlider::setMedia(const QString &file)
{
    init();

    if (file.isEmpty() || !QFile(file).exists()) {
        m_hasMedia = false;
        return;
    }

    m_hasMedia = true;
    m_waveBuilder->start(file);
}

// STYLESHEET PROPERTIES >>
int NWaveformSlider::radius() const
{
    return m_radius;
}

void NWaveformSlider::setRadius(int radius)
{
    m_radius = radius;
}

QBrush NWaveformSlider::background() const
{
    return m_background;
}

void NWaveformSlider::setBackground(QBrush brush)
{
    m_background = brush;
}

QBrush NWaveformSlider::waveBackground() const
{
    return m_waveBackground;
}

void NWaveformSlider::setWaveBackground(QBrush brush)
{
    m_waveBackground = brush;
}

QColor NWaveformSlider::waveBorderColor() const
{
    return m_waveBorderColor;
}

void NWaveformSlider::setWaveBorderColor(QColor color)
{
    m_waveBorderColor = color;
}

QBrush NWaveformSlider::progressPlayingBackground() const
{
    return m_progressPlayingBackground;
}

void NWaveformSlider::setProgressPlayingBackground(QBrush brush)
{
    m_progressPlayingBackground = brush;
}

QBrush NWaveformSlider::progressPausedBackground() const
{
    return m_progressPausedBackground;
}

void NWaveformSlider::setProgressPausedBackground(QBrush brush)
{
    m_progressPausedBackground = brush;
}

QBrush NWaveformSlider::remainingPlayingBackground() const
{
    return m_remainingPlayingBackground;
}

void NWaveformSlider::setRemainingPlayingBackground(QBrush brush)
{
    m_remainingPlayingBackground = brush;
}

QBrush NWaveformSlider::remainingPausedBackground() const
{
    return m_remainingPausedBackground;
}

void NWaveformSlider::setRemainingPausedBackground(QBrush brush)
{
    m_remainingPausedBackground = brush;
}

QColor NWaveformSlider::groovePlayingColor() const
{
    return m_groovePlayingColor;
}

void NWaveformSlider::setGroovePlayingColor(QColor color)
{
    m_groovePlayingColor = color;
}

QColor NWaveformSlider::groovePausedColor() const
{
    return m_groovePausedColor;
}

void NWaveformSlider::setGroovePausedColor(QColor color)
{
    m_groovePausedColor = color;
}

QString NWaveformSlider::playingComposition() const
{
    return ENUM_TO_STR(N, CompositionMode, m_playingComposition);
}

void NWaveformSlider::setPlayingComposition(const QString &mode)
{
    m_playingComposition = (QPainter::CompositionMode)STR_TO_ENUM(N, CompositionMode,
                                                                  mode.toLatin1());
}

QString NWaveformSlider::pausedComposition() const
{
    return ENUM_TO_STR(N, CompositionMode, m_pausedComposition);
}

void NWaveformSlider::setPausedComposition(const QString &mode)
{
    m_pausedComposition = (QPainter::CompositionMode)STR_TO_ENUM(N, CompositionMode,
                                                                 mode.toLatin1());
}

QColor NWaveformSlider::fileDropBorderColor() const
{
    return m_fileDropBorderColor;
}

void NWaveformSlider::setFileDropBorderColor(QColor color)
{
    m_fileDropBorderColor = color;
}

QBrush NWaveformSlider::fileDropBackground() const
{
    return m_fileDropBackground;
}

void NWaveformSlider::setFileDropBackground(QBrush brush)
{
    m_fileDropBackground = brush;
}
// << STYLESHEET PROPERTIES

// DRAG & DROP >>
void NWaveformSlider::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData() && event->mimeData()->hasUrls() && !event->mimeData()->urls().isEmpty()) {
        event->acceptProposedAction();
        m_fileDrop = true;
        update();
    } else {
        event->ignore();
    }
}

void NWaveformSlider::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void NWaveformSlider::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
    m_fileDrop = false;
    update();
}

void NWaveformSlider::dropEvent(QDropEvent *event)
{
    const QMimeData *data = event->mimeData();
    if (data->hasUrls()) {
        QList<NPlaylistDataItem> dataItems;
        foreach (QUrl url, data->urls()) {
            dataItems << NUtils::dirListRecursive(url.toLocalFile());
        }
        emit filesDropped(dataItems);
    }

    event->acceptProposedAction();

    m_fileDrop = false;
    update();
}

QStringList NWaveformSlider::mimeTypes() const
{
    QStringList qstrList;
    qstrList.append("text/uri-list");
    return qstrList;
}
// << DRAG & DROP
