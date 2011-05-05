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

#include "mainWindow.h"

#include "settings.h"
#include "pluginInterface.h"
#include "pluginLoader.h"
#include "waveformSlider.h"
#include "dropArea.h"
#include "skinFileSystem.h"
#include "rcDir.h"

#include <QUiLoader>
#include <QLayout>
#include <QIcon>

#include <QDebug>

NMainWindow::NMainWindow(QWidget *parent) : QDialog(parent) {}
NMainWindow::~NMainWindow() {}

void NMainWindow::init(const QString &uiFile)
{
	QUiLoader loader;
	QFile formFile(uiFile);
	formFile.open(QIODevice::ReadOnly);
	QWidget *form = loader.load(&formFile);
	formFile.close();

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(qFindChild<QWidget *>(form, "centralWidget"));
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);
	setStyleSheet(form->styleSheet());

	setSizeGripEnabled(TRUE);

	NWaveformSlider *waveformSlider = qFindChild<NWaveformSlider *>(this, "waveformSlider");
	waveformSlider->setBuilder(waveformPlugin(settings()));

	// enabling dragging window from any point
	QList<QWidget *> widgets = findChildren<QWidget *>();
	for (int i = 0; i < widgets.size(); ++i)
		widgets.at(i)->installEventFilter(this);

	QStringList iconList;
#if !defined WIN32 && !defined _WINDOWS && !defined Q_WS_WIN
	iconList << "icon.";
	QDir parentDir(QCoreApplication::applicationDirPath());
	if (parentDir.dirName() == "bin") {
		iconList << rcDir() + "/icon.";
		iconList << "../share/nulloy/icon.";
	}
#endif
	iconList << ":icon.";

	QStringList iconFormats;
	iconFormats << "png" << "svg";

	bool set = FALSE;
	foreach (QString icon, iconList) {
		foreach (QString format, iconFormats) {
			QString iconFull = icon + format;
			if (QFileInfo(iconFull).exists()) {
				setWindowIcon(QIcon(iconFull));
				set = TRUE;
				break;
			}
		}
		if (set)
			break;
	}

	QMetaObject::connectSlotsByName(this);
}

void NMainWindow::toggleVisibility()
{
	setVisible(!isVisible());
}

void NMainWindow::loadSettings()
{
	QStringList posList = settings()->value("GUI/Position").toStringList();
	if (!posList.isEmpty())
		move(posList.at(0).toInt(), posList.at(1).toInt());

	QStringList sizeList = settings()->value("GUI/Size").toStringList();
	if (!sizeList.isEmpty())
		resize(sizeList.at(0).toInt(), sizeList.at(1).toInt());
	else
		resize(430, 250);
}

void NMainWindow::saveSettings()
{
	settings()->setValue("GUI/Position", QStringList() << QString::number(pos().x()) << QString::number(pos().y()));
	settings()->setValue("GUI/Size", QStringList() << QString::number(width()) << QString::number(height()));
}

void NMainWindow::setTitle(QString title)
{
	if (!title.isEmpty()) {
		title.prepend("\"");
		title.append("\" - ");
	}
	title.append(QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion());
	setWindowTitle(title);
	emit newTitle(title);
}

bool NMainWindow::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::MouseButtonPress && obj != this)
		m_dragActive = FALSE;

	return FALSE;
}

void NMainWindow::mousePressEvent(QMouseEvent *event)
{
	activateWindow();
	m_dragActive = FALSE;
	if (event->button() == Qt::LeftButton) {
		m_dragActive = TRUE;
		m_dragPoint = event->globalPos() - frameGeometry().topLeft();
		event->accept();
	}
}

void NMainWindow::mouseMoveEvent(QMouseEvent *event)
{
	if ((event->buttons() & Qt::LeftButton) && m_dragActive) {
		move(event->globalPos() - m_dragPoint);
		event->accept();
	}
}

void NMainWindow::resizeEvent(QResizeEvent *event)
{
	QDialog::resizeEvent(event);
	emit resized();
}

void NMainWindow::closeEvent(QCloseEvent *event)
{
	QDialog::closeEvent(event);
	emit closed();
}

void NMainWindow::minimize()
{
	setWindowState(Qt::WindowMinimized);
}

/* vim: set ts=4 sw=4: */
