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
#include "player.h"

#ifndef _N_NO_SKINS_
#include "skinLoader.h"
#endif

#ifndef _N_NO_PLUGINS_
#include "pluginLoader.h"
#endif

#include <QMessageBox>
#include <QPushButton>

#include <QDebug>

NPreferencesDialog::~NPreferencesDialog() {}

NPreferencesDialog::NPreferencesDialog(QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);

	setObjectName("preferencesDialog");

	connect(ui.buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(saveSettings()));
	connect(this, SIGNAL(accepted()), this, SLOT(saveSettings()));

	setWindowTitle(QCoreApplication::applicationName() + " Preferences");

#ifdef _N_NO_SKINS_
	ui.skinLabel->hide();
	ui.skinComboBox->hide();
#endif

#ifdef _N_NO_PLUGINS_
	ui.tabWidget->removeTab(ui.tabWidget->indexOf(ui.pluginsTab));
#endif

#if QT_VERSION >= 0x040700
	ui.playlistTitleFormatLineEdit->setPlaceholderText("Example: %a - %t (%d)");
	ui.windowTitleFormatLineEdit->setPlaceholderText("Example: \"%a - %t\" - Nulloy %v");
#endif
}

void NPreferencesDialog::showEvent(QShowEvent *event)
{
	loadSettings();
	QDialog::showEvent(event);
}

void NPreferencesDialog::setVersionLabel(QString text)
{
	ui.versionLabel->setText(text);
}

void NPreferencesDialog::on_versionCheckButton_clicked()
{
	ui.versionLabel->setText("Checking...");
	emit versionOnlineRequested();
}

