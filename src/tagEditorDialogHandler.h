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

#ifndef N_TAG_EDITOR_DIALOG_HANDLER_H
#define N_TAG_EDITOR_DIALOG_HANDLER_H

#include "dialogHandler.h"

#include "coverReaderInterface.h"
#include "global.h"
#include "pluginLoader.h"
#include "tagReaderInterface.h"

class QQmlPropertyMap;
class NImage;

class NTagEditorDialogHandler : public NDialogHandler
{
    Q_OBJECT
    Q_PROPERTY(QStringList encodings READ encodings CONSTANT)
    Q_PROPERTY(int encodingCurrentIndex READ encodingCurrentIndex WRITE setEncodingCurrentIndex
                   NOTIFY encodingCurrentIndexChanged)
    Q_PROPERTY(bool readOnly READ isReadOnly NOTIFY readOnlyChanged)
    Q_PROPERTY(bool modifiedAndUnsaved READ isModifiedAndUnsaved NOTIFY modifiedAndUnsavedChanged)

public:
    NTagEditorDialogHandler(const QString &file, QObject *parentWindow = nullptr);
    ~NTagEditorDialogHandler() override;
    Q_INVOKABLE QVariantList coverImages() const;
    Q_INVOKABLE void editAsUtf8();
    Q_INVOKABLE void switchEncodingToUtf8();

private:
    QObject *m_parentWindow;
    QObject *m_root;
    NTagReaderInterface *m_tagReader;
    NCoverReaderInterface *m_coverReader;
    QString m_file;
    QQmlPropertyMap *m_tagsPropertyMap;

    QStringList m_encodings;
    int m_encodingPreviousIndex;
    int m_encodingCurrentIndex;
    int m_encodingUtf8Index;
    int m_encodingSettingsIndex;
    QStringList encodings() const;
    int encodingCurrentIndex() const;
    void setEncodingCurrentIndex(int index);

    void readTags();
    bool writeTags();

    bool m_readOnly;
    void setReadOnly(bool readOnly);
    bool isReadOnly() const;

    bool m_modifiedAndUnsaved;
    void setModifiedAndUnsaved(bool modifiedAndUnsaved);
    bool isModifiedAndUnsaved() const;

private slots:
    void on_revertClicked();
    void on_saveClicked();

signals:
    void encodingCurrentIndexChanged();
    void readOnlyChanged();
    void modifiedAndUnsavedChanged();
};

#endif
