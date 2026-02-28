/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2025 Sergey Vlasov <sergey@vlasov.me>
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

#include "tagEditorDialogHandler.h"

#include "common.h"
#include "image.h"
#include "messageBox.h"
#include "pluginLoader.h"
#include "settings.h"

#include <QMessageBox>
#include <QQmlPropertyMap>
#include <QTextCodec>

NTagEditorDialogHandler::NTagEditorDialogHandler(const QString &file, QObject *parentWindow)
    : NDialogHandler(NCore::qmlUrl("tagEditorDialog.qml"), parentWindow)
{
    m_root = nullptr;
    m_file = file;
    m_tagReader = dynamic_cast<NTagReaderInterface *>(NPluginLoader::getPlugin(N::TagReader));
    m_coverReader = dynamic_cast<NCoverReaderInterface *>(NPluginLoader::getPlugin(N::CoverReader));
    m_readOnly = false;
    m_modifiedAndUnsaved = false;

    {
        int index = 0;
        m_encodingUtf8Index = -1;
        QString settingsEncoding = NSettings::instance()->value("EncodingTrackInfo").toString();
        for (int mib : QTextCodec::availableMibs()) {
            QString codecName = QTextCodec::codecForMib(mib)->name();
            m_encodings << codecName;
            if (codecName == "UTF-8") {
                m_encodingUtf8Index = index;
            }
            if (codecName == settingsEncoding) {
                m_encodingSettingsIndex = index;
            }
            ++index;
        }
    }

    m_tagsPropertyMap = new QQmlPropertyMap(this);
    connect(m_tagsPropertyMap, &QQmlPropertyMap::valueChanged,
            [this]() { setModifiedAndUnsaved(true); });

    connect(this, &NDialogHandler::setupContext, [this](QQmlContext *context) {
        context->setContextProperty("NTagEditorDialogHandler", this);
        context->setContextProperty("NTags", m_tagsPropertyMap);
    });

    connect(this, &NDialogHandler::setupRoot, [this](QObject *root) {
        m_root = root;

        QObject *buttonBox = root->findChild<QObject *>("buttonBox");
        connect(buttonBox, SIGNAL(applied()), this, SLOT(on_saveClicked()));
        connect(buttonBox, SIGNAL(reset()), this, SLOT(on_revertClicked()));
    });

    m_encodingPreviousIndex = m_encodingSettingsIndex;
    setEncodingCurrentIndex(m_encodingSettingsIndex);
}

NTagEditorDialogHandler::~NTagEditorDialogHandler()
{
    destroyEngine();
}

void NTagEditorDialogHandler::editAsUtf8()
{
    m_encodingPreviousIndex = m_encodingUtf8Index;
    m_encodingCurrentIndex = m_encodingUtf8Index;
    emit encodingCurrentIndexChanged();

    setModifiedAndUnsaved(true);
    setReadOnly(false);
}

void NTagEditorDialogHandler::switchEncodingToUtf8()
{
    setEncodingCurrentIndex(m_encodingUtf8Index);
}

void NTagEditorDialogHandler::on_revertClicked()
{
    setEncodingCurrentIndex(m_encodingSettingsIndex);
    readTags();
    setReadOnly(m_encodingCurrentIndex != m_encodingUtf8Index);
}

bool NTagEditorDialogHandler::isReadOnly() const
{
    return m_readOnly;
}

void NTagEditorDialogHandler::setReadOnly(bool readOnly)
{
    m_readOnly = m_encodingCurrentIndex != m_encodingUtf8Index;
    emit readOnlyChanged();
}

void NTagEditorDialogHandler::setModifiedAndUnsaved(bool modifiedAndUnsaved)
{
    m_modifiedAndUnsaved = modifiedAndUnsaved;
    emit modifiedAndUnsavedChanged();
}

bool NTagEditorDialogHandler::isModifiedAndUnsaved() const
{
    return m_modifiedAndUnsaved;
}

QStringList NTagEditorDialogHandler::encodings() const
{
    return m_encodings;
}

int NTagEditorDialogHandler::encodingCurrentIndex() const
{
    return m_encodingCurrentIndex;
}

