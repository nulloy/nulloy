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

#include "core.h"
#include "settings.h"
#include "waveformSlider.h"
#include "dropArea.h"

#ifndef _N_NO_SKINS_
#include "skinFileSystem.h"
#include <QUiLoader>
#endif

#ifndef _N_NO_PLUGINS_
#include "pluginLoader.h"
#else
#include "waveformBuilderGstreamer.h"
#endif

#ifdef Q_WS_WIN
#include "w7TaskBar.h"
#include <windows.h>
#endif

#include <QLayout>
#include <QIcon>

#include <QDebug>

NMainWindow::NMainWindow(QWidget *parent) : QDialog(parent) {}
NMainWindow::~NMainWindow() {}

void NMainWindow::init(const QString &uiFile)
{
#ifndef _N_NO_SKINS_
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
#else
	Q_UNUSED(uiFile)
	ui.setupUi(this);
#endif

	NWaveformSlider *waveformSlider = qFindChild<NWaveformSlider *>(this, "waveformSlider");
#ifndef _N_NO_PLUGINS_
	waveformSlider->setBuilder(NPluginLoader::waveformPlugin());
#else
	NWaveformBuilderInterface *builder = dynamic_cast<NWaveformBuilderInterface *>(new NWaveformBuilderGstreamer());
	dynamic_cast<NPluginInterface *>(builder)->init();
	waveformSlider->setBuilder(builder);
#endif

	// enabling dragging window from any point
	QList<QWidget *> widgets = findChildren<QWidget *>();
	for (int i = 0; i < widgets.size(); ++i)
		widgets.at(i)->installEventFilter(this);

	QIcon icon;
#ifdef Q_WS_X11
	icon = QIcon::fromTheme("nulloy");
#endif
#ifndef Q_WS_MAC
	if (icon.isNull()) {
		QStringList files = QDir(":").entryList(QStringList() << "icon-*", QDir::Files);
		foreach (QString fileName, files)
				icon.addFile(":" + fileName);
	}
#else
	icon.addFile(":icon-16.png");
#endif
	setWindowIcon(icon);

	QMetaObject::connectSlotsByName(this);
}

void NMainWindow::toggleVisibility()
{
	setVisible(!isVisible());
}

void NMainWindow::loadSettings()
{
	QStringList posList = NSettings::instance()->value("GUI/Position").toStringList();
	if (!posList.isEmpty())
		move(posList.at(0).toInt(), posList.at(1).toInt());

	QStringList sizeList = NSettings::instance()->value("GUI/Size").toStringList();
	if (!sizeList.isEmpty())
		resize(sizeList.at(0).toInt(), sizeList.at(1).toInt());
	else
		resize(430, 250);
}

void NMainWindow::saveSettings()
{
	NSettings::instance()->setValue("GUI/Position", QStringList() << QString::number(pos().x()) << QString::number(pos().y()));
	NSettings::instance()->setValue("GUI/Size", QStringList() << QString::number(width()) << QString::number(height()));
}

void NMainWindow::setTitle(QString title)
{
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

void NMainWindow::showEvent(QShowEvent *event)
{
	loadSettings();
	QDialog::showEvent(event);
}

void NMainWindow::hideEvent(QHideEvent *event)
{
	saveSettings();
	QDialog::hideEvent(event);
}

void NMainWindow::closeEvent(QCloseEvent *event)
{
	saveSettings();
	QDialog::closeEvent(event);
	emit closed();
}

#ifdef Q_WS_WIN
bool NMainWindow::winEvent(MSG *message, long *result)
{
	return NW7TaskBar::winEvent(message, result);
}
#endif

void NMainWindow::minimize()
{
	setWindowState(Qt::WindowMinimized);
}

void NMainWindow::setOnTop(bool onTop)
{
#ifdef Q_WS_WIN
	if (onTop)
		SetWindowPos(this->winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	else
		SetWindowPos(this->winId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
#else
	Qt::WindowFlags flags = windowFlags();
	if (onTop)
		flags |= Qt::WindowStaysOnTopHint;
	else
		flags &= ~Qt::WindowStaysOnTopHint;
	setWindowFlags(flags);
	show();
#endif

#ifdef Q_WS_WIN
	NW7TaskBar::setWindow(this);
#endif
}

/* vim: set ts=4 sw=4: */