void NPreferencesDialog::on_titleFormatHelpButton_clicked()
{
	QDialog *dialog = new QDialog(this);
	dialog->setWindowTitle("Title Formats");
	dialog->setMaximumSize(0, 0);

	QVBoxLayout *layout = new QVBoxLayout;
	dialog->setLayout(layout);

	QTextBrowser *textBrowser = new QTextBrowser(this);
	textBrowser->setStyleSheet("background: transparent");
	textBrowser->setFrameShape(QFrame::NoFrame);
#ifdef Q_WS_MAC
	textBrowser->setHtml("<table width=\"300\">"
#else
	textBrowser->setHtml("<table width=\"200\">"
#endif
						"<tr><td><b>%a</b></td><td>Artist</td></tr>"
						"<tr><td><b>%t</b></td><td>Title</td></tr>"
						"<tr><td><b>%A</b></td><td>Album</td></tr>"
						"<tr><td><b>%c</b></td><td>Comment</td></tr>"
						"<tr><td><b>%g</b></td><td>Genre</td></tr>"
						"<tr><td><b>%y</b></td><td>Year</td></tr>"
						"<tr><td><b>%n</b></td><td>Track number</td></tr>"
						"<tr><td><b>%d</b></td><td>Duration formated as h:mm:ss</td></tr>"
						"<tr><td><b>%D</b></td><td>Duration in secods</td></tr>"
						"<tr><td><b>%B</b></td><td>Bitrate in Kb/s</td></tr>"
						"<tr><td><b>%s</b></td><td>Sample rate</td></tr>"
						"<tr><td><b>%c</b></td><td>Number of channels</td></tr>"
						"<tr><td><b>%f</b></td><td>File name without extension</td></tr>"
						"<tr><td><b>%F</b></td><td>File name</td></tr>"
						"<tr><td><b>%p</b></td><td>File name including absolute path</td></tr>"
						"<tr><td><b>%v</b></td><td>" + QCoreApplication::applicationName() + " version</td></tr>"
						"<tr><td><b>%%</b></td><td>\'%\' character</td></tr>"
						"</table><br>"
						"<br>"
						"Examples<br>"
						"Playlist: <b>%a - %t (%d)</b><br>"
						"Window title: <b>\"%a - %t\" - Nulloy %v</b>");

#ifdef Q_WS_MAC
	textBrowser->setMinimumWidth(350);
	textBrowser->setMinimumHeight(400);
#else
	textBrowser->setMinimumWidth(250);
	textBrowser->setMinimumHeight(350);
#endif
	layout->addWidget(textBrowser);

	QPushButton *closeButton = new QPushButton("Close");
	connect(closeButton, SIGNAL(clicked()), dialog, SLOT(accept()));
	QHBoxLayout* buttonLayout = new QHBoxLayout();
	buttonLayout->addStretch();
	buttonLayout->addWidget(closeButton);
	buttonLayout->addStretch();
	layout->addLayout(buttonLayout);

	dialog->show();
}

void NPreferencesDialog::loadSettings()
{
	ui.versionLabel->setText("");

	ui.playlistTitleFormatLineEdit->setText(NSettings::instance()->value("GUI/PlaylistTitleFormat").toString());
	ui.windowTitleFormatLineEdit->setText(NSettings::instance()->value("GUI/WindowTitleFormat").toString());
	ui.minimizeToTrayCheckBox->setChecked(NSettings::instance()->value("GUI/MinimizeToTray").toBool());
	ui.restorePlaybackCheckBox->setChecked(NSettings::instance()->value("RestorePlayback").toBool());
	ui.multipleInstansesCheckBox->setChecked(!NSettings::instance()->value("SingleInstanse").toBool());
	ui.trayIconCheckBox->setChecked(NSettings::instance()->value("GUI/TrayIcon").toBool());
	ui.versionCheckBox->setChecked(NSettings::instance()->value("AutoCheckUpdates").toBool());
	ui.displayLogDialogCheckBox->setChecked(NSettings::instance()->value("DisplayLogDialog").toBool());

	int index;

#ifndef _N_NO_PLUGINS_
	// plugins
	foreach (QString str, NPluginLoader::pluginIdentifiers()) {
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

	index = ui.playbackComboBox->findData(NSettings::instance()->value("Playback"));
	if (index != -1)
		ui.playbackComboBox->setCurrentIndex(index);
	index = ui.waveformComboBox->findData(NSettings::instance()->value("Waveform"));
	if (index != -1)
		ui.waveformComboBox->setCurrentIndex(index);
#endif

#ifndef _N_NO_SKINS_
	// skins
	foreach (QString str, NSkinLoader::skinIdentifiers()) {
		QString id = str.section('/', 2);
		ui.skinComboBox->addItem(id.replace('/', ' '), id);
	}

	if (ui.skinComboBox->count() == 1)
		ui.skinComboBox->setEnabled(FALSE);

	index = ui.skinComboBox->findData(NSettings::instance()->value("GUI/Skin"));
	if (index != -1)
		ui.skinComboBox->setCurrentIndex(index);
#endif

	NSettings::instance()->loadShortcuts();
	ui.shortcutEditorWidget->init(NSettings::instance()->shortcuts());
}

void NPreferencesDialog::saveSettings()
{
	NSettings::instance()->setValue("GUI/PlaylistTitleFormat", ui.playlistTitleFormatLineEdit->text());
	NSettings::instance()->setValue("GUI/WindowTitleFormat", ui.windowTitleFormatLineEdit->text());
	NSettings::instance()->setValue("GUI/MinimizeToTray", ui.minimizeToTrayCheckBox->isChecked());
	NSettings::instance()->setValue("RestorePlayback", ui.restorePlaybackCheckBox->isChecked());
	NSettings::instance()->setValue("SingleInstanse", !ui.multipleInstansesCheckBox->isChecked());
	NSettings::instance()->setValue("GUI/TrayIcon", ui.trayIconCheckBox->isChecked());
	NSettings::instance()->setValue("AutoCheckUpdates", ui.versionCheckBox->isChecked());
	NSettings::instance()->setValue("DisplayLogDialog", ui.displayLogDialogCheckBox->isChecked());

	bool showPluginMessage = FALSE;
	bool showSkinMessage = FALSE;

#ifndef _N_NO_PLUGINS_
	// plugins
	QVariant playbackVariant = ui.playbackComboBox->itemData(ui.playbackComboBox->currentIndex());
	if (NSettings::instance()->value("Playback").isValid() && playbackVariant != NSettings::instance()->value("Playback"))
		showPluginMessage = TRUE;
	QVariant waveformVariant = ui.waveformComboBox->itemData(ui.waveformComboBox->currentIndex());
	if (NSettings::instance()->value("Waveform").isValid() && waveformVariant != NSettings::instance()->value("Waveform"))
		showPluginMessage = TRUE;

	NSettings::instance()->setValue("Playback", playbackVariant);
	NSettings::instance()->setValue("Waveform", waveformVariant);
#endif

#ifndef _N_NO_SKINS_
	// skins
	QVariant skinVariant = ui.skinComboBox->itemData(ui.skinComboBox->currentIndex());
	if (NSettings::instance()->value("GUI/Skin").isValid() && skinVariant != NSettings::instance()->value("GUI/Skin"))
		showSkinMessage = TRUE;

	NSettings::instance()->setValue("GUI/Skin", skinVariant);
#endif

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

	ui.shortcutEditorWidget->applyShortcuts();
	NSettings::instance()->saveShortcuts();
	emit settingsChanged();
}

/* vim: set ts=4 sw=4: */
