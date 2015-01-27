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

#ifdef WIN32

#include "w7TaskBar.h"

#include <windows.h>
#include <shlobj.h>
#include <QIcon>

static NW7TaskBar *_instance = NULL;
static WId _winId;
static ITaskbarList3 *_taskBar = NULL;
static UINT _messageId;
static bool _enabled = TRUE;

NW7TaskBar* NW7TaskBar::instance()
{
	if (!_instance) {
		_instance = new NW7TaskBar();
		_messageId = RegisterWindowMessage(L"TaskbarButtonCreated");
	}

	return _instance;
}

void NW7TaskBar::setWindow(QWidget *window)
{
	_winId = window->winId();
}

void NW7TaskBar::setEnabled(bool enable)
{
	if (!enable) {
		setProgress(0);
		setState(NoProgress);
		setOverlayIcon(QIcon(), QString());
	}

	_enabled = enable;
}

bool NW7TaskBar::isEnabled()
{
	return _enabled;
}

bool NW7TaskBar::winEvent(MSG *message, long *result)
{
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
	if (!_taskBar || !_enabled)
		return;

	_taskBar->SetProgressValue(_winId, qRound(val * 100), 100);

	if (val == 0)
		setState(NoProgress);
}

void NW7TaskBar::setState(State state)
{
	if (!_taskBar || !_enabled)
		return;

	TBPFLAG flag;
	switch (state) {
		default:
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
	if (!_taskBar || !_enabled)
		return;

	HICON hIcon = NULL;
	if (!icon.isNull())
		hIcon = icon.pixmap(icon.availableSizes().first().width()).toWinHICON();

	wchar_t *wText = 0;
	wText = new wchar_t[text.length() + 1];
	wText[text.toWCharArray(wText)] = 0;

	_taskBar->SetOverlayIcon(_winId, hIcon, wText);

	if (hIcon)
		DestroyIcon(hIcon);
}

#endif

