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

#include "waveformSlider.h"
#include "waveformBuilderInterface.h"
#include "pluginLoader.h"

#include <QMouseEvent>
#include <QFile>
#include <QStylePainter>
#include <QStyleOptionFocusRect>

NWaveformSlider::NWaveformSlider(QWidget *parent) : QAbstractSlider(parent)
{
	m_radius = 0;
	m_background = QBrush(Qt::darkBlue);
	m_waveBackground = QBrush(Qt::darkGreen);
	m_waveBorderColor = QColor(Qt::green);
	m_progressNormalBackground = QBrush(Qt::darkCyan);
	m_progressPausedBackground = QBrush(Qt::darkGray);
	m_normalComposition = QPainter::CompositionMode_Overlay;
	m_pausedComposition = QPainter::CompositionMode_Overlay;

	m_waveBuilder = dynamic_cast<NWaveformBuilderInterface *>(NPluginLoader::getPlugin(N::WaveformBuilder));

	m_bufImage.resize(7);

	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(checkForUpdate()));
	m_timer->start(50);

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
	m_oldValue = -1;
	m_oldIndex = -1;
	m_oldBuildPos = -1;
	m_pausedState = FALSE;
	m_needsUpdate = FALSE;
	setEnabled(FALSE);

	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	setMinimumHeight(50);
	setMinimumWidth(150);
}

QSize NWaveformSlider::sizeHint() const
{
	return QSize(200, 80);
}

NWaveformSlider::~NWaveformSlider() {}

void NWaveformSlider::checkForUpdate()
{
	if (!m_waveBuilder)
		return;

	if (m_oldValue != value() || m_oldSize != size())
		m_needsUpdate = TRUE;

	float pos;
	int index;
	m_waveBuilder->positionAndIndex(pos, index);

	if ((!(pos < 0 || index < 0) &&
	     !(m_oldBuildPos == pos &&
	     m_oldIndex == index &&
	     m_waveImage.size() == size())) ||
	    m_needsUpdate)
	{
		m_oldBuildPos = pos;
		m_oldIndex = index;

		if (m_waveImage.size() != size())
			m_waveImage = QImage(size(), QImage::Format_ARGB32_Premultiplied);

		QPainter painter(&m_waveImage);
		m_waveImage.fill(0);

		painter.setRenderHint(QPainter::Antialiasing);
		painter.setBrush(m_waveBackground);
		QPen wavePen;
		wavePen.setWidth(0);
		wavePen.setColor(m_waveBorderColor);
		painter.setPen(wavePen);

		painter.translate(1, 1);
		painter.scale((qreal)(width() - 1) / m_oldIndex * m_oldBuildPos, (height() - 2) / 2);
		painter.translate(0, 1);

		QPainterPath pathPos;
		QPainterPath pathNeg;
		NWaveformPeaks *peaks = m_waveBuilder->peaks();
		for (int i = 0; i < m_oldIndex; ++i) {
			pathPos.lineTo(i, peaks->positive(i));
			pathNeg.lineTo(i, peaks->negative(i));
		}
		QPainterPath fullPath(pathNeg);
		fullPath.connectPath(pathPos.toReversed());
		fullPath.closeSubpath();
		painter.drawPath(fullPath);
	}

	if (m_needsUpdate)
		update();
	m_needsUpdate = FALSE;
}

