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

#include "preferencesDialog.h"

#include "i18nLoader.h"
#include "player.h"
#include "plugin.h"
#include "settings.h"
#include "skinFileSystem.h"

#ifdef Q_OS_WIN
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
#include <QTextCodec>
#include <QVBoxLayout>

using namespace NPluginLoader;

static QStringList vNames = QStringList() << "Top"
                                          << "Middle"
                                          << "Bottom";
static QStringList hNames = QStringList() << "Left"
                                          << "Center"
                                          << "Right";
static const char *LANGUAGE = QT_TRANSLATE_NOOP("PreferencesDialog", "English");

NPreferencesDialog::~NPreferencesDialog() {}

NPreferencesDialog::NPreferencesDialog(QWidget *parent) : QDialog(parent)
{
    ui.setupUi(this);

    connect(ui.buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this,
            SLOT(saveSettings()));
    connect(this, SIGNAL(accepted()), this, SLOT(saveSettings()));
    ui.buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Ok"));
    ui.buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    ui.buttonBox->button(QDialogButtonBox::Apply)->setText(tr("Apply"));

    connect(ui.singleInstanceCheckBox, SIGNAL(toggled(bool)), ui.enqueueFilesCheckBox,
            SLOT(setEnabled(bool)));
    connect(ui.enqueueFilesCheckBox, SIGNAL(toggled(bool)), ui.playEnqueuedCheckBox,
            SLOT(setEnabled(bool)));
    connect(ui.singleInstanceCheckBox, SIGNAL(toggled(bool)), ui.playEnqueuedCheckBox,
            SLOT(setEnabled(bool)));
    connect(ui.restorePlaylistCheckBox, SIGNAL(toggled(bool)), ui.startPausedCheckBox,
            SLOT(setEnabled(bool)));
    connect(ui.customFileManagerCheckBox, SIGNAL(toggled(bool)),
            ui.customFileManagerCommandLineEdit, SLOT(setEnabled(bool)));
    connect(ui.customTrashCheckBox, SIGNAL(toggled(bool)), ui.customTrashCommandLineEdit,
            SLOT(setEnabled(bool)));

    setWindowTitle(QCoreApplication::applicationName() + tr(" Preferences"));

#ifdef _N_NO_SKINS_
    ui.skinLabel->hide();
    ui.skinComboBox->hide();
#endif

#ifndef Q_OS_WIN
    ui.taskbarProgressContainer->hide();
#endif

#ifdef _N_NO_UPDATE_CHECK_
    ui.autoCheckUpdatesContainer->hide();
#endif

#if defined Q_OS_WIN || defined Q_OS_MAC
    ui.customTrashContainer->hide();
#endif

#if defined Q_OS_MAC
    ui.minimizeToTrayContainer->hide();
#else
    ui.quitOnCloseContainer->hide();
#endif

    QVBoxLayout *scrollLayout = new QVBoxLayout;
    ui.pluginsScrollArea->widget()->setLayout(scrollLayout);

    NFlagIterator<N::PluginType> iter(N::MaxPlugin);
    while (iter.hasNext()) {
        iter.next();
        N::PluginType type = iter.value();
        QGroupBox *groupBox = createGroupBox(type);
        if (groupBox) {
            scrollLayout->addWidget(groupBox);
        }
    }

    if (scrollLayout->count() > 0) {
        scrollLayout->addItem(
            new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding));
    } else {
        ui.tabWidget->removeTab(ui.tabWidget->indexOf(ui.pluginsTab));
    }

    ui.pluginsRestartLabel->setText("⚠ " + ui.pluginsRestartLabel->text());
    ui.pluginsRestartLabel->setVisible(false);

    ui.languageRestartLabel->setText("⚠ " + ui.languageRestartLabel->text());
    ui.languageRestartLabel->setVisible(false);

    ui.skinRestartLabel->setText("⚠ " + ui.skinRestartLabel->text());
    ui.skinRestartLabel->setVisible(false);
    connect(ui.skinComboBox, SIGNAL(activated(int)), ui.skinRestartLabel, SLOT(show()));

    ui.waveformTrackInfoTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    int i = 0;
    foreach (int mib, QTextCodec::availableMibs()) {
        QString codecName = QTextCodec::codecForMib(mib)->name();
        ui.encodingTrackInfoComboBox->addItem(codecName, mib);
        if (codecName == "UTF-8") {
            ui.encodingTrackInfoComboBox->setCurrentIndex(i);
        }
        ++i;
    }
}

void NPreferencesDialog::showEvent(QShowEvent *event)
{
    loadSettings();
    QDialog::showEvent(event);
}

