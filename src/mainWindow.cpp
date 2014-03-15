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

#include "mainWindow.h"

#include "common.h"
#include "settings.h"
#include "dropArea.h"

#ifndef _N_NO_SKINS_
#include "skinFileSystem.h"
#include <QUiLoader>
#endif

#ifndef _N_NO_PLUGINS_
#include "pluginLoader.h"
#include "tagReaderInterface.h"
#else
#include "tagReaderGstreamer.h"
#endif

#ifdef Q_WS_WIN
#include "w7TaskBar.h"
#include <windows.h>
#include <dwmapi.h>
#endif

#include <QEvent>
#include <QIcon>
#include <QLayout>
#include <QWindowStateChangeEvent>
#include <QToolTip>
#include <QTime>

NMainWindow::NMainWindow(QWidget *parent) : QDialog(parent)
{
#ifdef Q_WS_WIN
	m_framelessShadow = FALSE;
#endif
}

NMainWindow::~NMainWindow() {}

void NMainWindow::init(const QString &uiFile)
{
	setObjectName("mainWindow");

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
	form->setStyleSheet("");
#else
	Q_UNUSED(uiFile)
	ui.setupUi(this);
#endif

	m_oldPos = QPoint(-1, -1);
	m_oldSize = QSize(-1, -1);

	m_waveformSlider = qFindChild<QWidget *>(this, "waveformSlider");
	connect(m_waveformSlider, SIGNAL(mouseMoved(int, int)), this, SLOT(waveformSliderToolTip(int, int)));

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
	QStringList posList = NSettings::instance()->value("Position").toStringList();
	if (!posList.isEmpty()) {
		m_oldPos = QPoint(posList.at(0).toInt(), posList.at(1).toInt());
		move(m_oldPos);
	}

	QStringList sizeList = NSettings::instance()->value("Size").toStringList();
	if (!sizeList.isEmpty())
		m_oldSize = QSize(sizeList.at(0).toInt(), sizeList.at(1).toInt());
	else
		m_oldSize = QSize(430, 350);

	resize(m_oldSize);

	bool maximized = NSettings::instance()->value("Maximized").toBool();
	if (maximized)
		showMaximized();
}

void NMainWindow::saveSettings()
{
	bool maximized = isMaximized();
	NSettings::instance()->setValue("Maximized", maximized);

	QPoint savePos;
	QSize saveSize;
	if (!maximized) {
		savePos = pos();
		saveSize = size();
	} else {
		savePos = m_oldPos;
		saveSize = m_oldSize;
	}

	NSettings::instance()->setValue("Position", QStringList() << QString::number(savePos.x()) << QString::number(savePos.y()));
	NSettings::instance()->setValue("Size", QStringList() << QString::number(saveSize.width()) << QString::number(saveSize.height()));
}

void NMainWindow::setTitle(QString title)
{
	setWindowTitle(title);
	emit newTitle(title);
}

void NMainWindow::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::WindowStateChange) {
		QWindowStateChangeEvent *stateEvent = static_cast<QWindowStateChangeEvent *>(event);

		if (stateEvent->oldState() == Qt::WindowNoState && isMaximized()) {
			m_oldPos = pos();
			m_oldSize = size();
		}
	}

	QWidget::changeEvent(event);
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
bool _DwmIsCompositionEnabled()
{
	HMODULE library = LoadLibrary(L"dwmapi.dll");
	bool result = false;
	if (library) {
		BOOL enabled = FALSE;
		HRESULT (WINAPI *pFn)(BOOL *enabled) = (HRESULT (WINAPI *)(BOOL *enabled))(GetProcAddress(library, "DwmIsCompositionEnabled"));
		result = SUCCEEDED(pFn(&enabled)) && enabled;
		FreeLibrary(library);
	}
	return result;
}

void NMainWindow::setFramelessShadow(bool enabled)
{
	if (enabled != m_framelessShadow) {
		m_framelessShadow = enabled;
		updateFramelessShadow();
	}
}

void NMainWindow::updateFramelessShadow()
{
	DWORD version = GetVersion();
	DWORD major = (DWORD) (LOBYTE(LOWORD(version))); // major = 6 for vista/7/2008

	if (_DwmIsCompositionEnabled() && m_framelessShadow && major == 6)
		SetClassLongPtr(winId(), GCL_STYLE, GetClassLongPtr(winId(), GCL_STYLE) | CS_DROPSHADOW);
	else
		SetClassLongPtr(winId(), GCL_STYLE, GetClassLongPtr(winId(), GCL_STYLE) & ~CS_DROPSHADOW);

	hide();
	show();
}

bool NMainWindow::winEvent(MSG *message, long *result)
{
	if (message->message == WM_DWMCOMPOSITIONCHANGED) {
		updateFramelessShadow();
		return true;
	} else {
		return NW7TaskBar::winEvent(message, result);
	}
}
#endif

void NMainWindow::toggleMaximize()
{
	if (isMaximized())
		showNormal();
	else
		showMaximized();
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

void NMainWindow::waveformSliderToolTip(int x, int y)
{
	if (x != -1 && y != -1) {
		float pos = (float)x / m_waveformSlider->width();
		NTagReaderInterface *tagReader = dynamic_cast<NTagReaderInterface *>(NPluginLoader::getPlugin(N::TagReader));
		int duration = tagReader->toString("%D").toInt();
		int res = duration * pos;

		int hours = res / 60 / 60;
		QTime time = QTime().addSecs(res);
		QString timeStr;
		if (hours > 0)
			timeStr = time.toString("h:mm:ss");
		else
			timeStr = time.toString("m:ss");

		QToolTip::showText(m_waveformSlider->mapToGlobal(QPoint(x, y)), timeStr);
	}
}

