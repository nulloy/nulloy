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

#include "coverWidget.h"
#include "coverReaderInterface.h"
#include "pluginLoader.h"

#include <QResizeEvent>
#include <QDialog>
#include <QVBoxLayout>
#include <QCoreApplication>

class NCoverWidgetPopup : public QDialog
{
private:
	void mousePressEvent(QMouseEvent *)	{ hide(); }
public:
	NCoverWidgetPopup(QWidget *parent = 0) : QDialog(parent) {}
};

NCoverWidget::NCoverWidget(QWidget *parent) : QLabel(parent)
{
	m_coverReader = dynamic_cast<NCoverReaderInterface *>(NPluginLoader::getPlugin(N::CoverReader));

	if (m_coverReader) {
		m_popup = new NCoverWidgetPopup(this);
		m_popup->setMaximumSize(0, 0);
		QVBoxLayout *layout = new QVBoxLayout;
		layout->setContentsMargins(0, 0, 0, 0);
		m_popup->setLayout(layout);
		m_fullsizeLabel = new QLabel;
		layout->addWidget(m_fullsizeLabel);
	}

	hide();
	setScaledContents(TRUE);
}

NCoverWidget::~NCoverWidget() {}

void NCoverWidget::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::EnabledChange) {
		if (!m_pixmap.isNull())
			setVisible(isEnabled());
	}

	QLabel::changeEvent(event);
}

void NCoverWidget::setSource(const QString &file)
{
	if (!m_coverReader)
		return;

	hide();
	init();

	m_coverReader->setSource(file);
	m_pixmap = QPixmap::fromImage(m_coverReader->getImage());

	if (!m_pixmap.isNull()) { // first scale, then show
		setPixmap(m_pixmap);
		fitToHeight(height());
		if (isEnabled())
			show();
	}
}

void NCoverWidget::init()
{
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);

	setMinimumSize(0, 0);
	setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
}

void NCoverWidget::resizeEvent(QResizeEvent *event)
{
	fitToHeight(event->size().height());
}

void NCoverWidget::mousePressEvent(QMouseEvent *event)
{
	Q_UNUSED(event);
	m_fullsizeLabel->setPixmap(m_pixmap);
	m_popup->setWindowTitle(QString("%1x%2px").arg(m_pixmap.width()).arg(m_pixmap.height()));
	m_popup->show();
}

void NCoverWidget::fitToHeight(int height)
{
	QSize fixedAspect(m_pixmap.size());
	fixedAspect.scale(parentWidget()->width() / 2, height, Qt::KeepAspectRatio);

	setMaximumWidth(fixedAspect.width());
	setMinimumWidth(fixedAspect.width());

	if (fixedAspect.width() >= parentWidget()->width() / 2) // stop scaling, leave space for waveform slider
		setMaximumHeight(fixedAspect.height());
	else
		setMaximumHeight(QWIDGETSIZE_MAX);
}