#ifndef _N_NO_UPDATE_CHECK_
void NPreferencesDialog::setVersionLabel(QString text)
{
    ui.versionLabel->setText(text);
}

void NPreferencesDialog::on_versionCheckButton_clicked()
{
    ui.versionLabel->setText("Checking...");
    emit versionRequested();
}
#endif

void NPreferencesDialog::on_fileManagerHelpButton_clicked()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("File Manager Configuration");

    QVBoxLayout *layout = new QVBoxLayout;
    dialog->setLayout(layout);

    QTextBrowser *textBrowser = new QTextBrowser(this);
    // clang-format off
    textBrowser->setHtml(
        tr("Supported parameters:") +
        "<ul>" +
            "<li><b>%F</b> - " + tr("File name") + "</li>" +
            "<li><b>%p</b> - " + tr("File name including absolute path") + "</li>" +
            "<li><b>%P</b> - " + tr("Directory path without file name") + "</li>" +
        "</ul>" +
        tr("Examples:") +
        "<ul style=\"font-family: 'Lucida Console', Monaco, monospace\">" +
            "<li>open -a '/Applications/Path Finder.app' '%p'</li>" +
            "<li>pcmanfm -n '%P' & sleep 1.5 && xdotool type '%F' && xdotool key Escape</li>" +
        "</ul>");
    // clang-format on
    textBrowser->setStyleSheet("QTextBrowser { background: transparent }");
    textBrowser->setFrameShape(QFrame::NoFrame);

    layout->addWidget(textBrowser);

    QPushButton *closeButton = new QPushButton(tr("Close"));
    connect(closeButton, SIGNAL(clicked()), dialog, SLOT(accept()));
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);

    dialog->show();
    dialog->resize(640, 300);
}

void NPreferencesDialog::on_customTrashHelpButton_clicked()
{
#if !defined Q_OS_WIN && !defined Q_OS_MAC
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Trash Command Configuration");

    QVBoxLayout *layout = new QVBoxLayout;
    dialog->setLayout(layout);

    QTextBrowser *textBrowser = new QTextBrowser(this);
    // clang-format off
    textBrowser->setHtml(
        tr("Supported parameters:") +
        "<ul>" +
            "<li><b>%F</b> - " + tr("File name") +
            "</li>" + "<li><b>%p</b> - " + tr("File name including absolute path") +
            "</li>" + "<li><b>%P</b> - " + tr("Directory path without file name") +
            "</li>" +
        "</ul>" +
        tr("Examples:") +
        "<ul style=\"font-family: 'Lucida Console', Monaco, monospace\">" +
            "<li>trash-put '%p'</li>" + "<li>mv '%p' $HOME/.Trash</li>" +
        "</ul>");
    // clang-format on
    textBrowser->setStyleSheet("QTextBrowser { background: transparent }");
    textBrowser->setFrameShape(QFrame::NoFrame);

    layout->addWidget(textBrowser);

    QPushButton *closeButton = new QPushButton(tr("Close"));
    connect(closeButton, SIGNAL(clicked()), dialog, SLOT(accept()));
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);

    dialog->show();
    dialog->resize(640, 300);
#endif
}

