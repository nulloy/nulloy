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

#include <QSettings>

namespace NSettings
{
	QSettings *_instance = FALSE;
}

void NSettings::init(QObject *parent)
{
	if (_instance)
		return;

	_instance = new QSettings(rcDir() + "/" +
					applicationBinaryName() + ".cfg",
					QSettings::IniFormat, parent);

	_instance->setValue("GUI/MinimizeToTray", _instance->value("GUI/MinimizeToTray", FALSE).toBool());
	_instance->setValue("GUI/TrayIcon", _instance->value("GUI/TrayIcon", FALSE).toBool());
	_instance->setValue("GUI/AlwaysOnTop", _instance->value("GUI/AlwaysOnTop", FALSE).toBool());
	_instance->setValue("RestorePlayback", _instance->value("RestorePlayback", TRUE).toBool());
	_instance->setValue("SingleInstanse", _instance->value("SingleInstanse", TRUE).toBool());
	_instance->setValue("AutoCheckUpdates", _instance->value("AutoCheckUpdates", TRUE).toBool());
	_instance->setValue("DisplayLogDialog", _instance->value("DisplayLogDialog", TRUE).toBool());
	_instance->setValue("Volume", _instance->value("Volume", 0.8).toFloat());
}

QVariant NSettings::value(const QString &key)
{
	init();
	return _instance->value(key);
}

void NSettings::setValue(const QString &key, const QVariant &value)
{
	init();
	_instance->setValue(key, value);
}

void NSettings::remove(const QString &key)
{
	init();
	_instance->remove(key);
}

/* vim: set ts=4 sw=4: */
