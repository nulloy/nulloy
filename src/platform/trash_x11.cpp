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

#include <QDebug>
#include <QFileInfo>
#include <QProcess>

#include "settings.h"

int _trash(const QString &file, QString *error)
{
    bool customTrash = NSettings::instance()->value("CustomTrash").toBool();
    if (!customTrash) {
        *error = QString(QObject::tr("Custom Trash Command is not configured."));
        return -1;
    }

    QString cmd = NSettings::instance()->value("CustomTrashCommand").toString();
    if (cmd.isEmpty()) {
        *error = QString(QObject::tr("Custom Trash Command is enabled but not configured."));
        return -1;
    }

    QFileInfo fileInfo(file);
    QString filePath = file;
    QString fileName = fileInfo.fileName();
    QString canonicalPath = fileInfo.canonicalPath();

    // escape single quote
    filePath.replace("'", "'\\''");
    fileName.replace("'", "'\\''");
    canonicalPath.replace("'", "'\\''");

    cmd.replace("%p", filePath);
    cmd.replace("%F", fileName);
    cmd.replace("%P", canonicalPath);

    qDebug() << qPrintable(cmd);
    int res = QProcess::execute("sh", QStringList() << "-c" << cmd);
    if (res != 0) {
        *error =
            QString(QObject::tr("Custom Trash Command failed with exit code <b>%1</b>.")).arg(res);
        return -1;
    }

    return 0;
}
