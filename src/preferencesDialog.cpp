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

#include "preferencesDialog.h"

#include "settings.h"
#include "pluginLoader.h"
#include "skinLoader.h"
#include "player.h"
#include <QMessageBox>
#include <QPushButton>

#include <QDebug>

NPreferencesDialog::~NPreferencesDialog() {}

NPreferencesDialog::NPreferencesDialog(QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(saveSettings()));
	connect(this, SIGNAL(accepted()), this, SLOT(saveSettings()));

	setWindowTitle(QCoreApplication::applicationName() + " Preferences");
}

void NPreferencesDialog::initShortcuts()
{
	foreach (QWidget *widget, QApplication::topLevelWidgets()) {
		NPlayer *player = qobject_cast<NPlayer *>(widget);
		if (player) {
			QList<NAction *> allActions = player->findChildren<NAction *>();
			for (int i = 0; i < allActions.size(); ++i) {
				NAction *action = allActions.at(i);
				if (action->parent() == player && action->isGlobal())
					m_globalActionList << action;
			}
		}
	}
}

void NPreferencesDialog::show()
{
	ui.versionLabel->setText("");
	QDialog::show();
}

void NPreferencesDialog::setVersionLabel(QString text)
{
	ui.versionLabel->setText(text);
}

void NPreferencesDialog::on_versionCheckButton_clicked()
{
	ui.versionLabel->setText("Checking...");
	versionCheckOnline();
}

void NPreferencesDialog::loadSettings()
{
	ui.minimizeToTrayCheckBox->setChecked(settings()->value("GUI/MinimizeToTray", FALSE).toBool());
	ui.restorePlaybackCheckBox->setChecked(settings()->value("RestorePlayback", TRUE).toBool());
	ui.singleInstanseCheckBox->setChecked(settings()->value("SingleInstanse", FALSE).toBool());
	ui.trayIconCheckBox->setChecked(settings()->value("GUI/TrayIcon", FALSE).toBool());
	ui.versionCheckBox->setChecked(settings()->value("AutoCheckUpdates", TRUE).toBool());

	// plugins
	foreach (QString str, pluginIdentifiers(settings())) {
		QString id = str.section('/', 2);
		if (str.split('/').at(1) == "Playback")
			ui.playbackComboBox->addItem(id.replace('/', ' '), id);
		if (str.split('/').at(1) == "Waveform")
			ui.waveformComboBox->addItem(id.replace('/', ' '), id);
	}

	if (ui.playbackComboBox->count() == 1)
		ui.playbackComboBox->setEnabled(FALSE);
	if (ui.waveformComboBox->count() == 1)
		ui.waveformComboBox->setEnabled(FALSE);

	int index;
	index = ui.playbackComboBox->findData(settings()->value("Playback"));
	if (index != -1)
		ui.playbackComboBox->setCurrentIndex(index);
	index = ui.playbackComboBox->findData(settings()->value("Waveform"));
	if (index != -1)
		ui.waveformComboBox->setCurrentIndex(index);

	// skins
	foreach (QString str, skinIdentifiers(settings())) {
		QString id = str.section('/', 2);
		ui.skinComboBox->addItem(id.replace('/', ' '), id);
	}

	if (ui.skinComboBox->count() == 1)
		ui.skinComboBox->setEnabled(FALSE);

	index = ui.skinComboBox->findData(settings()->value("GUI/Skin"));
	if (index != -1)
		ui.skinComboBox->setCurrentIndex(index);

	// shortcuts
	for (int i = 0; i < m_globalActionList.size(); ++i) {
		QString strSeq = settings()->value("GlobalShortcuts/" + m_globalActionList.at(i)->objectName()).toString();
		if (!strSeq.isEmpty())
			m_globalActionList.at(i)->setShortcut(QKeySequence(strSeq));
	}
	ui.globalShortcutEditorWidget->init(m_globalActionList);
}

void NPreferencesDialog::saveSettings()
{
	settings()->setValue("GUI/MinimizeToTray", ui.minimizeToTrayCheckBox->isChecked());
	settings()->setValue("RestorePlayback", ui.restorePlaybackCheckBox->isChecked());
	settings()->setValue("SingleInstanse", ui.singleInstanseCheckBox->isChecked());
	settings()->setValue("GUI/TrayIcon", ui.trayIconCheckBox->isChecked());
	settings()->setValue("AutoCheckUpdates", ui.versionCheckBox->isChecked());

	// plugins
	bool showPluginMessage = FALSE;
	QVariant playbackVariant = ui.playbackComboBox->itemData(ui.playbackComboBox->currentIndex());
	if (settings()->value("Playback").isValid() && playbackVariant != settings()->value("Playback"))
		showPluginMessage = TRUE;
	QVariant waveformVariant = ui.waveformComboBox->itemData(ui.waveformComboBox->currentIndex());
	if (settings()->value("Waveform").isValid() && waveformVariant != settings()->value("Waveform"))
		showPluginMessage = TRUE;

	settings()->setValue("Playback", playbackVariant);
	settings()->setValue("Waveform", waveformVariant);

	// skins
	bool showSkinMessage = FALSE;
	QVariant skinVariant = ui.skinComboBox->itemData(ui.skinComboBox->currentIndex());
	if (settings()->value("GUI/Skin").isValid() && skinVariant != settings()->value("GUI/Skin"))
		showSkinMessage = TRUE;

	settings()->setValue("GUI/Skin", skinVariant);

	QString message;
	if (showPluginMessage && showSkinMessage) {
		message = tr("Switching plugins and skins requires restart.");
	} else {
		if (showPluginMessage)
			message = tr("Switching plugins requires restart.");
		if (showSkinMessage)
			message = tr("Switching skins requires restart.");
	}
	if (!message.isEmpty()) {
		QMessageBox box(QMessageBox::Information, windowTitle(), message, QMessageBox::Close, this);
		box.exec();
	}

	// shortcuts
	ui.globalShortcutEditorWidget->applyShortcuts();
	for (int i = 0; i < m_globalActionList.size(); ++i) {
		QList<QKeySequence> shortcut = m_globalActionList.at(i)->shortcuts();
		QStringList strSeqList;
		foreach (QKeySequence seq, shortcut)
			strSeqList << seq.toString();

		if (!strSeqList.isEmpty())
			settings()->setValue("GlobalShortcuts/" + m_globalActionList.at(i)->objectName(), strSeqList.join(", "));
		else
			settings()->remove("GlobalShortcuts/" + m_globalActionList.at(i)->objectName());
	}

	emit settingsChanged();
}

/* vim: set ts=4 sw=4: */
