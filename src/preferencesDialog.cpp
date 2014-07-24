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

#include "preferencesDialog.h"

#include "settings.h"
#include "player.h"
#include "skinFileSystem.h"
#include "plugin.h"
#include "i18nLoader.h"

#ifdef Q_WS_WIN
#include "w7TaskBar.h"
#endif

#ifndef _N_NO_SKINS_
#include "skinLoader.h"
#endif

#include <QGroupBox>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSpacerItem>
#include <QTextBrowser>
#include <QVBoxLayout>

using namespace NPluginLoader;

QStringList vNames = QStringList() << "Top" << "Middle" << "Bottom";
QStringList hNames = QStringList() << "Left" << "Center" << "Right";

NPreferencesDialog::~NPreferencesDialog() {}

NPreferencesDialog::NPreferencesDialog(QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(saveSettings()));
	connect(this, SIGNAL(accepted()), this, SLOT(saveSettings()));
	ui.buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Ok"));
	ui.buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
	ui.buttonBox->button(QDialogButtonBox::Apply)->setText(tr("Apply"));

	connect(ui.singleInstanceCheckBox, SIGNAL(toggled(bool)), ui.enqueueFilesCheckBox, SLOT(setEnabled(bool)));
	connect(ui.enqueueFilesCheckBox, SIGNAL(toggled(bool)), ui.playEnqueuedCheckBox, SLOT(setEnabled(bool)));
	connect(ui.singleInstanceCheckBox, SIGNAL(toggled(bool)), ui.playEnqueuedCheckBox, SLOT(setEnabled(bool)));
	connect(ui.restorePlaylistCheckBox, SIGNAL(toggled(bool)), ui.startPausedCheckBox, SLOT(setEnabled(bool)));

	setWindowTitle(QCoreApplication::applicationName() + tr(" Preferences"));

#ifdef _N_NO_SKINS_
	ui.skinLabel->hide();
	ui.skinComboBox->hide();
#endif

	QPixmap pixmap = QIcon::fromTheme("dialog-warning", style()->standardIcon(QStyle::SP_MessageBoxWarning)).pixmap(16);
	QByteArray byteArray;
	QBuffer buffer(&byteArray);
	pixmap.save(&buffer, "PNG");
	NSkinFileSystem::addFile("warning.png", byteArray);
	QString url = "<img src=\"skin:warning.png\"/>";

#ifdef _N_NO_PLUGINS_
	ui.tabWidget->removeTab(ui.tabWidget->indexOf(ui.pluginsTab));
#else
	QVBoxLayout *scrollLayout = new QVBoxLayout;
	ui.pluginsScrollArea->widget()->setLayout(scrollLayout);

	NFlagIterator<N::PluginType> iter(N::MaxPlugin);
	while (iter.hasNext()) {
		iter.next();
		N::PluginType type = iter.value();
		QGroupBox *groupBox = createGroupBox(type);
		if (groupBox)
			scrollLayout->addWidget(groupBox);
	}

	if (scrollLayout->count() > 0)
		scrollLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding));
	else
		ui.tabWidget->removeTab(ui.tabWidget->indexOf(ui.pluginsTab));

	ui.pluginsRestartLabel->setText(url + "&nbsp;&nbsp;" + ui.pluginsRestartLabel->text());
	ui.pluginsRestartLabel->setVisible(FALSE);
#endif

	ui.languageRestartLabel->setText(url + "&nbsp;&nbsp;" + ui.languageRestartLabel->text());
	ui.languageRestartLabel->setVisible(FALSE);

	ui.skinRestartLabel->setText(url + "&nbsp;&nbsp;" + ui.skinRestartLabel->text());
	ui.skinRestartLabel->setVisible(FALSE);
	connect(ui.skinComboBox, SIGNAL(activated(int)), ui.skinRestartLabel, SLOT(show()));

#ifndef Q_WS_WIN
	ui.taskbarProgressCheckBox->hide();
#endif

	ui.waveformTrackInfoTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
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
	emit versionRequested();
}

