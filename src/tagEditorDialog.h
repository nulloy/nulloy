/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2024 Sergey Vlasov <sergey@vlasov.me>
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

#ifndef N_TAG_EDITOR_DIALOG_H
#define N_TAG_EDITOR_DIALOG_H

#include <QDialog>

#include "coverReaderInterface.h"
#include "global.h"
#include "pluginLoader.h"
#include "tagReaderInterface.h"
#include "ui_tagEditorDialog.h"

class QEvent;

class NTagEditorDialog : public QDialog
{
    Q_OBJECT

private:
    Ui::TagEditorDialog ui;
    NTagReaderInterface *m_tagReader;
    NCoverReaderInterface *m_coverReader;
    QString m_file;
    int m_encodingPreviousIndex;
    int m_encodingUtf8Index;
    int m_encodingSettingsIndex;
    bool m_hasChanges;
    void readTags();
    bool writeTags();

public:
    NTagEditorDialog(const QString &file, QWidget *parent = 0);
    ~NTagEditorDialog();

private slots:
    void on_encodingComboBox_activated(int index);
    void onSaveClicked();
    void setReadOnlyMode(bool readOnly);
};

#endif
