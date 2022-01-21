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

#ifdef WIN32

#include "w7TaskBar.h"

#include <shlobj.h>
#include <windows.h>

#include <QIcon>
#include <QtWin>

#if defined(__GNUC__)
// mingw fails to define this
const GUID CLSID_TaskbarList = {0x56fdf344,
                                0xfd6d,
                                0x11d0,
                                {0x95, 0x8a, 0x00, 0x60, 0x97, 0xc9, 0xa0, 0x90}};
const GUID IID_ITaskbarList3 = {0xea1afb91,
                                0x9e28,
                                0x4b86,
                                {0x90, 0xe9, 0x9e, 0x9f, 0x8a, 0x5e, 0xef, 0xaf}};
#endif

static NW7TaskBar *_instance = NULL;
static WId _winId;
static ITaskbarList3 *_taskBar = NULL;
static UINT _messageId;
static bool _enabled = true;

NW7TaskBar *NW7TaskBar::instance()
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

bool NW7TaskBar::isEnabled() const
{
    return _enabled;
}

bool NW7TaskBar::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(eventType);
    MSG *msg = reinterpret_cast<MSG *>(message);
    if (msg->message == _messageId) {
        HRESULT hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER,
                                      IID_ITaskbarList3, reinterpret_cast<void **>(&_taskBar));
        if (SUCCEEDED(hr)) {
            hr = _taskBar->HrInit();
            if (FAILED(hr)) {
                _taskBar->Release();
                _taskBar = NULL;
                return false;
            }
        }
        *result = hr;
        return true;
    }
    return false;
}

void NW7TaskBar::setProgress(qreal val)
{
    if (!_taskBar || !_enabled)
        return;

    _taskBar->SetProgressValue((HWND)_winId, qRound(val * 100), 100);

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
    _taskBar->SetProgressState((HWND)_winId, flag);
}

void NW7TaskBar::setOverlayIcon(const QIcon &icon, const QString &text)
{
    if (!_taskBar || !_enabled)
        return;

    HICON hIcon = NULL;
    if (!icon.isNull())
        hIcon = QtWin::toHICON(icon.pixmap(icon.availableSizes().first().width()));

    wchar_t *wText = 0;
    wText = new wchar_t[text.length() + 1];
    wText[text.toWCharArray(wText)] = 0;

    _taskBar->SetOverlayIcon((HWND)_winId, hIcon, wText);

    if (hIcon)
        DestroyIcon(hIcon);
}

#endif
