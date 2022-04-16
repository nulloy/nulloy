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

#include "tagEditorDialog.h"

#include "coverWidget.h"
#include "pluginLoader.h"
#include "settings.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMap>
#include <QMessageBox>
#include <QMetaEnum>
#include <QPushButton>
#include <QTextEdit>

NTagEditorDialog::NTagEditorDialog(const QString &file, QWidget *parent) : QDialog(parent)
{
    ui.setupUi(this);

    m_file = file;
    m_tagReader = dynamic_cast<NTagReaderInterface *>(NPluginLoader::getPlugin(N::TagReader));

    QString settingsEncoding = NSettings::instance()->value("EncodingTrackInfo").toString();
    m_tagReader->setEncoding(settingsEncoding);
    m_tagReader->setSource(m_file);

    QMap<QString, QStringList> tags = m_tagReader->getTags();
    if (!tags.isEmpty()) {
        if (tags.value("Error").join("") == "Invalid") {
            QMessageBox msgBox(QMessageBox::Information, tr("Unsupported File"),
                               "This file format does not support tags." +
                                   QString("%1").arg(" ", 20, ' '),
                               QMessageBox::Close, parent);
            msgBox.exec();
            reject();
            return;
        }
    }

    connect(ui.buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this,
            SLOT(onSaveClicked()));

    m_coverReader = dynamic_cast<NCoverReaderInterface *>(NPluginLoader::getPlugin(N::CoverReader));
    m_encodingUtf8Index = -1;
    m_hasChanges = false;

    ui.tabWidget->setCornerWidget(ui.encodingFrame);
    int i = 0;
    foreach (int mib, QTextCodec::availableMibs()) {
        QString codecName = QTextCodec::codecForMib(mib)->name();
        ui.encodingComboBox->addItem(codecName, mib);
        if (codecName == "UTF-8") {
            m_encodingUtf8Index = i;
        }
        if (codecName == settingsEncoding) {
            ui.encodingComboBox->setCurrentIndex(i);
            m_encodingPreviousIndex = i;
            m_encodingSettingsIndex = i;
        }
        ++i;
    }
    connect(ui.encodingResetButton, &QPushButton::clicked, [=] {
        ui.encodingComboBox->setCurrentIndex(m_encodingUtf8Index);
        on_encodingComboBox_activated(-1);
    });
    connect(ui.editAsUtf8Button, &QPushButton::clicked, [=] {
        ui.encodingComboBox->setCurrentIndex(m_encodingUtf8Index);
        m_encodingPreviousIndex = m_encodingUtf8Index;
        m_hasChanges = true;
        setReadOnlyMode(false);
    });
    on_encodingComboBox_activated(-1);

    setWindowTitle("\"" + QFileInfo(file).fileName() + "\" — " + tr("Tag Editor"));

    m_coverReader->setSource(m_file);
    QList<QImage> images = m_coverReader->getImages();
    for (QImage image : images) {
        QLabel *label = new QLabel;
        label->setFixedSize(100, 100);
        label->setScaledContents(true);
        label->setPixmap(QPixmap::fromImage(image));
        QListWidgetItem *widgetItem = new QListWidgetItem;
        widgetItem->setSizeHint(label->size());
        ui.artworkListWidget->addItem(widgetItem);
        ui.artworkListWidget->setItemWidget(widgetItem, label);
    }
    ui.artworkListWidget->setMinimumHeight(111);

    ui.buttonBox->button(QDialogButtonBox::Reset)->setText(tr("Revert"));
    connect(ui.buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked, [=] {
        ui.encodingComboBox->setCurrentIndex(m_encodingSettingsIndex);
        readTags();
        setReadOnlyMode(ui.encodingComboBox->currentIndex() != m_encodingUtf8Index);
    });

    exec();
}

void NTagEditorDialog::setReadOnlyMode(bool readOnly)
{
    ui.generalTab->setEnabled(!readOnly);
    ui.rawTagsTab->setEnabled(!readOnly);
    ui.encodingResetButton->setVisible(readOnly);
    ui.readOnlyLabel->setVisible(readOnly);
    ui.editAsUtf8Button->setVisible(readOnly);
    ui.buttonBox->button(QDialogButtonBox::Save)->setEnabled(!readOnly && m_hasChanges);
    ui.buttonBox->button(QDialogButtonBox::Reset)->setEnabled(!readOnly && m_hasChanges);
}

