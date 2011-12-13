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

#include "core.h"

#include <QDir>
#include <QCoreApplication>
#include <QDesktopServices>

#include <QDebug>

NSettings *NSettings::m_instance = NULL;

NSettings::NSettings(QObject *parent)
:	QSettings(NCore::rcDir() + "/" +
				NCore::applicationBinaryName() + ".cfg",
				QSettings::IniFormat, parent)
{
	Q_ASSERT_X(!m_instance, "NSettings", "NSettings instance already exists.");
	m_instance = this;

	if (!QFileInfo(this->fileName()).exists()) {
		setValue("Shortcuts/playAction", QStringList() << "X" << "C" << "Space");
		setValue("Shortcuts/stopAction", "V");
		setValue("Shortcuts/prevAction", "Z");
		setValue("Shortcuts/nextAction", "B");

		setValue("GUI/PlaylistTitleFormat", "%a - %t (%d)");
		setValue("GUI/WindowTitleFormat", "\"%a - %t\" - " + QCoreApplication::applicationName() + " %v");
	}

	setValue("GUI/MinimizeToTray", value("GUI/MinimizeToTray", FALSE).toBool());
	setValue("GUI/TrayIcon", value("GUI/TrayIcon", FALSE).toBool());
	setValue("GUI/AlwaysOnTop", value("GUI/AlwaysOnTop", FALSE).toBool());
	setValue("GUI/WhilePlayingOnTop", value("GUI/WhilePlayingOnTop", FALSE).toBool());
	setValue("RestorePlayback", value("RestorePlayback", TRUE).toBool());
	setValue("SingleInstanse", value("SingleInstanse", TRUE).toBool());
	setValue("AutoCheckUpdates", value("AutoCheckUpdates", TRUE).toBool());
	setValue("DisplayLogDialog", value("DisplayLogDialog", TRUE).toBool());
	setValue("LastDirectory", value("LastDirectory", QDesktopServices::storageLocation(QDesktopServices::MusicLocation)).toString());
	setValue("LoadNext", value("LoadNext", FALSE).toBool());
	setValue("LoadNextSort", value("LoadNextSort", QDir::Name).toInt());
	setValue("Volume", value("Volume", 0.8).toFloat());
}

NSettings::~NSettings()
{
	m_instance = NULL;
}

NSettings* NSettings::instance()
{
	Q_ASSERT_X(m_instance, "NSettings", "NSettings instance has not been created yet.");
	return m_instance;
}

void NSettings::initShortcuts(QObject *instance)
{
	foreach (NAction *action, instance->findChildren<NAction *>()) {
		if (action->isCustomizable())
			m_actionList << action;
	}
}

void NSettings::loadShortcuts()
{
	struct _local
	{
		static QList<QKeySequence> strToSeq(const QStringList &strList)
		{
			QList<QKeySequence> shortcuts;
			foreach (QString str, strList)
				shortcuts << QKeySequence(str);
			return shortcuts;
		}
	};

	foreach (NAction *action, m_actionList) {
		action->setShortcuts(_local::strToSeq(value("Shortcuts/" + action->objectName()).toStringList()));
		action->setGlobalShortcuts(_local::strToSeq(value("GlobalShortcuts/" + action->objectName()).toStringList()));
	}
}

void NSettings::saveShortcuts()
{
	struct _local
	{
		static void save(NSettings *settings, QList<QKeySequence> keys, QString name)
		{
			QStringList keyStrings;
			foreach (QKeySequence seq, keys) {
				if (!seq.isEmpty())
					keyStrings << seq.toString();
			}
			if (!keyStrings.isEmpty())
				settings->setValue(name, keyStrings);
			else
				settings->remove(name);
		}
	};

	foreach (NAction *action, m_actionList) {
		if (action->objectName().isEmpty() || !action->isCustomizable())
			continue;
		_local::save(this, action->shortcuts(), "Shortcuts/" + action->objectName());
		_local::save(this, action->globalShortcuts(), "GlobalShortcuts/" + action->objectName());
	}
}

QList<NAction *> NSettings::shortcuts()
{
	return m_actionList;
}

void NSettings::setValue(const QString &key, const QVariant &value)
{
	if ((value.type() == QVariant::String && value.toString().isEmpty()) ||
		(value.type() == QVariant::StringList && value.toStringList().isEmpty()) ||
		(value.type() == QVariant::List && value.toList().isEmpty()))
	{
		QSettings::remove(key);
		emit valueChanged(key, QString());
		return;
	}

	QSettings::setValue(key, value);
	emit valueChanged(key, value);
}

void NSettings::remove(const QString &key)
{
	QSettings::remove(key);
	emit valueChanged(key, QString());
}

/* vim: set ts=4 sw=4: */
