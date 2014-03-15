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

class NWaveformBuilderInterface;

class NWaveformSlider : public QAbstractSlider
{
	Q_OBJECT
	Q_PROPERTY(int radius READ getRadius WRITE setRadius DESIGNABLE true)
	Q_PROPERTY(QBrush background READ getBackground WRITE setBackground DESIGNABLE true)
	Q_PROPERTY(QBrush waveBackground READ getWaveBackground WRITE setWaveBackground DESIGNABLE true)
	Q_PROPERTY(QColor waveBorderColor READ getWaveBorderColor WRITE setWaveBorderColor DESIGNABLE true)
	Q_PROPERTY(QBrush progressBackground READ getProgressBackground WRITE setProgressBackground DESIGNABLE true)
	Q_PROPERTY(QBrush pausedBackground READ getPausedBackground WRITE setPausedBackground DESIGNABLE true)

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
	void mouseMoveEvent(QMouseEvent *event);
	void leaveEvent(QEvent *event);
	void wheelEvent(QWheelEvent *event);
	void paintEvent(QPaintEvent *event);
	void init();

public:
	NWaveformSlider(QWidget *parent = 0);
	~NWaveformSlider();
	QSize sizeHint() const;

public slots:
	void drawFile(const QString &file);
	void setValue(int value);
	void setPausedState(bool);

signals:
	void mouseMoved(int x, int y);

private slots:
	void checkForUpdate();

// STYLESHEET PROPERTIES
private:
	int m_radius;
	QBrush m_background;
	QBrush m_waveBackground;
	QColor m_waveBorderColor;
	QBrush m_progressBackground;
	QBrush m_pausedBackground;

public:
	int getRadius();
	void setRadius(int radius);

	QBrush getBackground();
	void setBackground(QBrush brush);

	QBrush getWaveBackground();
	void setWaveBackground(QBrush brush);

	QColor getWaveBorderColor();
	void setWaveBorderColor(QColor color);

	QBrush getProgressBackground();
	void setProgressBackground(QBrush brush);

	QBrush getPausedBackground();
	void setPausedBackground(QBrush brush);
};

#endif