void NTagEditorDialog::on_encodingComboBox_activated(int index)
{
    if (m_hasChanges) {
        ui.encodingComboBox->setCurrentIndex(m_encodingPreviousIndex);
        QMessageBox msgBox(QMessageBox::Warning, tr("Warning"),
                           tr("Tags have been modified.") + QString("%1").arg(" ", 50, ' '),
                           QMessageBox::Close, this);
        msgBox.setInformativeText(tr("Save or revert the changes before switching encoding."));
        msgBox.exec();
        return;
    }
    if (index == -1) {
        index = ui.encodingComboBox->currentIndex();
    }

    setReadOnlyMode(index != m_encodingUtf8Index);
    m_encodingPreviousIndex = index;

    readTags();
}

void NTagEditorDialog::readTags()
{
    m_hasChanges = false;
    QString encoding = ui.encodingComboBox->itemText(ui.encodingComboBox->currentIndex());
    m_tagReader->setEncoding(encoding);

    QLayout *oldFormLayout = ui.rawTagsScrollArea->widget()->layout();
    if (oldFormLayout) {
        delete oldFormLayout;
        qDeleteAll(ui.rawTagsScrollArea->widget()->children());
    }

    // create raw widgets for standard tags:
    QRegularExpression widgetNameRegex("Tag");
    QMap<QString, QStringList> tags = m_tagReader->getTags();
    QFormLayout *rawTagsFormLayout = new QFormLayout;
    rawTagsFormLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    QMetaEnum enumerator = ENUMERATOR(N, Tag);
    for (int index = 1; index < enumerator.keyCount(); ++index) { // skip UnknownTag
        QString enumKey = enumerator.key(index);

        // create line edit for standard raw tag(s):
        QString tagKey = m_tagReader->tagToKey((N::Tag)enumerator.value(index));
        QString label = tagKey + ":";
        QStringList values = tags.take(tagKey);
        if (values.isEmpty()) { // create at least one line edit
            values << "";
        }
        foreach (QString value, values) {
            QLineEdit *rawTagLineEdit = new QLineEdit(value);
            rawTagsFormLayout->addRow(label, rawTagLineEdit);

            connect(rawTagLineEdit, &QLineEdit::textEdited, [=](const QString &newValue) {
                m_hasChanges = true;
                ui.buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);
                ui.buttonBox->button(QDialogButtonBox::Reset)->setEnabled(true);
            });

            if (label.isEmpty()) { // connect widgets only for the first tag value
                continue;
            }

            QString widgetNamePrefix = enumKey.replace(0, 1, enumKey[0].toLower());
            if (QLineEdit *standardTagLineEdit = ui.generalTab->findChild<QLineEdit *>(
                    widgetNamePrefix + "LineEdit")) {
                standardTagLineEdit->setText(values.first());
                QMetaObject::Connection connection =
                    connect(standardTagLineEdit, &QLineEdit::textEdited,
                            [=](const QString &newValue) {
                                rawTagLineEdit->setText(newValue);
                                m_hasChanges = true;
                                ui.buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);
                                ui.buttonBox->button(QDialogButtonBox::Reset)->setEnabled(true);
                            });
                connect(rawTagLineEdit, &QObject::destroyed,
                        [=] { QObject::disconnect(connection); });
                connect(rawTagLineEdit, &QLineEdit::textEdited,
                        [=](const QString &newValue) { standardTagLineEdit->setText(newValue); });
            } else if (QPlainTextEdit *standardTagPlainTextEdit =
                           ui.generalTab->findChild<QPlainTextEdit *>(widgetNamePrefix +
                                                                      "PlainTextEdit")) {
                standardTagPlainTextEdit->setPlainText(values.first());
                QMetaObject::Connection connection =
                    connect(standardTagPlainTextEdit, &QPlainTextEdit::textChanged, [=] {
                        rawTagLineEdit->setText(standardTagPlainTextEdit->toPlainText());
                        m_hasChanges = true;
                        ui.buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);
                        ui.buttonBox->button(QDialogButtonBox::Reset)->setEnabled(true);
                    });
                connect(rawTagLineEdit, &QObject::destroyed,
                        [=] { QObject::disconnect(connection); });
                connect(rawTagLineEdit, &QLineEdit::textEdited, [=](const QString &newValue) {
                    QTextCursor cursor = standardTagPlainTextEdit->textCursor();
                    standardTagPlainTextEdit->setPlainText(newValue);
                    standardTagPlainTextEdit->setTextCursor(cursor);
                });
            } else {
                QMessageBox::critical(NULL, "Match error", "No widget for tag: " + enumKey,
                                      QMessageBox::Close);
            }

            label = "";
        }
    }

    // create line edits for the rest for raw tags:
    foreach (QString tagKey, tags.keys()) {
        QString label = tagKey + ":";
        QStringList values = tags.take(tagKey);

        foreach (QString value, values) {
            QLineEdit *rawTagLineEdit = new QLineEdit(value);
            rawTagsFormLayout->addRow(label, rawTagLineEdit);

            label = "";
        }
    }

    ui.rawTagsScrollArea->widget()->setLayout(rawTagsFormLayout);
    ui.buttonBox->button(QDialogButtonBox::Reset)->setEnabled(false);
    ui.buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
}

