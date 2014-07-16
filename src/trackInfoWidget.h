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

#ifndef N_TRACK_INFO_WIDGET_H
#define N_TRACK_INFO_WIDGET_H

#include <QWidget>
#include <QMap>

class QPropertyAnimation;
class QGraphicsOpacityEffect;
class QLabel;

class NTrackInfoWidget : public QWidget
{
	Q_OBJECT

private:
	QMap <QLabel *, QString> m_map;
	QMap <QLabel *, QString> m_mapTick;
	QGraphicsOpacityEffect *m_effect;
	QPropertyAnimation *m_animation;

	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);

public:
	NTrackInfoWidget(QWidget *parent = 0);
	~NTrackInfoWidget();

public slots:
	void updateInfo();
	void readSettings();
	void tick(qint64 msec);
};

#endif

