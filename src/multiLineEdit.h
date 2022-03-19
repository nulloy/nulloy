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

#ifndef N_MULTI_LINE_EDIT_H
#define N_MULTI_LINE_EDIT_H

#include <QPlainTextEdit>

class NMultiLineEdit : public QPlainTextEdit
{
    Q_OBJECT

private:
    void keyPressEvent(QKeyEvent *e);
    void insertFromMimeData(const QMimeData *source);
    void appendPlainText(const QString &text) {}
    void insertPlainText(const QString &text) {}
    void setPlainText(const QString &text) {}

public:
    NMultiLineEdit(QWidget *parent = 0);
    QString text();

public slots:
    void setText(const QString &text);
};

#endif
