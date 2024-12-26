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

#include "trash.h"

#include "settings.h"

#include <QCheckBox>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>

int _trash(const QString &file, QString *error);

QStringList NTrash::moveToTrash(QStringList files)
{
    foreach (QString file, files) {
        if (NSettings::instance()->value("DisplayMoveToTrashConfirmDialog").toBool()) {
            QCheckBox *checkBox = new QCheckBox(QObject::tr("Don't show this dialog anymore"));

            QMessageBox msgBox(QMessageBox::Question, QObject::tr("Confirmation"),
                               QObject::tr("Do you want to move <b>%1</b> to Trash?<br><br>Please "
                                           "backup the file before trying this for the "
                                           "first time.")
                                   .arg(QFileInfo(file).fileName()),
                               QMessageBox::Yes | QMessageBox::Cancel, NULL);
            msgBox.setDefaultButton(QMessageBox::Cancel);
            msgBox.setCheckBox(checkBox);
            int res = msgBox.exec();

            if (res != QMessageBox::Yes) {
                return files;
            }

            NSettings::instance()->setValue("DisplayMoveToTrashConfirmDialog",
                                            !checkBox->isChecked());
        }

        QString error;
        if (_trash(file, &error) != 0) {
            QMessageBox box(QMessageBox::Warning, QObject::tr("Trash Error"), "",
                            QMessageBox::Yes | QMessageBox::Cancel, NULL);
            box.setDefaultButton(QMessageBox::Cancel);
            box.setText(
                QObject::tr("Couldn't to move to Trash <b>%1</b>.").arg(QFileInfo(file).fileName()) +
                (error.isEmpty() ? "" : " <br>" + error));
            box.setInformativeText(QObject::tr("Do you want to delete permanently?"));
            if (box.exec() == QMessageBox::Yes) {
                if (!QFile::remove(file)) {
                    QMessageBox::critical(NULL, QObject::tr("File Delete Error"),
                                          QObject::tr("Failed to delete <b>%1</b>.").arg(file));
                    break;
                }
            } else {
                break;
            }
        }

        files.removeAt(files.indexOf(file));
    }

    return files;
}