void NTagEditorDialogHandler::setEncodingCurrentIndex(int index)
{
    if (m_encodingCurrentIndex == index) {
        return;
    }

    if (m_modifiedAndUnsaved) {
        m_encodingCurrentIndex = m_encodingPreviousIndex;
        emit encodingCurrentIndexChanged();

        NMessageBox *msgBox =
            new NMessageBox(tr("Warning"),
                            tr("Tags have been modified.") + "\n\n" +
                                tr("Save or revert the changes before switching encoding."),
                            QMessageBox::Ok, m_root);
        connect(msgBox, &NMessageBox::closed, msgBox, &QObject::deleteLater);
        msgBox->showDialog();
        return;
    }

    m_encodingPreviousIndex = m_encodingCurrentIndex;
    m_encodingCurrentIndex = index;
    emit encodingCurrentIndexChanged();

    readTags();
    setReadOnly(index != m_encodingUtf8Index);
}

QVariantList NTagEditorDialogHandler::coverImages() const
{
    QVariantList imageList;
    m_coverReader->setSource(m_file);
    QList<QImage> images = m_coverReader->getImages();
    for (const QImage &image : images) {
        imageList.append(image);
    }
    m_coverReader->setSource(""); // release the file
    return imageList;
}

void NTagEditorDialogHandler::readTags()
{
    QString encoding = m_encodings.at(m_encodingCurrentIndex);
    m_tagReader->setEncoding(encoding);
    m_tagReader->setSource(m_file);

    QMap<QString, QStringList> tags = m_tagReader->getTags();
    m_tagReader->setSource(""); // release the file
    if (!tags.isEmpty()) {
        if (tags.value("Error").join("") == "Invalid") {
            NMessageBox *msgBox = new NMessageBox(tr("Unsupported File"),
                                                  tr("This file format does not support tags."),
                                                  QMessageBox::Ok, m_root);
            connect(msgBox, &NMessageBox::closed, msgBox, &QObject::deleteLater);
            msgBox->showDialog();
            return;
        }
    }

    // add to property map the standard tags first:
    QMetaEnum enumerator = ENUMERATOR(N, Tag);
    for (int index = 1; index < enumerator.keyCount(); ++index) { // skip UnknownTag
        QString enumKey = enumerator.key(index);

        QString tagName = m_tagReader->tagToKey((N::Tag)enumerator.value(index));
        QStringList values = tags.take(tagName); // removes it from the map
        if (values.isEmpty()) {                  // to have at least one value
            values << "";
        }

        m_tagsPropertyMap->insert(tagName, values);
    }
    // add rest of the tags (if any):
    for (const QString &tagName : tags.keys()) {
        QStringList values = tags.value(tagName);
        m_tagsPropertyMap->insert(tagName, values);
    }

    setModifiedAndUnsaved(false);
}

bool NTagEditorDialogHandler::writeTags()
{
    m_tagReader->setSource(m_file);
    QMap<QString, QStringList> tags = m_tagReader->getTags();
    auto tagsBackup = tags;

    for (const QString &tagName : m_tagsPropertyMap->keys()) {
        tags[tagName] = m_tagsPropertyMap->value(tagName).toStringList();
    }

    QMap<QString, QStringList> unsaved = m_tagReader->setTags(tags);
    m_tagReader->setSource(""); // release the file
    if (!unsaved.isEmpty()) {
        QString title;
        QString text;
        if (unsaved.value("Error").join("") == "Write") {
            title = tr("Write Fail");
            text = tr("Write operation did not succeed.");
        } else {
            title = tr("Save Fail");
            text = tr("Saving aborted. Failed tags: %1").arg(unsaved.keys().join(", "));
        }
        NMessageBox *msgBox = new NMessageBox(title, text, QMessageBox::Close, m_root);
        connect(msgBox, &NMessageBox::closed, msgBox, &QObject::deleteLater);
        msgBox->showDialog();
        return false;
    }

    return true;
}

void NTagEditorDialogHandler::on_saveClicked()
{
    if (NSettings::instance()->value("DisplayTagEditorConfirmDialog").toBool()) {
        NMessageBox *msgBox = new NMessageBox(tr("Confirmation"),
                                              tr("Do you want to save your changes?"),
                                              QMessageBox::Save | QMessageBox::Cancel, m_root);
        msgBox->setCheckBoxText(tr("Don't show this dialog anymore"));

        connect(msgBox, &NMessageBox::accepted, this, [this, msgBox]() {
            NSettings::instance()->setValue("DisplayTagEditorConfirmDialog",
                                            !msgBox->isCheckBoxChecked());
            if (writeTags()) {
                readTags();
            }
        });
        connect(msgBox, &NMessageBox::closed, msgBox, &QObject::deleteLater);
        msgBox->showDialog();
    } else {
        if (writeTags()) {
            readTags();
        }
    }
}