void NPreferencesDialog::on_titleFormatHelpButton_clicked()
{
	QDialog *dialog = new QDialog(this);
	dialog->setWindowTitle("Title Formats");
	dialog->setMaximumSize(0, 0);

	QVBoxLayout *layout = new QVBoxLayout;
	dialog->setLayout(layout);

	QTextBrowser *textBrowser = new QTextBrowser(this);
	textBrowser->setHtml(
		"<table width=\"100%\">"
			"<tr><td><b>%a</b></td><td>" + tr("Artist") + "</td></tr>"
			"<tr><td><b>%t</b></td><td>" + tr("Title") + "</td></tr>"
			"<tr><td><b>%A</b></td><td>" + tr("Album") + "</td></tr>"
			"<tr><td><b>%c</b></td><td>" + tr("Comment") + "</td></tr>"
			"<tr><td><b>%g</b></td><td>" + tr("Genre") + "</td></tr>"
			"<tr><td><b>%y</b></td><td>" + tr("Year") + "</td></tr>"
			"<tr><td><b>%n</b></td><td>" + tr("Track number") + "</td></tr>"
			"<tr><td></td><td></td></tr>"
			"<tr><td><b>%T</b></td><td>" + tr("Current time position (Waveform only)") + "</td></tr>"
			"<tr><td><b>%r</b></td><td>" + tr("Remaining time (Waveform only)") + "</td></tr>"
			"<tr><td></td><td></td></tr>"
			"<tr><td><b>%C</b></td><td>" + tr("Time position under cursor (Tooltip only)") + "</td></tr>"
			"<tr><td><b>%o</b></td><td>" + tr("Time offset under cursor (Tooltip only)") + "</td></tr>"
			"<tr><td></td><td></td></tr>"
			"<tr><td><b>%d</b></td><td>" + tr("Duration in format hh:mm:ss") + "</td></tr>"
			"<tr><td><b>%d</b></td><td>" + tr("Duration in seconds") + "</td></tr>"
			"<tr><td><b>%b</b></td><td>" + tr("Bit depth") + "</td></tr>"
			"<tr><td><b>%B</b></td><td>" + tr("Bitrate in Kbps") + "</td></tr>"
			"<tr><td><b>%s</b></td><td>" + tr("Sample rate in kHz") + "</td></tr>"
			"<tr><td><b>%c</b></td><td>" + tr("Number of channels") + "</td></tr>"
			"<tr><td></td><td></td></tr>"
			"<tr><td><b>%f</b></td><td>" + tr("File name without extension") + "</td></tr>"
			"<tr><td><b>%F</b></td><td>" + tr("File name") + "</td></tr>"
			"<tr><td><b>%p</b></td><td>" + tr("File name including absolute path") + "</td></tr>"
			"<tr><td><b>%e</b></td><td>" + tr("File name extension") + "</td></tr>"
			"<tr><td><b>%E</b></td><td>" + tr("File name extension, uppercased") + "</td></tr>"
			"<tr><td></td><td></td></tr>"
			"<tr><td><b>%v</b></td><td>" + tr("Version number") + "</td></tr>"
			"<tr><td><b>%%</b></td><td>" + tr("\'%\' character") + "</td></tr>"
			"<tr><td></td><td></td></tr>"
			"<tr><td>" + tr("Conditions:") + "</td><td></td></tr>"
			"<tr><td><b>{</b><i>" + tr("true") + "</i><b>|</b><i>" + tr("false") + "</i><b>}</b></td><td>" + tr("if...else: evaluate for <i>true</i> or <i>false</i> case. Note: nesting conditions is not supported yet.") + "</td></tr>"
			"<tr><td></td><td></td></tr>"
			"<tr><td>" + tr("Examples:") + "</td><td></td></tr>"
			"<tr><td><b>{%a - %t|%F}&nbsp;&nbsp;</b></td><td>" + tr("Print Artist and Title, separated with \"-\". If either of the tags is not available, print file name instead.") + "</td></tr>"
			"<tr><td></td><td></td></tr>"
			"<tr><td><b>{%g|}</b></td><td>" + tr("Print Genre. If not available, print nothing.") + "</td></tr>"
		"</table><br>");
	textBrowser->setStyleSheet("background: transparent");
	textBrowser->setFrameShape(QFrame::NoFrame);
	textBrowser->setMinimumWidth(400);
	textBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	layout->addWidget(textBrowser);

	QPushButton *closeButton = new QPushButton(tr("Close"));
	connect(closeButton, SIGNAL(clicked()), dialog, SLOT(accept()));
	QHBoxLayout *buttonLayout = new QHBoxLayout();
	buttonLayout->addStretch();
	buttonLayout->addWidget(closeButton);
	buttonLayout->addStretch();
	layout->addLayout(buttonLayout);

	dialog->show();

	// resize according to content
	QSize textSize = textBrowser->document()->size().toSize();
	textBrowser->setMinimumHeight(textSize.height());
}

