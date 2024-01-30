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

#include "multiLineEdit.h"

#include <QMimeData>

NMultiLineEdit::NMultiLineEdit(QWidget *parent) : QPlainTextEdit(parent) {}

QString NMultiLineEdit::text()
{
    return toPlainText();
}

void NMultiLineEdit::setText(const QString &text)
{
    QPlainTextEdit::setPlainText(QString(text).replace('\n', ' '));
}

void NMultiLineEdit::insertFromMimeData(const QMimeData *source)
{
    QMimeData *newData = new QMimeData;
    newData->setText(source->text().replace('\n', ' '));
    QPlainTextEdit::insertFromMimeData(newData);
}

void NMultiLineEdit::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        return;
    }

    QPlainTextEdit::keyPressEvent(e);
}
