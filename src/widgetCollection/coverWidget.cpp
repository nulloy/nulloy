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

#include "coverWidget.h"
#include "coverReaderInterface.h"
#include "pluginLoader.h"
#include "settings.h"

#include <QResizeEvent>
#include <QDialog>
#include <QBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QCoreApplication>

#define MARGIN 50

class NCoverWidgetPopup : public QDialog
{
private:
	void mousePressEvent(QMouseEvent *)	{ hide(); }
	void changeEvent(QEvent *) { if (!isActiveWindow()) hide(); }
public:
	NCoverWidgetPopup(QWidget *parent = 0) : QDialog(parent) {}
};

NCoverWidget::NCoverWidget(QWidget *parent) : QLabel(parent)
{
	m_coverReader = dynamic_cast<NCoverReaderInterface *>(NPluginLoader::getPlugin(N::CoverReader));
	m_popup = new NCoverWidgetPopup(this);
	m_fullsizeLabel = new QLabel;

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
	hLayout->addWidget(m_fullsizeLabel);
	hLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));

	QVBoxLayout *vLayout = new QVBoxLayout;
	vLayout->setContentsMargins(MARGIN, MARGIN, MARGIN, MARGIN);
	vLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
	vLayout->addLayout(hLayout);
	vLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));

	QWidget *container = new QWidget();
	QVBoxLayout *cLayout = new QVBoxLayout;
	cLayout->addWidget(container);
	cLayout->setContentsMargins(0, 0, 0, 0);
	container->setLayout(vLayout);
	container->setStyleSheet("background-color: rgba(0, 0, 0, 200);");

	m_popup->setLayout(cLayout);
	m_popup->setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
	m_popup->setAttribute(Qt::WA_TranslucentBackground);

	hide();
	setScaledContents(true);
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
	hide();
	init();

	if (m_coverReader) {
		m_coverReader->setSource(file);
		m_pixmap = QPixmap::fromImage(m_coverReader->getImage());
		m_coverReader->setSource(""); // release the file
	}

	if (m_pixmap.isNull()){ // fallback to external file
		QString pixmapFile;
		QDir dir = QFileInfo(file).absoluteDir();
		QStringList images = dir.entryList(QStringList() << "*.jpg" << "*.png", QDir::Files);

		// search for image which file name starts same as source file
		QString baseName = QFileInfo(file).completeBaseName();
		foreach (QString image, images) {
			if (baseName.startsWith(QFileInfo(image).completeBaseName())) {
				pixmapFile = dir.absolutePath() + "/" + image;
				break;
			}
		}

		// search for cover.* or folder.* or front.*
		if (pixmapFile.isEmpty()) {
			QStringList matchedImages = images.filter(QRegExp("^(cover|folder|front)\\..*$", Qt::CaseInsensitive));
			if (!matchedImages.isEmpty())
				pixmapFile = dir.absolutePath() + "/" + matchedImages.first();
		}

		m_pixmap = QPixmap(pixmapFile);
	}

	if (!m_pixmap.isNull()) { // first scale, then show
		setPixmap(m_pixmap);
		fitToHeight(height());
		if (isEnabled())
			show();
	}
}

void NCoverWidget::init()
{
	m_pixmap = QPixmap();

	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);

	setMinimumSize(0, 0);
	setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
}

void NCoverWidget::resizeEvent(QResizeEvent *event)
{
	fitToHeight(event->size().height());
}

void NCoverWidget::mousePressEvent(QMouseEvent *)
{
	QSize margin = QSize(MARGIN * 2, MARGIN * 2);
	QPixmap pixmap = m_pixmap;
	QSize pixmapMaxSize = QWidget::window()->size() - margin;
	if (pixmap.height() > pixmapMaxSize.height() || pixmap.width() > pixmapMaxSize.width())
		pixmap = pixmap.scaled(pixmapMaxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	m_fullsizeLabel->setPixmap(pixmap);
	m_popup->show();
	m_popup->setMinimumSize(QWidget::window()->size());
	m_popup->setMaximumSize(QWidget::window()->size());
	m_popup->setGeometry(QWidget::window()->geometry());
	m_popup->setToolTip(QString("%1 x %2").arg(m_pixmap.width()).arg(m_pixmap.height()));
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

