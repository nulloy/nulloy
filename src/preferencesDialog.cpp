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

	ui.skinRestartLabel->setVisible(FALSE);
	ui.pluginsRestartLabel->setVisible(FALSE);

	QPixmap pixmap = QIcon::fromTheme("dialog-warning", style()->standardIcon(QStyle::SP_MessageBoxWarning)).pixmap(16);
	QByteArray byteArray;
	QBuffer buffer(&byteArray);
	pixmap.save(&buffer, "PNG");
	NSkinFileSystem::addFile("warning.png", byteArray);
	QString url = "<img src=\"skin:warning.png\"/>";

	ui.skinRestartLabel->setText(url + "&nbsp;&nbsp;" + ui.skinRestartLabel->text());
	ui.pluginsRestartLabel->setText(url + "&nbsp;&nbsp;" + ui.pluginsRestartLabel->text());
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
	textBrowser->setHtml("<table width=\"100%\">"
	                       "<tr><td><b>%a</b></td><td>Artist</td></tr>"
	                       "<tr><td><b>%t</b></td><td>Title</td></tr>"
	                       "<tr><td><b>%A</b></td><td>Album</td></tr>"
	                       "<tr><td><b>%c</b></td><td>Comment</td></tr>"
	                       "<tr><td><b>%g</b></td><td>Genre</td></tr>"
	                       "<tr><td><b>%y</b></td><td>Year</td></tr>"
	                       "<tr><td><b>%n</b></td><td>Track number</td></tr>"
	                       "<tr><td><b>%d</b></td><td>Duration</td></tr>"
	                       "<tr><td><b>%T</b></td><td>Current time position (Waveform only)</td></tr>"
	                       "<tr><td><b>%r</b></td><td>Remaining time (Waveform only)</td></tr>"
	                       "<tr><td><b>%B</b></td><td>Bitrate in Kb/s</td></tr>"
	                       "<tr><td><b>%s</b></td><td>Sample rate in kHz</td></tr>"
	                       "<tr><td><b>%c</b></td><td>Number of channels</td></tr>"
	                       "<tr><td><b>%f</b></td><td>File name without extension</td></tr>"
	                       "<tr><td><b>%F</b></td><td>File name</td></tr>"
	                       "<tr><td><b>%p</b></td><td>File name including absolute path</td></tr>"
	                       "<tr><td><b>%v</b></td><td>" + QCoreApplication::applicationName() + " version</td></tr>"
	                       "<tr><td><b>%%</b></td><td>\'%\' character</td></tr>"
	                       "<tr><td></td><td></td></tr>"
	                       "<tr><td>Conditions:</td><td></td></tr>"
	                       "<tr><td><b>{</b><i>true</i><b>|</b><i>false</i><b>}</b></td><td>If / Else: evaluate for <i>true</i> or <i>false</i> case. Note: nesting conditions is not supported yet.</td></tr>"
	                       "<tr><td></td><td></td></tr>"
	                       "<tr><td>Examples:</td><td></td></tr>"
	                       "<tr><td><b>{%a - %t|%F}&nbsp;&nbsp;</b></td><td>Print Artist and Title, separated with \"-\". If either of the tags is not available, print file name instead.</td></tr>"
	                       "<tr><td></td><td></td></tr>"
	                       "<tr><td><b>{%g|}</b></td><td>Print Genre. If not available, print nothing.</td></tr>"
	                     "</table><br>");
	textBrowser->setStyleSheet("background: transparent");
	textBrowser->setFrameShape(QFrame::NoFrame);
	textBrowser->setMinimumWidth(400);
	textBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	layout->addWidget(textBrowser);

	QPushButton *closeButton = new QPushButton("Close");
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

	QString typeString = ENUM_NAME(N, PluginType, type);
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
		connect(button, SIGNAL(toggled(bool)), this, SLOT(pluginsChanged()));
		if (containerName == settingsContainer)
			button->setChecked(TRUE);
		m_radioButtons[button] = descriptors.at(i);
		layout->addWidget(button);
	}

	return groupBox;
}

void NPreferencesDialog::pluginsChanged()
{
	ui.pluginsRestartLabel->setVisible(TRUE);
}

