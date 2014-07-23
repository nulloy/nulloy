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

#include <QLabel>
#include <QLayout>
#include <QTime>
#include <QToolTip>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QMouseEvent>

NTrackInfoWidget::~NTrackInfoWidget() {}

NTrackInfoWidget::NTrackInfoWidget(QWidget *parent) : QWidget(parent)
{
	QStringList vNames = QStringList() << "Top" << "Middle" << "Bottom";
	QStringList hNames = QStringList() << "Left" << "Center" << "Right";
	QVBoxLayout *vLayout = new QVBoxLayout;
	for (int i = 0; i < vNames.count(); ++i) {
		QHBoxLayout *hLayout = new QHBoxLayout;
		if (i > 0) {
			QSpacerItem *vSpacer = new QSpacerItem(20, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
			vLayout->addItem(vSpacer);
		}
		for (int j = 0; j < hNames.count(); ++j) {
			if (j > 0) {
				QSpacerItem *hSpacer = new QSpacerItem(40, 14, QSizePolicy::Expanding, QSizePolicy::Minimum);
				hLayout->addItem(hSpacer);
			}
			QLabel *label = new QLabel;
			label->setObjectName(vNames.at(i) + hNames.at(j));
			hLayout->addWidget(label);
		}
		vLayout->addLayout(hLayout);
	}
	setLayout(vLayout);
	vLayout->setContentsMargins(2, 2, 2, 2);

	setMouseTracking(TRUE);
	QList<QLabel *> labels = findChildren<QLabel *>();
	foreach (QLabel *label, labels)
		label->setAttribute(Qt::WA_TransparentForMouseEvents);

	m_effect = new QGraphicsOpacityEffect(this);
	m_effect->setOpacity(1.0);
	setGraphicsEffect(m_effect);

	m_animation = new QPropertyAnimation(m_effect, "opacity", this);
	m_animation->setDuration(100);
	m_animation->setEasingCurve(QEasingCurve::OutQuad);
	m_animation->setStartValue(1.0);
	m_animation->setEndValue(0.0);

	m_msec = 0;

	readSettings();
	updateInfo();
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

bool NTrackInfoWidget::event(QEvent *event)
{
	if (event->type() == QEvent::ToolTip)
		QToolTip::hideText();

	return QWidget::event(event);
}

void NTrackInfoWidget::mouseMoveEvent(QMouseEvent *event)
{
	showToolTip(event->x(), event->y());
}

void NTrackInfoWidget::updateInfo()
{
	NTagReaderInterface *tagReader = dynamic_cast<NTagReaderInterface *>(NPluginLoader::getPlugin(N::TagReader));
	if (!tagReader->isValid()) {
		hide();
	} else {
		show();
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

	m_tooltipFormat = NSettings::instance()->value("TooltipTrackInfo").toString();
}

void NTrackInfoWidget::tick(qint64 msec)
{
	m_msec = msec;

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

void NTrackInfoWidget::showToolTip(int x, int y)
{
	if (!rect().contains(QPoint(x, y)) || m_tooltipFormat.isEmpty()) {
		QToolTip::hideText();
		return;
	}

	NTagReaderInterface *tagReader = dynamic_cast<NTagReaderInterface *>(NPluginLoader::getPlugin(N::TagReader));
	QString text = m_tooltipFormat;

	if (m_tooltipFormat.contains("%C") || m_tooltipFormat.contains("%a")) {
		int durationSec = tagReader->toString("%D").toInt();
		float posAtX = (float)x / width();
		int secAtX = durationSec * posAtX;
		QTime timeAtX = QTime().addSecs(secAtX);
		QString strAtPos;
		if (secAtX > 60 * 60) // has hours
			strAtPos = timeAtX.toString("h:mm:ss");
		else
			strAtPos = timeAtX.toString("m:ss");
		text.replace("%C", strAtPos);

		int secCur = m_msec / 1000;
		int secDiff = secAtX - secCur;
		QTime timeDiff = QTime().addSecs(qAbs(secDiff));
		QString diffStr;
		if (qAbs(secDiff) > 60 * 60) // has hours
			diffStr = timeDiff.toString("h:mm:ss");
		else
			diffStr = timeDiff.toString("m:ss");
		text.replace("%o", QString("%1%2").arg(secDiff < 0 ? "-" : "+").arg(diffStr));
	}

	text = tagReader->toString(text);
	QToolTip::showText(mapToGlobal(QPoint(x, y)), text);
}

