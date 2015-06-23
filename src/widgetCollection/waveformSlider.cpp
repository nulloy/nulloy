/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2015 Sergey Vlasov <sergey@vlasov.me>
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
#include "settings.h"
#include "common.h"

#include <QMouseEvent>
#include <QFile>
#include <QStylePainter>
#include <QStyleOptionFocusRect>

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

	m_waveBuilder = dynamic_cast<NWaveformBuilderInterface *>(NPluginLoader::getPlugin(N::WaveformBuilder));

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
	if (!m_waveBuilder)
		return;

	float builderPos;
	int builderIndex;
	m_waveBuilder->positionAndIndex(builderPos, builderIndex);

	if (m_oldSize != size() || m_oldBuilderIndex != builderIndex)
		m_needsUpdate = true;

	if ((builderPos != 0.0 && builderPos != 1.0) && m_timer->interval() != FAST_INTERVAL)
		m_timer->setInterval(FAST_INTERVAL);
	else
	if ((builderPos == 0.0 || builderPos == 1.0) && m_timer->interval() != IDLE_INTERVAL)
		m_timer->setInterval(IDLE_INTERVAL);

	if (m_needsUpdate) {
		QPainter painter;
		QImage waveImage;
		QImage backgroundImage;
		waveImage = backgroundImage = m_progressPlayingImage = m_progressPausedImage = m_remainingPlayingImage = m_remainingPausedImage = QImage(size(), QImage::Format_ARGB32_Premultiplied);

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

		painter.translate(1, 1);
		painter.scale((qreal)(width() - 1) / m_oldBuilderIndex * m_oldBuilderPos, (height() - 2) / 2);
		painter.translate(0, 1);

		QPainterPath pathPos;
		QPainterPath pathNeg;
		NWaveformPeaks *peaks = m_waveBuilder->peaks();
		for (int i = 0; i < m_oldBuilderIndex; ++i) {
			pathPos.lineTo(i, peaks->positive(i));
			pathNeg.lineTo(i, peaks->negative(i));
		}
		QPainterPath fullPath(pathNeg);
		fullPath.connectPath(pathPos.toReversed());
		fullPath.closeSubpath();
		painter.setRenderHint(QPainter::Antialiasing);
		painter.drawPath(fullPath);
		painter.end();
		// << waveform


		// main background >>
		painter.begin(&backgroundImage);
		backgroundImage.fill(0);
		painter.setPen(Qt::NoPen);
		painter.setBrush(m_background);
		painter.setRenderHint(QPainter::Antialiasing);
		painter.drawRoundedRect(rect(), m_radius, m_radius);
		painter.end();
		// << main background


		QList<QImage *> images; images << &m_progressPlayingImage << &m_progressPausedImage << &m_remainingPlayingImage << &m_remainingPausedImage;
		QList<QPainter::CompositionMode> modes; modes << m_playingComposition << m_pausedComposition << m_playingComposition << m_pausedComposition;
		QList<QBrush> brushes; brushes << m_progressPlayingBackground << m_progressPausedBackground << m_remainingPlayingBackground << m_remainingPausedBackground;
		for (int i = 0; i < images.size(); ++i) {
			painter.begin(images[i]);
			// background
			images[i]->fill(0);
			// + overlay
			painter.setPen(Qt::NoPen);
			painter.setBrush(brushes[i]);
			painter.setRenderHint(QPainter::Antialiasing);
			painter.drawRoundedRect(rect(), m_radius, m_radius);
			// + waveform
			painter.setCompositionMode(modes[i]);
			painter.drawImage(0, 0, waveImage);
			painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
			painter.drawImage(0, 0, backgroundImage);
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
		int x = qRound((qreal)value() / maximum() * width());

		QRect left = rect().adjusted(0, 0, x - width(), 0);
		painter.drawImage(left, m_pausedState ? m_progressPausedImage : m_progressPlayingImage, left);

		QRect right = rect().adjusted(x, 0, 0, 0);
		painter.drawImage(right, m_pausedState ? m_remainingPausedImage : m_remainingPlayingImage, right);

		QColor grooveColor = m_pausedState ? m_groovePausedColor : m_groovePlayingColor;
		if (grooveColor != Qt::transparent) {
			painter.setPen(grooveColor);
			painter.drawLine(x, 0, x, height());
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
		painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), m_radius - 1, m_radius - 1);
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
	event->ignore();
}

void NWaveformSlider::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::StyleChange)
		m_needsUpdate = true;
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

QBrush NWaveformSlider::progressPlayingBackground()
{
	return m_progressPlayingBackground;
}

void NWaveformSlider::setProgressPlayingBackground(QBrush brush)
{
	m_progressPlayingBackground = brush;
}

QBrush NWaveformSlider::progressPausedBackground()
{
	return m_progressPausedBackground;
}

void NWaveformSlider::setProgressPausedBackground(QBrush brush)
{
	m_progressPausedBackground = brush;
}

QBrush NWaveformSlider::remainingPlayingBackground()
{
	return m_remainingPlayingBackground;
}

void NWaveformSlider::setRemainingPlayingBackground(QBrush brush)
{
	m_remainingPlayingBackground = brush;
}

QBrush NWaveformSlider::remainingPausedBackground()
{
	return m_remainingPausedBackground;
}

void NWaveformSlider::setRemainingPausedBackground(QBrush brush)
{
	m_remainingPausedBackground = brush;
}

QColor NWaveformSlider::groovePlayingColor()
{
	return m_groovePlayingColor;
}

void NWaveformSlider::setGroovePlayingColor(QColor color)
{
	m_groovePlayingColor = color;
}

QColor NWaveformSlider::groovePausedColor()
{
	return m_groovePausedColor;
}

void NWaveformSlider::setGroovePausedColor(QColor color)
{
	m_groovePausedColor = color;
}

QString NWaveformSlider::playingComposition()
{
	return ENUM_TO_STR(N, CompositionMode, m_playingComposition);
}

void NWaveformSlider::setPlayingComposition(const QString &mode)
{
	m_playingComposition = (QPainter::CompositionMode)STR_TO_ENUM(N, CompositionMode, mode.toAscii());
}

QString NWaveformSlider::pausedComposition()
{
	return ENUM_TO_STR(N, CompositionMode, m_pausedComposition);
}

void NWaveformSlider::setPausedComposition(const QString &mode)
{
	m_pausedComposition = (QPainter::CompositionMode)STR_TO_ENUM(N, CompositionMode, mode.toAscii());
}

QColor NWaveformSlider::fileDropBorderColor()
{
	return m_fileDropBorderColor;
}

void NWaveformSlider::setFileDropBorderColor(QColor color)
{
	m_fileDropBorderColor = color;
}

QBrush NWaveformSlider::fileDropBackground()
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
		QStringList files;
		foreach (QUrl url, data->urls())
			files << NCore::dirListRecursive(url.toLocalFile(), NSettings::instance()->value("FileFilters").toString().split(' '));
		emit filesDropped(files);
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
