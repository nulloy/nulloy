/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2022 Sergey Vlasov <sergey@vlasov.me>
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

#include <QResizeEvent>

#include "coverReaderInterface.h"
#include "coverWidgetPopup.h"
#include "pluginLoader.h"
#include "settings.h"

NCoverWidget::NCoverWidget(QWidget *parent) : QLabel(parent)
{
    m_coverReader = dynamic_cast<NCoverReaderInterface *>(NPluginLoader::getPlugin(N::CoverReader));
    m_popup = NULL;

    hide();
    setScaledContents(true);
}

NCoverWidget::~NCoverWidget() {}

void NCoverWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::EnabledChange) {
        if (!m_pixmap.isNull()) {
            setVisible(isEnabled());
        }
    }

    QLabel::changeEvent(event);
}

void NCoverWidget::setSource(const QString &file)
{
    hide();
    init();

    QFileInfo fileInfo(file);
    if (!fileInfo.exists()) {
        return;
    }

    if (m_coverReader) {
        m_coverReader->setSource(file);
        m_pixmap = QPixmap::fromImage(m_coverReader->getImage());
    }

    if (m_pixmap.isNull()) { // fallback to external file
        QString pixmapFile;
        QDir dir = fileInfo.absoluteDir();
        QStringList images = dir.entryList(QStringList() << "*.jpg"
                                                         << "*.jpeg"
                                                         << "*.png",
                                           QDir::Files);

        // search for image which file name starts same as source file
        QString baseName = fileInfo.completeBaseName();
        foreach (QString image, images) {
            if (baseName.startsWith(QFileInfo(image).completeBaseName())) {
                pixmapFile = dir.absolutePath() + "/" + image;
                break;
            }
        }

        // search for cover.* or folder.* or front.*
        if (pixmapFile.isEmpty()) {
            QStringList matchedImages = images.filter(
                QRegExp("^(cover|folder|front)\\..*$", Qt::CaseInsensitive));
            if (!matchedImages.isEmpty()) {
                pixmapFile = dir.absolutePath() + "/" + matchedImages.first();
            }
        }

        m_pixmap = QPixmap(pixmapFile);
    }

    if (!m_pixmap.isNull()) { // first scale, then show
        setPixmap(m_pixmap);
        fitToHeight(height());
        if (isEnabled()) {
            show();
        }
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

void NCoverWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }

    if (!m_popup) {
        m_popup = new NCoverWidgetPopup(QWidget::window());
    }
    m_popup->setPixmap(m_pixmap);
    m_popup->show();
}

void NCoverWidget::fitToHeight(int height)
{
    QSize fixedAspect(m_pixmap.size());
    fixedAspect.scale(parentWidget()->width() / 2, height, Qt::KeepAspectRatio);

    setMaximumWidth(fixedAspect.width());
    setMinimumWidth(fixedAspect.width());

    if (fixedAspect.width() >=
        parentWidget()->width() / 2) { // stop scaling, leave space for waveform slider
        setMaximumHeight(fixedAspect.height());
    } else {
        setMaximumHeight(QWIDGETSIZE_MAX);
    }
}