void NPreferencesDialog::on_titleFormatHelpButton_clicked()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Title Formats");

    QVBoxLayout *layout = new QVBoxLayout;
    dialog->setLayout(layout);

    QTextBrowser *textBrowser = new QTextBrowser(this);
    // clang-format off
    textBrowser->setHtml(
        tr("Supported parameters:") +
        "<ul>" +
            "<li><b>%a</b> - " + tr("Artist") + "</li>" +
            "<li><b>%t</b> - " + tr("Title") + "</li>" +
            "<li><b>%A</b> - " + tr("Album") + "</li>" +
            "<li><b>%c</b> - " + tr("Comment") + "</li>" +
            "<li><b>%g</b> - " + tr("Genre") + "</li>" +
            "<li><b>%y</b> - " + tr("Year") + "</li>" +
            "<li><b>%n</b> - " + tr("Track number") + "</li>" +
            "<li><b>%T</b> - " + tr("Current time position (Waveform only)") + "</li>" +
            "<li><b>%r</b> - " + tr("Remaining time (Waveform only)") + "</li>" +
            "<li><b>%C</b> - " + tr("Time position under cursor (Tooltip only)") + "</li>" +
            "<li><b>%o</b> - " + tr("Time offset under cursor (Tooltip only)") + "</li>" +
            "<li><b>%d</b> - " + tr("Duration in format hh:mm:ss") + "</li>" +
            "<li><b>%D</b> - " + tr("Duration in seconds") + "</li>" +
            "<li><b>%b</b> - " + tr("Bit depth") + "</li>" +
            "<li><b>%B</b> - " + tr("Bitrate in Kbps") + "</li>" +
            "<li><b>%s</b> - " + tr("Sample rate in kHz") + "</li>" +
            "<li><b>%H</b> - " + tr("Number of channels") + "</li>" +
            "<li><b>%f</b> - " + tr("File name without extension") + "</li>" +
            "<li><b>%F</b> - " + tr("File name") + "</li>" +
            "<li><b>%p</b> - " + tr("File name including absolute path") + "</li>" +
            "<li><b>%P</b> - " + tr("Directory path without file name") + "</li>" +
            "<li><b>%e</b> - " + tr("File name extension") + "</li>" +
            "<li><b>%E</b> - " + tr("File name extension, uppercased") + "</li>" +
            "<li><b>%v</b> - " + tr("Nulloy version number") + "</li>" +
            "<li><b>%%</b> - " + tr("\'%\' character") + "</li>" +
        "</ul>" +
        tr("Conditions:") +
        "<ul>" +
            "<li><b>{<i>" + tr("true") + "</i>|<i>" + tr("false") + "</i>}</b> - " + tr("Evaluate for <b><i>true</i></b> or <b><i>false</i></b> case. Note: nesting conditions is not supported yet.") + "</li>"
        "</ul>" +
        tr("Examples:") +
        "<ul>" +
            "<li><b>{%a - %t|%F}</b> - " + tr("Print Artist and Title, separated with \"-\". If either of the tags is not available, print file name instead.") + "</li>"
            "<li><b>{%g|}</b> - " + tr("Print Genre. If not available, print nothing.") + "</li>" +
        "</ul>");
    // clang-format on
    textBrowser->setStyleSheet("QTextBrowser { background: transparent }");
    textBrowser->setFrameShape(QFrame::NoFrame);

    layout->addWidget(textBrowser);

    QPushButton *closeButton = new QPushButton(tr("Close"));
    connect(closeButton, SIGNAL(clicked()), dialog, SLOT(accept()));
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);

    dialog->show();
    dialog->resize(640, 480);
}

QGroupBox *NPreferencesDialog::createGroupBox(N::PluginType type)
{
    QList<Descriptor> descriptors = NPluginLoader::descriptors();

    QString typeString = ENUM_TO_STR(N, PluginType, type);
    QString settingsContainer = NSettings::instance()->value("Plugins/" + typeString).toString();

    QList<int> indexesFilteredByType;
    for (int i = 0; i < descriptors.count(); ++i) {
        if (descriptors.at(i)[TypeRole] == type) {
            indexesFilteredByType << i;
        }
    }

    if (indexesFilteredByType.count() < 2) { // need at least two plugins to choose from
        return NULL;
    }

    QGroupBox *groupBox = new QGroupBox(typeString);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 5, 5, 0);
    layout->setSpacing(0);
    groupBox->setLayout(layout);

    foreach (int i, indexesFilteredByType) {
        QString containerName = descriptors.at(i)[ContainerNameRole].toString();
        QRadioButton *button = new QRadioButton(containerName);
        connect(button, SIGNAL(toggled(bool)), ui.pluginsRestartLabel, SLOT(show()));
        if (containerName == settingsContainer) {
            button->setChecked(true);
        }
        m_radioButtons[button] = descriptors.at(i);
        layout->addWidget(button);
    }

    return groupBox;
}

void NPreferencesDialog::on_languageComboBox_activated(int index)
{
    ui.languageRestartLabel->setVisible(true);

    QLocale locale = ui.languageComboBox->itemData(index).toLocale();
    QString newText = NI18NLoader::translate(locale.language(), "PreferencesDialog",
                                             "Switching languages requires restart");
    ui.languageRestartLabel->setText(
        ui.languageRestartLabel->text().replace(QRegExp("(.*)&nbsp;.*"), "\\1&nbsp;" + newText));
}

