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

#ifndef N_MESSAGE_BOX_H
#define N_MESSAGE_BOX_H

#include "dialogHandler.h"

#include <QMessageBox>

class NMessageBox : public NDialogHandler
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString text READ text CONSTANT)
    Q_PROPERTY(int standardButtons READ standardButtons CONSTANT)
    Q_PROPERTY(QString checkBoxText READ checkBoxText CONSTANT)
    Q_PROPERTY(bool checkBoxChecked READ isCheckBoxChecked WRITE setCheckBoxChecked)

public:
    NMessageBox(const QString &title, const QString &text, QMessageBox::StandardButtons buttons,
                QObject *parentWindow = nullptr);
    ~NMessageBox() override;

    QString title() const;
    QString text() const;
    int standardButtons() const;

    void setCheckBoxText(const QString &text);
    QString checkBoxText() const;

    bool isCheckBoxChecked() const;
    void setCheckBoxChecked(bool checked);

signals:
    void accepted();
    void rejected();

private:
    QString m_title;
    QString m_text;
    QMessageBox::StandardButtons m_buttons;
    QString m_checkBoxText;
    bool m_checkBoxChecked;
};

#endif