QGroupBox* NPreferencesDialog::createGroupBox(N::PluginType type)
{
	QList<Descriptor> descriptors = NPluginLoader::descriptors();

	QString typeString = ENUM_TO_STR(N, PluginType, type);
	QString settingsContainer = NSettings::instance()->value("Plugins/" + typeString).toString();

	QList<int> indexesFilteredByType;
	for (int i = 0; i < descriptors.count(); ++i) {
		if (descriptors.at(i)[TypeRole] == type)
			indexesFilteredByType << i;
	}

	if (indexesFilteredByType.count() < 2) // need at least two plugins to choose from
		return NULL;

	QGroupBox *groupBox = new QGroupBox(typeString);
	QVBoxLayout *layout = new QVBoxLayout;
	layout->setContentsMargins(0, 5, 5, 0);
	layout->setSpacing(0);
	groupBox->setLayout(layout);

	foreach (int i, indexesFilteredByType) {
		QString containerName = descriptors.at(i)[ContainerNameRole].toString();
		QRadioButton *button = new QRadioButton(containerName);
		connect(button, SIGNAL(toggled(bool)), ui.pluginsRestartLabel, SLOT(show()));
		if (containerName == settingsContainer)
			button->setChecked(TRUE);
		m_radioButtons[button] = descriptors.at(i);
		layout->addWidget(button);
	}

	return groupBox;
}

void NPreferencesDialog::on_languageComboBox_activated(int index)
{
	ui.languageRestartLabel->setVisible(TRUE);

	QLocale locale = ui.languageComboBox->itemData(index).toLocale();
	NI18NLoader::loadTranslation(locale.language());
	QString newText = QCoreApplication::translate("PreferencesDialog", "Switching languages requires restart", 0, QApplication::UnicodeUTF8);
    ui.languageRestartLabel->setText(ui.languageRestartLabel->text().replace(QRegExp("(.*)&nbsp;.*"), "\\1&nbsp;" + newText));
}

QString NPreferencesDialog::selectedContainer(N::PluginType type)
{
	QList<Descriptor> descriptors = NPluginLoader::descriptors();
	QList<int> indexesFilteredByType;
	for (int i = 0; i < descriptors.count(); ++i) {
		if (descriptors.at(i)[TypeRole] == type)
			indexesFilteredByType << i;
	}

	QString containerName;
	foreach (int i, indexesFilteredByType) {
		QRadioButton *button = m_radioButtons.key(descriptors.at(i));
		if (button && button->isChecked()) {
			containerName = descriptors.at(i)[ContainerNameRole].toString();
			break;
		}
	}

	return containerName;
}