QString NPreferencesDialog::selectedContainer(N::PluginType type)
{
    QList<Descriptor> descriptors = NPluginLoader::descriptors();
    QList<int> indexesFilteredByType;
    for (int i = 0; i < descriptors.count(); ++i) {
        if (descriptors.at(i)[TypeRole] == type) {
            indexesFilteredByType << i;
        }
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
    QList<QWidget *> widgets = findChildren<QWidget *>();
    foreach (QWidget *widget, widgets) {
        if (!widget->inherits("QCheckBox") && !widget->inherits("QLineEdit") &&
            !widget->inherits("QAbstractSpinBox")) {
            continue;
        }
        QString className = QString(widget->metaObject()->className()).mid(1); // remove leading 'Q'
        QString objectName = widget->objectName();
        if (objectName == "qt_spinbox_lineedit") { // QSpinBox inner widget
            continue;
        }
        if (objectName.startsWith("tooltipOffset")) {
            continue;
        }
        QString settingsName = objectName;
        settingsName.remove(className);
        settingsName[0] = settingsName.at(0).toUpper();
        if (widget->inherits("QCheckBox")) {
            qobject_cast<QCheckBox *>(widget)->setChecked(
                NSettings::instance()->value(settingsName).toBool());
        } else if (widget->inherits("QLineEdit")) {
            qobject_cast<QLineEdit *>(widget)->setText(
                NSettings::instance()->value(settingsName).toString());
        } else if (widget->inherits("QDoubleSpinBox")) {
            qobject_cast<QDoubleSpinBox *>(widget)->setValue(
                NSettings::instance()->value(settingsName).toDouble());
        }
    }

    ui.startPausedCheckBox->setEnabled(NSettings::instance()->value("RestorePlaylist").toBool());
    ui.enqueueFilesCheckBox->setEnabled(NSettings::instance()->value("SingleInstance").toBool());
    ui.playEnqueuedCheckBox->setEnabled(NSettings::instance()->value("SingleInstance").toBool() &&
                                        NSettings::instance()->value("EnqueueFiles").toBool());
    ui.customFileManagerCommandLineEdit->setEnabled(
        NSettings::instance()->value("CustomFileManager").toBool());
    ui.customTrashCommandLineEdit->setEnabled(NSettings::instance()->value("CustomTrash").toBool());
    ui.versionLabel->setText("");
    // << general

    // track info overlay >>
    for (int i = 0; i < ui.waveformTrackInfoTable->rowCount(); ++i) {
        for (int j = 0; j < ui.waveformTrackInfoTable->columnCount(); ++j) {
            QString objecName = ui.waveformTrackInfoTable->verticalHeaderItem(i)->text() +
                                ui.waveformTrackInfoTable->horizontalHeaderItem(j)->text();
            QTableWidgetItem *item = new QTableWidgetItem(
                NSettings::instance()->value("TrackInfo/" + vNames.at(i) + hNames.at(j)).toString());
            item->setTextAlignment(Qt::AlignCenter);
            ui.waveformTrackInfoTable->setItem(i, j, item);
        }
    }

    int height = ui.waveformTrackInfoTable->horizontalHeader()->height();
    for (int i = 0; i < ui.waveformTrackInfoTable->rowCount(); ++i) {
        height += ui.waveformTrackInfoTable->rowHeight(i);
    }
    height += 2; // frame
    ui.waveformTrackInfoTable->setMaximumHeight(height);
    // << track info overlay

    // track info encoding >>
    QString encoding = NSettings::instance()->value("EncodingTrackInfo").toString();
    for (int i = 0; i < ui.encodingTrackInfoComboBox->count(); ++i) {
        if (ui.encodingTrackInfoComboBox->itemText(i) == encoding) {
            ui.encodingTrackInfoComboBox->setCurrentIndex(i);
            break;
        }
    }
    // << track info encoding

    // tooltip offset >>
    QStringList offsetList = NSettings::instance()->value("TooltipOffset").toStringList();
    ui.tooltipOffsetXSpinBox->setValue(offsetList.at(0).toInt());
    ui.tooltipOffsetYSpinBox->setValue(offsetList.at(1).toInt());
    // << tooltip offset

    // skins >>
#ifndef _N_NO_SKINS_
    int skinIndex;
    ui.skinComboBox->clear();
    foreach (QString str, NSkinLoader::skinIdentifiers()) {
        QString id = str.section('/', 2);
        QString userData = id;
        QString text = id.replace('/', ' ').replace(" (Built-in)", tr(" (Built-in)"));
        ui.skinComboBox->addItem(text, userData);
    }

    if (ui.skinComboBox->count() == 1) {
        ui.skinComboBox->setEnabled(false);
    }

    skinIndex = ui.skinComboBox->findData(NSettings::instance()->value("Skin"));
    if (skinIndex != -1) {
        ui.skinComboBox->setCurrentIndex(skinIndex);
    }
#endif
    // << skins

    // translations >>
    int localeIndex;
    ui.languageComboBox->clear();
    foreach (QLocale::Language language, NI18NLoader::translations()) {
        QString languageString = QLocale::languageToString(language);
        QString localizedString = NI18NLoader::translate(language, "PreferencesDialog", LANGUAGE);
        ui.languageComboBox->addItem(QString("%1 (%2)").arg(localizedString).arg(languageString),
                                     QLocale(language));
    }

    if (ui.languageComboBox->count() == 1) {
        ui.languageComboBox->setEnabled(false);
    }

    localeIndex = ui.languageComboBox->findData(
        QLocale(NSettings::instance()->value("Language").toString()));
    if (localeIndex != -1) {
        ui.languageComboBox->setCurrentIndex(localeIndex);
    }
    // << translations

    // shortcuts >>
    ui.shortcutEditorWidget->init(NSettings::instance()->shortcuts());
    // << shortcuts
}

void NPreferencesDialog::saveSettings()
{
    // general >>
    QList<QWidget *> widgets = findChildren<QWidget *>();
    foreach (QWidget *widget, widgets) {
        if (!widget->inherits("QCheckBox") && !widget->inherits("QLineEdit") &&
            !widget->inherits("QAbstractSpinBox")) {
            continue;
        }
        QString className = QString(widget->metaObject()->className()).mid(1); // remove leading 'Q'
        QString objectName = widget->objectName();
        if (objectName == "qt_spinbox_lineedit") { // QSpinBox inner widget
            continue;
        }
        if (objectName.startsWith("tooltipOffset")) {
            continue;
        }
        QString settingsName = objectName;
        settingsName.remove(className);
        settingsName[0] = settingsName.at(0).toUpper();
        if (widget->inherits("QCheckBox")) {
            NSettings::instance()->setValue(settingsName,
                                            qobject_cast<QCheckBox *>(widget)->isChecked());
        } else if (widget->inherits("QLineEdit")) {
            NSettings::instance()->setValue(settingsName, qobject_cast<QLineEdit *>(widget)->text());
        } else if (widget->inherits("QDoubleSpinBox")) {
            NSettings::instance()->setValue(settingsName,
                                            qobject_cast<QDoubleSpinBox *>(widget)->value());
        }
    }

#ifdef Q_OS_WIN
    NW7TaskBar::instance()->setEnabled(NSettings::instance()->value("TaskbarProgress").toBool());
#endif
    // << general

    // track info overlay >>
    for (int i = 0; i < ui.waveformTrackInfoTable->rowCount(); ++i) {
        for (int j = 0; j < ui.waveformTrackInfoTable->columnCount(); ++j) {
            QString objecName = ui.waveformTrackInfoTable->verticalHeaderItem(i)->text() +
                                ui.waveformTrackInfoTable->horizontalHeaderItem(j)->text();
            NSettings::instance()->setValue("TrackInfo/" + vNames.at(i) + hNames.at(j),
                                            ui.waveformTrackInfoTable->item(i, j)->text());
        }
    }
    // << track info overlay

    // track info encoding >>
    NSettings::instance()->setValue("EncodingTrackInfo",
                                    ui.encodingTrackInfoComboBox->itemText(
                                        ui.encodingTrackInfoComboBox->currentIndex()));
    // << track info encoding

    // tooltip offset >>
    int tooltipOffsetX = ui.tooltipOffsetXSpinBox->value();
    int tooltipOffsetY = ui.tooltipOffsetYSpinBox->value();
    NSettings::instance()->setValue("TooltipOffset", QStringList()
                                                         << QString::number(tooltipOffsetX)
                                                         << QString::number(tooltipOffsetY));
    // << tooltip offset

    // plugins >>
    NFlagIterator<N::PluginType> iter(N::MaxPlugin);
    while (iter.hasNext()) {
        iter.next();
        N::PluginType type = iter.value();
        QString typeString = ENUM_TO_STR(N, PluginType, type);
        QString containerName = selectedContainer(type);
        if (!containerName.isEmpty()) {
            NSettings::instance()->setValue(QString() + "Plugins/" + typeString, containerName);
        }
    }
    // << plugins

    // skins >>
#ifndef _N_NO_SKINS_
    NSettings::instance()->setValue("Skin",
                                    ui.skinComboBox->itemData(ui.skinComboBox->currentIndex()));
#endif
    // << skins

    // translations >>
    NSettings::instance()->setValue("Language", ui.languageComboBox
                                                    ->itemData(ui.languageComboBox->currentIndex())
                                                    .toLocale()
                                                    .bcp47Name()
                                                    .split('-')
                                                    .first());
    // << translations

    // shortcuts >>
    ui.shortcutEditorWidget->applyShortcuts();
    NSettings::instance()->saveShortcuts();
    emit settingsChanged();
    // << shortcuts

    // systray check >>
    if (ui.trayIconCheckBox->isChecked() && !QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::warning(this, "Systray Error",
                             QObject::tr("System Tray (Notification Area) is "
                                         "not available on your system."));
    }
    // << systray check
}
