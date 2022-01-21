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

#include <objc/message.h>
#include <Cocoa/Cocoa.h>

#include "macDock.h"

static NMacDock *_instance = NULL;

void _handleClick(id, SEL)
{
    _instance->_emitClicked();
}

NMacDock* NMacDock::instance()
{
    if (!_instance)
        _instance = new NMacDock();
    return _instance;
}

void NMacDock::registerClickHandler()
{
    Class cls = [[[NSApplication sharedApplication] delegate] class];
    SEL sel = @selector(applicationShouldHandleReopen:hasVisibleWindows:);
    if (class_getInstanceMethod(cls, sel))
        class_replaceMethod(cls, sel, (IMP)_handleClick, "v@:");
    else
        class_addMethod(cls, sel, (IMP)_handleClick,"v@:");
}

void NMacDock::_emitClicked()
{
    emit clicked();
}
