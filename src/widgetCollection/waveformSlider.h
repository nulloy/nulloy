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

#ifndef N_WAVEFORM_SLIDER_H
#define N_WAVEFORM_SLIDER_H

#include "waveformBuilderInterface.h"
#include <QMouseEvent>
#include <QAbstractSlider>
#include <QTimer>

class NWaveformSlider : public QAbstractSlider
{
	Q_OBJECT

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
	void reset();

public:
	NWaveformSlider(QWidget *parent = 0);
	~NWaveformSlider();

	void setBuilder(NWaveformBuilderInterface *builder);

public slots:
	void drawFile(const QString &file);
	void setValue(int value);
	void setPausedState(bool);

private slots:
	void checkForUpdate();
};

#endif

/* vim: set ts=4 sw=4: */
