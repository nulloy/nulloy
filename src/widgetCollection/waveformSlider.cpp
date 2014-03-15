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

#include <QMouseEvent>
#include <QPainter>
#include <QFile>
#include <QStylePainter>
#include <QStyleOptionFocusRect>

#ifndef _N_NO_PLUGINS_
#include "pluginLoader.h"
#else
#include "waveformBuilderGstreamer.h"
#endif

NWaveformSlider::NWaveformSlider(QWidget *parent) : QAbstractSlider(parent)
{
	m_radius = 0;
	m_background = QBrush(Qt::darkBlue);
	m_waveBackground = QBrush(Qt::darkGreen);
	m_waveBorderColor = QColor(Qt::green);
	m_progressBackground = QBrush(Qt::darkCyan);
	m_pausedBackground = QBrush(Qt::darkGray);

	setMouseTracking(TRUE);

#ifndef _N_NO_PLUGINS_
	m_waveBuilder = dynamic_cast<NWaveformBuilderInterface *>(NPluginLoader::getPlugin(N::WaveformBuilder));
#else
	NWaveformBuilderInterface *builder = dynamic_cast<NWaveformBuilderInterface *>(new NWaveformBuilderGstreamer());
	dynamic_cast<NPlugin *>(builder)->init();
	m_waveBuilder = builder;
#endif

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

	bool needsUpdate = FALSE;
	if (m_oldValue != value() || m_oldSize != size())
		needsUpdate = TRUE;

	float pos;
	int index;
	m_waveBuilder->positionAndIndex(pos, index);

	if (!(pos < 0 || index < 0) &&
	    !(m_oldBuildPos == pos &&
	    m_oldIndex == index &&
	    m_waveImage.size() == size()))
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

		needsUpdate = TRUE;
	}

	if (needsUpdate)
		update();
}

void NWaveformSlider::mouseMoveEvent(QMouseEvent *event)
{
	emit mouseMoved(event->x(), event->y());
}

void NWaveformSlider::leaveEvent(QEvent *event)
{
	Q_UNUSED(event);
	emit mouseMoved(-1, -1);
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
		if (!m_pausedState)
			painter.setBrush(m_progressBackground);
		else
			painter.setBrush(m_pausedBackground);
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
		painter.setCompositionMode(QPainter::CompositionMode_Overlay);
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

	int value = qreal(event->x()) / width() * maximum();

	emit sliderMoved(value);
//	emit valueChanged(value);
	setValue(value);
}

void NWaveformSlider::wheelEvent(QWheelEvent *event)
{
	event->accept();
}

void NWaveformSlider::setValue(int value)
{
	QAbstractSlider::setValue(value);

//	update();
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

int NWaveformSlider::getRadius()
{
	return m_radius;
}

void NWaveformSlider::setRadius(int radius)
{
	m_radius = radius;
}

QBrush NWaveformSlider::getBackground()
{
	return m_background;
}

void NWaveformSlider::setBackground(QBrush brush)
{
	m_background = brush;
}

QBrush NWaveformSlider::getWaveBackground()
{
	return m_waveBackground;
}

void NWaveformSlider::setWaveBackground(QBrush brush)
{
	m_waveBackground = brush;
}

QColor NWaveformSlider::getWaveBorderColor()
{
	return m_waveBorderColor;
}

void NWaveformSlider::setWaveBorderColor(QColor color)
{
	m_waveBorderColor = color;
}

QBrush NWaveformSlider::getProgressBackground()
{
	return m_progressBackground;
}

void NWaveformSlider::setProgressBackground(QBrush brush)
{
	m_progressBackground = brush;
}

QBrush NWaveformSlider::getPausedBackground()
{
	return m_pausedBackground;
}

void NWaveformSlider::setPausedBackground(QBrush brush)
{
	m_pausedBackground = brush;
}