void NPreferencesDialog::on_skinComboBox_activated(int index)
{
	Q_UNUSED(index);
	ui.skinRestartLabel->setVisible(TRUE);
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
	ui.versionLabel->setText("");

	ui.playlistTrackInfoLineEdit->setText(NSettings::instance()->value("PlaylistTrackInfo").toString());
	ui.windowTrackInfoLineEdit->setText(NSettings::instance()->value("WindowTitleTrackInfo").toString());
	ui.minimizeToTrayCheckBox->setChecked(NSettings::instance()->value("MinimizeToTray").toBool());
	ui.restorePlaybackCheckBox->setChecked(NSettings::instance()->value("RestorePlayback").toBool());
	ui.multipleInstansesCheckBox->setChecked(!NSettings::instance()->value("SingleInstanse").toBool());
	ui.trayIconCheckBox->setChecked(NSettings::instance()->value("TrayIcon").toBool());
	ui.versionCheckBox->setChecked(NSettings::instance()->value("AutoCheckUpdates").toBool());
	ui.displayLogDialogCheckBox->setChecked(NSettings::instance()->value("DisplayLogDialog").toBool());

	for (int i = 0; i < ui.waveformTrackInfoTable->rowCount(); ++i) {
		for (int j = 0; j < ui.waveformTrackInfoTable->columnCount(); ++j) {
			QString objecName = ui.waveformTrackInfoTable->verticalHeaderItem(i)->text() + ui.waveformTrackInfoTable->horizontalHeaderItem(j)->text();
			QTableWidgetItem *item = new QTableWidgetItem(NSettings::instance()->value("TrackInfo/" + objecName).toString());
			item->setTextAlignment(Qt::AlignCenter);
			ui.waveformTrackInfoTable->setItem(i, j, item);
		}
	}

	int height = ui.waveformTrackInfoTable->horizontalHeader()->height();
	for (int i = 0; i < ui.waveformTrackInfoTable->rowCount(); ++i)
		height += ui.waveformTrackInfoTable->rowHeight(i);
	height += 2; // frame
	ui.waveformTrackInfoTable->setMaximumHeight(height);

	int index;

#ifndef _N_NO_SKINS_
	ui.skinComboBox->clear();
	foreach (QString str, NSkinLoader::skinIdentifiers()) {
		QString id = str.section('/', 2);
		ui.skinComboBox->addItem(id.replace('/', ' '), id);
	}

	if (ui.skinComboBox->count() == 1)
		ui.skinComboBox->setEnabled(FALSE);

	index = ui.skinComboBox->findData(NSettings::instance()->value("Skin"));
	if (index != -1)
		ui.skinComboBox->setCurrentIndex(index);
#endif

	NSettings::instance()->loadShortcuts();
	ui.shortcutEditorWidget->init(NSettings::instance()->shortcuts());
}

void NPreferencesDialog::saveSettings()
{
	NSettings::instance()->setValue("PlaylistTrackInfo", ui.playlistTrackInfoLineEdit->text());
	NSettings::instance()->setValue("WindowTitleTrackInfo", ui.windowTrackInfoLineEdit->text());
	NSettings::instance()->setValue("MinimizeToTray", ui.minimizeToTrayCheckBox->isChecked());
	NSettings::instance()->setValue("RestorePlayback", ui.restorePlaybackCheckBox->isChecked());
	NSettings::instance()->setValue("SingleInstanse", !ui.multipleInstansesCheckBox->isChecked());
	NSettings::instance()->setValue("TrayIcon", ui.trayIconCheckBox->isChecked());
	NSettings::instance()->setValue("AutoCheckUpdates", ui.versionCheckBox->isChecked());
	NSettings::instance()->setValue("DisplayLogDialog", ui.displayLogDialogCheckBox->isChecked());

	for (int i = 0; i < ui.waveformTrackInfoTable->rowCount(); ++i) {
		for (int j = 0; j < ui.waveformTrackInfoTable->columnCount(); ++j) {
			QString objecName = ui.waveformTrackInfoTable->verticalHeaderItem(i)->text() + ui.waveformTrackInfoTable->horizontalHeaderItem(j)->text();
			NSettings::instance()->setValue("TrackInfo/" + objecName, ui.waveformTrackInfoTable->item(i, j)->text());
		}
	}

#ifndef _N_NO_PLUGINS_
	// plugins
	NFlagIterator<N::PluginType> iter(N::MaxPlugin);
	while (iter.hasNext()) {
		iter.next();
		N::PluginType type = iter.value();
		QString typeString = ENUM_NAME(N, PluginType, type);
		QString containerName = selectedContainer(type);
		if (!containerName.isEmpty())
			NSettings::instance()->setValue(QString() + "Plugins/" + typeString, containerName);
	}
#endif

#ifndef _N_NO_SKINS_
	// skins
	QVariant skinVariant = ui.skinComboBox->itemData(ui.skinComboBox->currentIndex());
	NSettings::instance()->setValue("Skin", skinVariant);

	ui.skinRestartLabel->setVisible(NSettings::instance()->value("Skin").isValid() && skinVariant != NSettings::instance()->value("Skin"));
#endif

	ui.shortcutEditorWidget->applyShortcuts();
	NSettings::instance()->saveShortcuts();
	emit settingsChanged();

	if (ui.trayIconCheckBox->isChecked() && !QSystemTrayIcon::isSystemTrayAvailable())
		QMessageBox::warning(this, "Systray Error", QObject::tr("System Tray (Notification Area) is not available on your system."));
}

