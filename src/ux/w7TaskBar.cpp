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

#ifdef WIN32

#include "w7TaskBar.h"

#include <windows.h>
#include <shlobj.h>
#include <QIcon>

#include <QDebug>

static WId _winId;
static ITaskbarList3 *_taskBar = NULL;
static UINT _messageId;
NW7TaskBar *NW7TaskBar::m_instance = NULL;

NW7TaskBar::NW7TaskBar(QObject *parent) : QObject(parent)
{
	Q_ASSERT_X(!m_instance, "NW7TaskBar", "NW7TaskBar instance already exists.");
	_messageId = RegisterWindowMessage(L"TaskbarButtonCreated");
}

NW7TaskBar::~NW7TaskBar()
{
	m_instance = NULL;
}

NW7TaskBar* NW7TaskBar::instance()
{
	Q_ASSERT_X(m_instance, "NW7TaskBar", "NW7TaskBar instance has not been created yet.");
	return m_instance;
}

void NW7TaskBar::init(QObject *parent)
{
	if (!m_instance)
		m_instance = new NW7TaskBar(parent);
}

void NW7TaskBar::setWindow(QWidget *window)
{
	Q_ASSERT_X(m_instance, "NW7TaskBar", "NW7TaskBar instance has not been created yet.");
	_winId = window->winId();
}

bool NW7TaskBar::winEvent(MSG *message, long *result)
{
	Q_ASSERT_X(m_instance, "NW7TaskBar", "NW7TaskBar instance has not been created yet.");

	if (message->message == _messageId) {
		HRESULT hr = CoCreateInstance(CLSID_TaskbarList, NULL,
										CLSCTX_INPROC_SERVER,
										IID_ITaskbarList3,
										reinterpret_cast<void **>(&_taskBar));
		if (SUCCEEDED(hr)) {
			hr = _taskBar->HrInit();
			if (FAILED(hr)) {
				_taskBar->Release();
				_taskBar = NULL;
				return FALSE;
			}
		}
		*result = hr;
		return TRUE;
	}
	return FALSE;
}

void NW7TaskBar::setProgress(qreal val)
{
	if (!_taskBar)
		return;

	_taskBar->SetProgressValue(_winId, qRound(val * 100), 100);

	if (val == 0)
		setState(NoProgress);
}

void NW7TaskBar::setState(State state)
{
	if (!_taskBar)
		return;

	TBPFLAG flag;
	switch (state) {
		case NoProgress:
			flag = TBPF_NOPROGRESS;
			break;
		case Indeterminate:
			flag = TBPF_INDETERMINATE;
			break;
		case Normal:
			flag = TBPF_NORMAL;
			break;
		case Error:
			flag = TBPF_ERROR;
			break;
		case Paused:
			flag = TBPF_PAUSED;
	}
	_taskBar->SetProgressState(_winId, flag);
}

void NW7TaskBar::setOverlayIcon(const QIcon &icon, const QString &text)
{
	Q_ASSERT_X(m_instance, "NW7TaskBar", "NW7TaskBar instance has not been created yet.");

	if (!_taskBar)
		return;

	HICON hIcon = NULL;
	if (!icon.isNull())
		hIcon = icon.pixmap(icon.availableSizes().first().width()).toWinHICON();

	_taskBar->SetOverlayIcon(_winId, hIcon, text.utf16());

	if (hIcon)
		DestroyIcon(hIcon);
}

#endif

/* vim: set ts=4 sw=4: */
