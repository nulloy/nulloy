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
#include "action.h"

#include <QDir>
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
	QList<NAction *> allActions = instance->findChildren<NAction *>();
	for (int i = 0; i < allActions.size(); ++i) {
		if (allActions.at(i)->parent() == instance && allActions.at(i)->isGlobal())
			m_globalActionList << allActions.at(i);
	}
}

void NSettings::loadShortcuts()
{
	for (int i = 0; i < m_globalActionList.size(); ++i) {
		QString strSeq = NSettings::value("GlobalShortcuts/" + m_globalActionList.at(i)->objectName()).toString();
		if (!strSeq.isEmpty())
			dynamic_cast<NAction *>(m_globalActionList.at(i))->setShortcut(QKeySequence(strSeq));
	}
}

void NSettings::saveShortcuts()
{
	for (int i = 0; i < m_globalActionList.size(); ++i) {
		QList<QKeySequence> shortcut = m_globalActionList.at(i)->shortcuts();
		QStringList strSeqList;
		foreach (QKeySequence seq, shortcut)
			strSeqList << seq.toString();

		if (!strSeqList.isEmpty() && m_globalActionList.at(i)->isEnabled())
			NSettings::setValue("GlobalShortcuts/" + m_globalActionList.at(i)->objectName(), strSeqList.join(", "));
		else
			NSettings::remove("GlobalShortcuts/" + m_globalActionList.at(i)->objectName());
	}
}

QList<QAction *> NSettings::shortcuts()
{
	return m_globalActionList;
}

void NSettings::setValue(const QString &key, const QVariant &value)
{
	QSettings::setValue(key, value);
	emit valueChanged(key, value);
}

void NSettings::remove(const QString &key)
{
	QSettings::remove(key);
	emit valueChanged(key, QString());
}

/* vim: set ts=4 sw=4: */
