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

#ifndef N_PREFERENCES_DIALOG_H
#define N_PREFERENCES_DIALOG_H

#include <QDialog>

#include "global.h"
#include "pluginLoader.h"
#include "ui_preferencesDialog.h"

class QGroupBox;
class QRadioButton;

class NPreferencesDialog : public QDialog
{
    Q_OBJECT

private:
    Ui::PreferencesDialog ui;
    void showEvent(QShowEvent *event);
    QGroupBox *createGroupBox(N::PluginType type);
    QString selectedContainer(N::PluginType type);
    QMap<QRadioButton *, NPluginLoader::Descriptor> m_radioButtons;

public:
    NPreferencesDialog(QWidget *parent = 0);
    ~NPreferencesDialog();

private slots:
    void loadSettings();
    void saveSettings();
    void on_fileManagerHelpButton_clicked();
    void on_customTrashHelpButton_clicked();
    void on_titleFormatHelpButton_clicked();
    void on_languageComboBox_activated(int index);

signals:
    void settingsChanged();

#ifndef _N_NO_UPDATE_CHECK_
public slots:
    void setVersionLabel(QString text);

private slots:
    void on_versionCheckButton_clicked();

signals:
    void versionRequested();
#endif
};

#endif
