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

#ifndef N_WAVEFORM_SLIDER_H
#define N_WAVEFORM_SLIDER_H

#include <QAbstractSlider>
#include <QVector>
#include <QPainter>

class NWaveformBuilderInterface;

class NWaveformSlider : public QAbstractSlider
{
	Q_OBJECT
	Q_PROPERTY(int radius READ radius WRITE setRadius)
	Q_PROPERTY(QBrush  background                 READ background WRITE setBackground)
	Q_PROPERTY(QBrush  wave_background            READ waveBackground  WRITE setWaveBackground)
	Q_PROPERTY(QColor  wave_border_color          READ waveBorderColor WRITE setWaveBorderColor)
	Q_PROPERTY(QBrush  progress_normal_background READ progressNormalBackground WRITE setProgressNormalBackground)
	Q_PROPERTY(QBrush  progress_paused_background READ progressPausedBackground WRITE setProgressPausedBackground)
	Q_PROPERTY(QString normal_composition         READ normalComposition WRITE setNormalComposition)
	Q_PROPERTY(QString paused_composition         READ pausedComposition WRITE setPausedComposition)

private:
	NWaveformBuilderInterface *m_waveBuilder;
	QImage m_waveImage;
	QVector<QImage> m_bufImage;
	QTimer *m_timer;
	bool m_pausedState;

	QSize m_oldSize;
	int m_oldValue;
	int m_oldIndex;
	float m_oldBuildPos;

	void mousePressEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);
	void paintEvent(QPaintEvent *event);
	void changeEvent(QEvent *event);
	void init();

public:
	NWaveformSlider(QWidget *parent = 0);
	~NWaveformSlider();
	QSize sizeHint() const;

public slots:
	void drawFile(const QString &file);
	void setPausedState(bool);
	void setValue(qreal value);

private slots:
	void checkForUpdate();
	void setValue(int) {};

signals:
	void sliderMoved(qreal value);

// STYLESHEET PROPERTIES
private:
	int m_radius;
	QBrush m_background;
	QBrush m_waveBackground;
	QColor m_waveBorderColor;
	QBrush m_progressNormalBackground;
	QBrush m_progressPausedBackground;
	QPainter::CompositionMode m_normalComposition;
	QPainter::CompositionMode m_pausedComposition;
	bool m_needsUpdate;

public:
	int radius();
	void setRadius(int radius);

	QBrush background();
	void setBackground(QBrush brush);

	QBrush waveBackground();
	void setWaveBackground(QBrush brush);

	QColor waveBorderColor();
	void setWaveBorderColor(QColor color);

	QBrush progressNormalBackground();
	void setProgressNormalBackground(QBrush brush);

	QBrush progressPausedBackground();
	void setProgressPausedBackground(QBrush brush);

	QString normalComposition();
	void setNormalComposition(const QString &mode);

	QString pausedComposition();
	void setPausedComposition(const QString &mode);
};

#endif

