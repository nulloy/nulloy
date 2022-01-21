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

#include "trash.h"

#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>

int _trash(const QString &file, QString *error);

QStringList NTrash::moveToTrash(QStringList files)
{
    foreach (QString file, files) {
        QString error;
        if (_trash(file, &error) != 0) {
            QMessageBox box(QMessageBox::Warning, "Trash Error", "",
                            QMessageBox::Yes | QMessageBox::Cancel, NULL);
            box.setDefaultButton(QMessageBox::Cancel);
            box.setText("Couldn't to move to Trash <b>" + QFileInfo(file).fileName() + "</b>." +
                        (error.isEmpty() ? "" : " <br>" + error));
            box.setInformativeText("Do you want to delete permanently?");
            if (box.exec() == QMessageBox::Yes) {
                if (!QFile::remove(file)) {
                    QMessageBox::critical(NULL, "File Delete Error",
                                          "Failed to delete <b>" + file + "</b>.");
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