void NPreferencesDialog::loadSettings()
{
	// general >>
	ui.playlistTrackInfoLineEdit->setText(NSettings::instance()->value("PlaylistTrackInfo").toString());
	ui.windowTrackInfoLineEdit->setText(NSettings::instance()->value("WindowTitleTrackInfo").toString());
	ui.tooltipTrackInfoLineEdit->setText(NSettings::instance()->value("TooltipTrackInfo").toString());
	ui.minimizeToTrayCheckBox->setChecked(NSettings::instance()->value("MinimizeToTray").toBool());
	ui.restorePlaylistCheckBox->setChecked(NSettings::instance()->value("RestorePlaylist").toBool());
	ui.startPausedCheckBox->setChecked(NSettings::instance()->value("StartPaused").toBool());
	ui.startPausedCheckBox->setEnabled(NSettings::instance()->value("RestorePlaylist").toBool());
	ui.singleInstanceCheckBox->setChecked(NSettings::instance()->value("SingleInstance").toBool());
	ui.enqueueFilesCheckBox->setChecked(NSettings::instance()->value("EnqueueFiles").toBool());
	ui.enqueueFilesCheckBox->setEnabled(NSettings::instance()->value("SingleInstance").toBool());
	ui.playEnqueuedCheckBox->setChecked(NSettings::instance()->value("PlayEnqueued").toBool());
	ui.playEnqueuedCheckBox->setEnabled(NSettings::instance()->value("SingleInstance").toBool() && NSettings::instance()->value("EnqueueFiles").toBool());
	ui.trayIconCheckBox->setChecked(NSettings::instance()->value("TrayIcon").toBool());
	ui.versionCheckBox->setChecked(NSettings::instance()->value("AutoCheckUpdates").toBool());
	ui.displayLogDialogCheckBox->setChecked(NSettings::instance()->value("DisplayLogDialog").toBool());
	ui.showDecibelsVolumeCheckBox->setChecked(NSettings::instance()->value("ShowDecibelsVolume").toBool());
	ui.fileFiltersTextEdit->setPlainText(NSettings::instance()->value("FileFilters").toStringList().join(" "));
#ifdef Q_WS_WIN
	ui.taskbarProgressCheckBox->setChecked(NSettings::instance()->value("TaskbarProgress").toBool());
#endif
	ui.versionLabel->setText("");
	// << general


	// track info overlay >>
	for (int i = 0; i < ui.waveformTrackInfoTable->rowCount(); ++i) {
		for (int j = 0; j < ui.waveformTrackInfoTable->columnCount(); ++j) {
			QString objecName = ui.waveformTrackInfoTable->verticalHeaderItem(i)->text() + ui.waveformTrackInfoTable->horizontalHeaderItem(j)->text();
			QTableWidgetItem *item = new QTableWidgetItem(NSettings::instance()->value("TrackInfo/" + vNames.at(i) + hNames.at(j)).toString());
			item->setTextAlignment(Qt::AlignCenter);
			ui.waveformTrackInfoTable->setItem(i, j, item);
		}
	}

	int height = ui.waveformTrackInfoTable->horizontalHeader()->height();
	for (int i = 0; i < ui.waveformTrackInfoTable->rowCount(); ++i)
		height += ui.waveformTrackInfoTable->rowHeight(i);
	height += 2; // frame
	ui.waveformTrackInfoTable->setMaximumHeight(height);
	// << track info overlay


	// skins >>
#ifndef _N_NO_SKINS_
	int skinIndex;
	ui.skinComboBox->clear();
	foreach (QString str, NSkinLoader::skinIdentifiers()) {
		QString id = str.section('/', 2);
		ui.skinComboBox->addItem(id.replace('/', ' ').replace(" (Built-in)", tr(" (Built-in)")), id);
	}

	if (ui.skinComboBox->count() == 1)
		ui.skinComboBox->setEnabled(FALSE);

	skinIndex = ui.skinComboBox->findData(NSettings::instance()->value("Skin"));
	if (skinIndex != -1)
		ui.skinComboBox->setCurrentIndex(skinIndex);
#endif
	// << skins


	// translations >>
	int localeIndex;
	ui.languageComboBox->clear();
	foreach (QLocale::Language language, NI18NLoader::translations())
		ui.languageComboBox->addItem(QLocale::languageToString(language), QLocale(language));

	if (ui.languageComboBox->count() == 1)
		ui.languageComboBox->setEnabled(FALSE);

	localeIndex = ui.languageComboBox->findData(QLocale(NSettings::instance()->value("Language").toString()));
	if (localeIndex != -1)
		ui.languageComboBox->setCurrentIndex(localeIndex);
	// << translations


	// shortcuts >>
	NSettings::instance()->loadShortcuts();
	ui.shortcutEditorWidget->init(NSettings::instance()->shortcuts());
	// << shortcuts
}

