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

#ifdef Q_WS_WIN
#include "w7TaskBar.h"
#include <windows.h>
#include <dwmapi.h>
#endif

#include <QEvent>
#include <QIcon>
#include <QLayout>
#include <QWindowStateChangeEvent>
#include <QTime>
#include <QDesktopWidget>
#include <QApplication>

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

	m_unmaximizedSize = QSize();
	m_unmaximizedPos = QPoint();
	m_isFullScreen = FALSE;
	m_dragActive = FALSE;

	// enabling dragging window from any point
	QList<QWidget *> widgets = findChildren<QWidget *>();
	foreach (QWidget *widget, widgets)
		widget->installEventFilter(this);

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

void NMainWindow::loadSettings()
{
	QPoint _pos;
	QStringList posList = NSettings::instance()->value("Position").toStringList();
	if (!posList.isEmpty()) {
		_pos = QPoint(posList.at(0).toInt(), posList.at(1).toInt());
		move(_pos);
	}

	QSize _size;
	QStringList sizeList = NSettings::instance()->value("Size").toStringList();
	if (!sizeList.isEmpty())
		_size = QSize(sizeList.at(0).toInt(), sizeList.at(1).toInt());
	else
		_size = QSize(430, 350);
	resize(_size);

	if (NSettings::instance()->value("Maximized").toBool()) {
		m_unmaximizedPos = _pos;
		m_unmaximizedSize = _size;
		toggleMaximize();
	}
}

void NMainWindow::saveSettings()
{
	NSettings::instance()->setValue("Maximized", isMaximized());

	QPoint _pos = pos();
	QSize _size = size();
	if (m_unmaximizedSize.isValid()) {
		_pos = m_unmaximizedPos;
		_size = m_unmaximizedSize;
	}

	NSettings::instance()->setValue("Position", QStringList() << QString::number(_pos.x()) << QString::number(_pos.y()));
	NSettings::instance()->setValue("Size", QStringList() << QString::number(_size.width()) << QString::number(_size.height()));
}

void NMainWindow::show()
{
	if (isMaximized()) {
		showMaximized();
		setGeometry(QApplication::desktop()->availableGeometry());
		showMaximized();
	} else {
		showNormal();
	}
}

void NMainWindow::toggleMaximize()
{
	if (isMaximized()) {
		showNormal();
		resize(m_unmaximizedSize);
		move(m_unmaximizedPos);
		m_unmaximizedPos = QPoint();
		m_unmaximizedSize = QSize();
	} else {
		m_unmaximizedPos = pos();
		m_unmaximizedSize = size();
		showMaximized();
#ifdef Q_WS_WIN
		setGeometry(QApplication::desktop()->availableGeometry());
		showMaximized();
#endif
	}

	emit fullScreenEnabled(FALSE);
	emit maximizeEnabled(isMaximized());
}

void NMainWindow::toggleFullScreen()
{
	if (!m_isFullScreen) {
		if (!m_unmaximizedSize.isValid()) {
			m_unmaximizedSize = size();
			m_unmaximizedPos = pos();
		}
		QDialog::showFullScreen();
	} else {
		QPoint _pos = m_unmaximizedPos;
		QSize _size = m_unmaximizedSize;
		QDialog::showNormal();
		m_unmaximizedPos = _pos;
		m_unmaximizedSize = _size;
		if (m_unmaximizedSize.isValid()) {
			resize(m_unmaximizedSize);
			move(m_unmaximizedPos);
			m_unmaximizedSize = QSize();
			m_unmaximizedPos = QPoint();
		}
	}

	m_isFullScreen = !m_isFullScreen;
	emit fullScreenEnabled(m_isFullScreen);
}

void NMainWindow::setTitle(QString title)
{
	setWindowTitle(title);
	emit newTitle(title);
}

void NMainWindow::changeEvent(QEvent *event)
{
	QWidget::changeEvent(event);

	if (windowFlags() & Qt::FramelessWindowHint)
		return;

	if (event->type() == QEvent::WindowStateChange) {
		QWindowStateChangeEvent *stateEvent = static_cast<QWindowStateChangeEvent *>(event);
		if (stateEvent->oldState() == Qt::WindowNoState && isMaximized()) {
			if (!m_unmaximizedSize.isValid()) {
				m_unmaximizedPos = pos();
				m_unmaximizedSize = size();
			}
		} else if (!isMaximized() && !isMinimized() && !m_isFullScreen) {
			m_unmaximizedSize = QSize();
			m_unmaximizedPos = QPoint();
		}
	}
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
	if ((event->buttons() & Qt::LeftButton) && m_dragActive && !isMaximized()) {
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
		return NW7TaskBar::instance()->winEvent(message, result);
	}
}
#endif

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
	NW7TaskBar::instance()->setWindow(this);
#endif
}

