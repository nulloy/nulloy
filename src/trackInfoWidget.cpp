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

#include "trackInfoWidget.h"

#include "tagReaderInterface.h"
#include "settings.h"
#include "pluginLoader.h"

#include <QTime>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

NTrackInfoWidget::~NTrackInfoWidget() {}

NTrackInfoWidget::NTrackInfoWidget(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);

	setMouseTracking(TRUE);

	m_effect = new QGraphicsOpacityEffect(this);
	m_effect->setOpacity(1.0);
	setGraphicsEffect(m_effect);

	m_animation = new QPropertyAnimation(m_effect, "opacity", this);
	m_animation->setDuration(100);
	m_animation->setEasingCurve(QEasingCurve::OutQuad);
	m_animation->setStartValue(1.0);
	m_animation->setEndValue(0.0);

	readSettings();
}

void NTrackInfoWidget::enterEvent(QEvent *)
{
	m_animation->setDirection(QAbstractAnimation::Forward);
	if (m_animation->state() == QAbstractAnimation::Stopped)
		m_animation->start();
}

void NTrackInfoWidget::leaveEvent(QEvent *)
{
	m_animation->setDirection(QAbstractAnimation::Backward);
	if (m_animation->state() == QAbstractAnimation::Stopped)
		m_animation->start();
}

void NTrackInfoWidget::updateInfo()
{
	NTagReaderInterface *tagReader = dynamic_cast<NTagReaderInterface *>(NPluginLoader::getPlugin(N::TagReader));
	if (!tagReader->isValid()) {
		QList<QLabel *> labels = findChildren<QLabel *>();
		foreach (QLabel *label, labels)
			label->hide();
	} else {
		foreach (QLabel *label, m_map.keys()) {
			QString info = tagReader->toString(m_map[label]);
			if (!info.isEmpty()) {
				label->setText(info);
				label->show();
			} else {
				label->hide();
			}
		}
	}
}

void NTrackInfoWidget::readSettings()
{
	QList<QLabel *> labels = findChildren<QLabel *>();
	for (int i = 0; i < labels.size(); ++i) {
		QString format = NSettings::instance()->value("TrackInfo/" + labels.at(i)->objectName()).toString();
		if (!format.isEmpty()) {
			if (!format.contains("%T") && !format.contains("%r"))
				m_map[labels.at(i)] = format;
			else
				m_mapTick[labels.at(i)] = format;
			labels.at(i)->show();
		} else {
			labels.at(i)->hide();
		}
	}
}

void NTrackInfoWidget::tick(qint64 msec)
{
	NTagReaderInterface *tagReader = dynamic_cast<NTagReaderInterface *>(NPluginLoader::getPlugin(N::TagReader));
	int total = tagReader->toString("%D").toInt();
	int hours = total / 60 / 60;
	QTime current = QTime().addMSecs(msec);
	QTime remaining = QTime().addMSecs(total * 1000 - msec);
	foreach (QLabel *label, m_mapTick.keys()) {
		QString text = m_mapTick[label];
		if (hours > 0) {
			text.replace("%T", current.toString("h:mm:ss"));
			text.replace("%r", remaining.toString("h:mm:ss"));
		} else {
			text.replace("%T", current.toString("m:ss"));
			text.replace("%r", remaining.toString("m:ss"));
		}
		if (text.contains("%"))
			text = tagReader->toString(text);
		label->setText(text);
		label->show();
	}
}