void NPreferencesDialog::saveSettings()
{
	// general >>
	NSettings::instance()->setValue("PlaylistTrackInfo", ui.playlistTrackInfoLineEdit->text());
	NSettings::instance()->setValue("WindowTitleTrackInfo", ui.windowTrackInfoLineEdit->text());
	NSettings::instance()->setValue("TooltipTrackInfo", ui.tooltipTrackInfoLineEdit->text());
	NSettings::instance()->setValue("MinimizeToTray", ui.minimizeToTrayCheckBox->isChecked());
	NSettings::instance()->setValue("StartPaused", ui.startPausedCheckBox->isChecked());
	NSettings::instance()->setValue("RestorePlaylist", ui.restorePlaylistCheckBox->isChecked());
	NSettings::instance()->setValue("SingleInstance", ui.singleInstanceCheckBox->isChecked());
	NSettings::instance()->setValue("EnqueueFiles", ui.enqueueFilesCheckBox->isChecked());
	NSettings::instance()->setValue("PlayEnqueued", ui.playEnqueuedCheckBox->isChecked());
	NSettings::instance()->setValue("TrayIcon", ui.trayIconCheckBox->isChecked());
	NSettings::instance()->setValue("AutoCheckUpdates", ui.versionCheckBox->isChecked());
	NSettings::instance()->setValue("DisplayLogDialog", ui.displayLogDialogCheckBox->isChecked());
	NSettings::instance()->setValue("ShowDecibelsVolume", ui.showDecibelsVolumeCheckBox->isChecked());
	NSettings::instance()->setValue("FileFilters", ui.fileFiltersTextEdit->toPlainText().split(" "));
#ifdef Q_WS_WIN
	NSettings::instance()->setValue("TaskbarProgress", ui.taskbarProgressCheckBox->isChecked());
	NW7TaskBar::instance()->setEnabled(NSettings::instance()->value("TaskbarProgress").toBool());
#endif
	// << general


	// track info overlay >>
	for (int i = 0; i < ui.waveformTrackInfoTable->rowCount(); ++i) {
		for (int j = 0; j < ui.waveformTrackInfoTable->columnCount(); ++j) {
			QString objecName = ui.waveformTrackInfoTable->verticalHeaderItem(i)->text() + ui.waveformTrackInfoTable->horizontalHeaderItem(j)->text();
			NSettings::instance()->setValue("TrackInfo/" + vNames.at(i) + hNames.at(j), ui.waveformTrackInfoTable->item(i, j)->text());
		}
	}
	// << track info overlay


	// plugins >>
#ifndef _N_NO_PLUGINS_
	NFlagIterator<N::PluginType> iter(N::MaxPlugin);
	while (iter.hasNext()) {
		iter.next();
		N::PluginType type = iter.value();
		QString typeString = ENUM_TO_STR(N, PluginType, type);
		QString containerName = selectedContainer(type);
		if (!containerName.isEmpty())
			NSettings::instance()->setValue(QString() + "Plugins/" + typeString, containerName);
	}
#endif
	// << plugins


	// skins >>
#ifndef _N_NO_SKINS_
	NSettings::instance()->setValue("Skin", ui.skinComboBox->itemData(ui.skinComboBox->currentIndex()));
#endif
	// << skins


	// translations >>
	NSettings::instance()->setValue("Language", ui.languageComboBox->itemData(ui.languageComboBox->currentIndex()).toLocale().bcp47Name().split('-').first());
	// << translations


	// shortcuts >>
	ui.shortcutEditorWidget->applyShortcuts();
	NSettings::instance()->saveShortcuts();
	emit settingsChanged();
	// << shortcuts


	// systray check >>
	if (ui.trayIconCheckBox->isChecked() && !QSystemTrayIcon::isSystemTrayAvailable())
		QMessageBox::warning(this, "Systray Error", QObject::tr("System Tray (Notification Area) is not available on your system."));
	// << systray check
}