bool NTagEditorDialog::writeTags()
{
    QMap<QString, QStringList> tags;

    QFormLayout *rawTagsFormLayout = qobject_cast<QFormLayout *>(
        ui.rawTagsScrollArea->widget()->layout());

    QString tagName;
    QStringList valueList;

    for (size_t i = 0; i < rawTagsFormLayout->rowCount(); ++i) {
        QLayoutItem *item = rawTagsFormLayout->itemAt(i, QFormLayout::LabelRole);
        if (item) {
            if (!valueList.isEmpty()) {
                tags[tagName] = valueList;
                valueList.clear();
            }
            QLabel *label = qobject_cast<QLabel *>(item->widget());
            tagName = label->text().chopped(1);
        }
        QString value = qobject_cast<QLineEdit *>(
                            rawTagsFormLayout->itemAt(i, QFormLayout::FieldRole)->widget())
                            ->text();
        if (!value.isEmpty()) {
            valueList << value;
        }
    }
    if (!valueList.isEmpty()) {
        tags[tagName] = valueList;
        valueList.clear();
    }

    QMap<QString, QStringList> unsaved = m_tagReader->setTags(tags);
    if (!unsaved.isEmpty()) {
        if (unsaved.value("Error").join("") == "Write") {
            QMessageBox msgBox(QMessageBox::Critical, tr("Write Fail"),
                               "Write operation did not succeed.", QMessageBox::Close, this);
            msgBox.exec();
        } else {
            QMessageBox msgBox(QMessageBox::Warning, tr("Save Fail"),
                               "Saving aborted. Failed tags: " + unsaved.keys().join(", "),
                               QMessageBox::Close, this);
            msgBox.exec();
        }
        return false;
    }

    return true;
}

NTagEditorDialog::~NTagEditorDialog() {}

void NTagEditorDialog::onSaveClicked()
{
    if (NSettings::instance()->value("DisplayTagEditorConfirmDialog").toBool()) {
        QCheckBox *checkBox = new QCheckBox(tr("Don't show this dialog anymore"));

        QMessageBox msgBox(QMessageBox::Question, tr("Confirmation"),
                           tr("Do you want to save your changes?"),
                           QMessageBox::Save | QMessageBox::Cancel, this);
        msgBox.setDefaultButton(QMessageBox::Save);
        msgBox.setCheckBox(checkBox);
        int res = msgBox.exec();

        if (res != QMessageBox::Save) {
            return;
        }

        NSettings::instance()->setValue("DisplayTagEditorConfirmDialog", !checkBox->isChecked());
    }

    if (writeTags()) {
        readTags();
    }
}
