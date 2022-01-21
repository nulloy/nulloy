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

#ifndef N_LOG_DIALOG_H
#define N_LOG_DIALOG_H

#include <QDialog>

#include "global.h"

class QCheckBox;
class QString;
class QTextBrowser;
class QTextBrowser;

class NLogDialog : public QDialog
{
    Q_OBJECT

private:
    QTextBrowser *m_textBrowser;
    QCheckBox *m_checkBox;
    QString m_oldTitle;
    QString m_text;

    void closeEvent(QCloseEvent *event);

public:
    NLogDialog(QWidget *parent = 0);
    ~NLogDialog();

public slots:
    void showMessage(N::MessageIcon icon, const QString &title, const QString &msg);
};

#endif
