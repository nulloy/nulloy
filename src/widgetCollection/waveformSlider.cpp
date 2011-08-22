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

#include "waveformSlider.h"

#include <QPainter>
#include <QFile>
#include <QDebug>

QColor main_bg =		QColor("#3f4f61");
QColor main_border =	QColor("#000000");

QColor wave_bg =		QColor("#4e9a06");
QColor wave_border =	QColor("#8ae234");

QColor progress_bg =	QColor("#0080ff");
QColor paused_bg =		QColor("#ce8419");

NWaveformSlider::NWaveformSlider(QWidget *parent) : QAbstractSlider(parent)
{
	m_waveBuilder = NULL;
	m_bufImage.resize(7);

	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	setMinimumHeight(50);

	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(checkForUpdate()));
	m_timer->start(50);

	m_oldSize = QSize(0, 0);
	reset();
}

void NWaveformSlider::setBuilder(NWaveformBuilderInterface *builder)
{
	m_waveBuilder = builder;
	m_waveBuilder->setParent(this);
}

void NWaveformSlider::setPausedState(bool state)
{
	m_pausedState = state;
	update();
}

void NWaveformSlider::reset()
{
	m_oldValue = -1;
	m_oldIndex = -1;
	m_oldBuildPos = -1;
	m_pausedState = FALSE;
	setEnabled(FALSE);
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
		painter.setBrush(wave_bg);
		QPen wavePen;
		wavePen.setWidth(0);
		wavePen.setColor(wave_border);
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
	painter.setBrush(main_bg);
	painter.drawRoundedRect(rect(), 5, 5);
	painter.end();

	int x = qRound((qreal)m_oldValue / maximum() * width());

	if (isEnabled()) {
		painter.begin(&m_bufImage[3]);
		// progress rectangle
		painter.setPen(Qt::NoPen);
		if (!m_pausedState)
			painter.setBrush(progress_bg);
		else
			painter.setBrush(paused_bg);
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
	reset();

	if (!QFile(file).exists())
		return;

	m_waveBuilder->start(file);
	setEnabled(TRUE);
}

/* vim: set ts=4 sw=4: */