void NWaveformSlider::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	if (m_oldSize != size()) {
		for (int i = 0; i < m_bufImage.size(); ++i)
			m_bufImage[i] = QImage(size(), QImage::Format_ARGB32_Premultiplied);
	}

	m_oldSize = size();
	m_oldValue = value();

	for (int i = 0; i < m_bufImage.size(); ++i)
		m_bufImage[i].fill(0);

	QPen pen;
	QBrush brush;
	QPainter painter;

	painter.begin(&m_bufImage[2]);
	painter.setRenderHint(QPainter::Antialiasing);
	// main bg
	painter.setPen(Qt::NoPen);
	painter.setBrush(m_background);
	painter.drawRoundedRect(rect(), m_radius, m_radius);
	painter.end();

	int x = qRound((qreal)m_oldValue / maximum() * width());

	if (isEnabled()) {
		painter.begin(&m_bufImage[3]);
		// progress rectangle
		painter.setPen(Qt::NoPen);
		QPainter::CompositionMode progressComposition;
		if (!m_pausedState) {
			painter.setBrush(m_progressNormalBackground);
			progressComposition = m_normalComposition;
		} else {
			painter.setBrush(m_progressPausedBackground);
			progressComposition = m_pausedComposition;
		}
		painter.drawRect(rect().adjusted(0, 0, x - width(), 0));
		painter.end();

		painter.begin(&m_bufImage[0]);
		painter.drawImage(0, 0, m_bufImage[2]);
		painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
		painter.drawImage(0, 0, m_bufImage[3]);
		painter.end();

		painter.begin(&m_bufImage[1]);
		painter.drawImage(0, 0, m_bufImage[2]);
		painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
		painter.drawImage(0, 0, m_bufImage[3]);
		painter.end();

		painter.begin(&m_bufImage[4]);
		painter.drawImage(0, 0, m_bufImage[0]);
		painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
		painter.drawImage(0, 0, m_waveImage);
		painter.end();

		painter.begin(&m_bufImage[5]);
		painter.drawImage(0, 0, m_bufImage[1]);
		painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
		painter.drawImage(0, 0, m_waveImage);
		painter.end();

		painter.begin(&m_bufImage[6]);
		painter.drawImage(0, 0, m_bufImage[0]);
		painter.drawImage(0, 0, m_bufImage[1]);
		painter.drawImage(0, 0, m_bufImage[5]);
		painter.setCompositionMode(progressComposition);
		painter.drawImage(0, 0, m_bufImage[4]);

		// progress line
		/*painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
		QColor main_border = QColor("#000000");
		main_border.setAlpha(200);
		painter.setPen(main_border);
		painter.drawLine(x, 0, x, height());*/
		painter.end();

		painter.begin(this);
		painter.drawImage(0, 0, m_bufImage[6]);
		// main border
		/*painter.setRenderHint(QPainter::Antialiasing);
		painter.setBrush(Qt::NoBrush);
		main_border.setAlpha(255);
		painter.setPen(main_border);
		painter.drawRoundedRect(rect(), 6, 6);*/
		painter.end();
	} else {
		painter.begin(this);
		painter.drawImage(0, 0, m_bufImage[2]);
		painter.end();
	}
}

void NWaveformSlider::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton)
		return;

	qreal value = (qreal)event->x() / width();

	emit sliderMoved(value);
	setValue(value);
}

void NWaveformSlider::wheelEvent(QWheelEvent *event)
{
	event->accept();
}

void NWaveformSlider::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::StyleChange)
		m_needsUpdate = TRUE;
	QWidget::changeEvent(event);
}

void NWaveformSlider::setValue(qreal value)
{
	QAbstractSlider::setValue(value * maximum());
}

void NWaveformSlider::drawFile(const QString &file)
{
	init();

	if (!QFile(file).exists())
		return;

	m_waveBuilder->start(file);
	setEnabled(TRUE);
}

// STYLESHEET PROPERTIES

int NWaveformSlider::radius()
{
	return m_radius;
}

void NWaveformSlider::setRadius(int radius)
{
	m_radius = radius;
}

QBrush NWaveformSlider::background()
{
	return m_background;
}

void NWaveformSlider::setBackground(QBrush brush)
{
	m_background = brush;
}

QBrush NWaveformSlider::waveBackground()
{
	return m_waveBackground;
}

void NWaveformSlider::setWaveBackground(QBrush brush)
{
	m_waveBackground = brush;
}

QColor NWaveformSlider::waveBorderColor()
{
	return m_waveBorderColor;
}

void NWaveformSlider::setWaveBorderColor(QColor color)
{
	m_waveBorderColor = color;
}

QBrush NWaveformSlider::progressNormalBackground()
{
	return m_progressNormalBackground;
}

void NWaveformSlider::setProgressNormalBackground(QBrush brush)
{
	m_progressNormalBackground = brush;
}

QBrush NWaveformSlider::progressPausedBackground()
{
	return m_progressPausedBackground;
}

void NWaveformSlider::setProgressPausedBackground(QBrush brush)
{
	m_progressPausedBackground = brush;
}

QString NWaveformSlider::normalComposition()
{
	return ENUM_TO_STR(N, CompositionMode, m_normalComposition);
}

void NWaveformSlider::setNormalComposition(const QString &mode)
{
	m_normalComposition = (QPainter::CompositionMode)STR_TO_ENUM(N, CompositionMode, mode.toAscii());
}

QString NWaveformSlider::pausedComposition()
{
	return ENUM_TO_STR(N, CompositionMode, m_pausedComposition);
}

void NWaveformSlider::setPausedComposition(const QString &mode)
{
	m_pausedComposition = (QPainter::CompositionMode)STR_TO_ENUM(N, CompositionMode, mode.toAscii());
}
