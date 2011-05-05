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

#include "settings.h"
#include "rcDir.h"
#include "arguments.h"

static bool _settings_init = FALSE;
static QSettings *_settings;;

QSettings* settings()
{
	if (!_settings_init) {
		_settings = new QSettings(rcDir() + "/" +
						applicationBinaryName() + ".cfg",
						QSettings::IniFormat);
		_settings_init = TRUE;
	}

	return _settings;
}

/* vim: set ts=4 sw=4: */
