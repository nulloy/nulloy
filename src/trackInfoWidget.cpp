/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2013 Sergey Vlasov <sergey@vlasov.me>
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

#include "settings.h"
#include <QTime>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>

NTrackInfoWidget::~NTrackInfoWidget() {}

NTrackInfoWidget::NTrackInfoWidget(QWidget *parent) : QWidget(parent)
{
	m_container = new QWidget;
	ui.setupUi(m_container);

	m_container->setStyleSheet(m_container->styleSheet() + "#TrackInfoWidget { background: transparent }");
	m_container->layout()->setContentsMargins(2, 2, 2, 2);
	m_container->installEventFilter(this);

	m_scene = new QGraphicsScene;
	m_scene->installEventFilter(this);
	m_proxy = m_scene->addWidget(m_container);

	m_view = new QGraphicsView(this);
	m_view->setStyleSheet("border: 0 solid");
	m_view->viewport()->setAutoFillBackground(false);
	m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setScene(m_scene);

	m_animation = new QPropertyAnimation(m_proxy, "opacity", this);
	m_animation->setDuration(100);
	m_animation->setEasingCurve(QEasingCurve::OutQuad);
	m_animation->setStartValue(1);
	m_animation->setEndValue(0);

	readSettings();
}

QString NTrackInfoWidget::styleSheet() const
{
	return m_container->styleSheet();
}

void NTrackInfoWidget::setStyleSheet(const QString &stylesheet)
{
	m_container->setStyleSheet(stylesheet);
}

bool NTrackInfoWidget::eventFilter(QObject *object, QEvent *event)
{
	if (event->type() == QEvent::DragEnter || event->type() == QEvent::Leave)
		QApplication::sendEvent(parent(), event);


	if (event->type() == QEvent::GraphicsSceneMouseMove) {
		QGraphicsSceneMouseEvent *gfx_event = static_cast<QGraphicsSceneMouseEvent*>(event);
		QMouseEvent mouse_event(QEvent::MouseMove, gfx_event->scenePos().toPoint(), gfx_event->screenPos(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
		QApplication::sendEvent(parent(), &mouse_event);
	}

	return FALSE;
}

void NTrackInfoWidget::resizeEvent(QResizeEvent *event)
{
	Q_UNUSED(event);

	m_view->resize(size());
	m_proxy->resize(size());
	m_scene->setSceneRect(m_proxy->rect());
}

void NTrackInfoWidget::enterEvent(QEvent *event)
{
	Q_UNUSED(event);

	m_animation->setDirection(QAbstractAnimation::Forward);
	if (m_animation->state() == QAbstractAnimation::Stopped)
		m_animation->start();
}

void NTrackInfoWidget::leaveEvent(QEvent *event)
{
	Q_UNUSED(event);

	m_animation->setDirection(QAbstractAnimation::Backward);
	if (m_animation->state() == QAbstractAnimation::Stopped)
		m_animation->start();
}

void NTrackInfoWidget::setTagReader(NTagReaderInterface *tagReader)
{
	m_tagReader = tagReader;
	updateInfo();
}

void NTrackInfoWidget::updateInfo()
{
	if (!m_tagReader->isValid()) {
		QList<QLabel *> labels = m_container->findChildren<QLabel *>();
		foreach (QLabel *label, labels)
			label->hide();
	} else {
		foreach (QLabel *label, m_map.keys()) {
			QString info = m_tagReader->toString(m_map[label]);
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
	QList<QLabel *> labels = m_container->findChildren<QLabel *>();
	for (int i = 0; i < labels.size(); ++i) {
		QString objecName = labels.at(i)->objectName();
		objecName[0] = objecName.at(0).toUpper();
		QString format = NSettings::instance()->value("TrackInfo/" + objecName).toString();

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
	int total = m_tagReader->toString("%D").toInt();
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
			text = m_tagReader->toString(text);
		label->setText(text);
		label->show();
	}
}

/* vim: set ts=4 sw=4: */
