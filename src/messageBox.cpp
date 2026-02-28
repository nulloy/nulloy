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

#include "messageBox.h"

#include <QQmlContext>

NMessageBox::NMessageBox(const QString &title, const QString &text,
                         QMessageBox::StandardButtons buttons, QObject *parentWindow)
    : NDialogHandler(QUrl::fromLocalFile("src/messageBox.qml"), parentWindow), m_title(title),
      m_text(text), m_buttons(buttons), m_checkBoxChecked(false)
{
    connect(this, &NDialogHandler::setupContext, [this](QQmlContext *context) {
        context->setContextProperty("NMessageBoxHandler", this);
    });

    connect(this, &NDialogHandler::setupRoot, [this](QObject *root) {
        QObject *buttonBox = root->findChild<QObject *>("buttonBox");
        connect(buttonBox, SIGNAL(accepted()), this, SIGNAL(accepted()));
        connect(buttonBox, SIGNAL(rejected()), this, SIGNAL(rejected()));
    });
}

NMessageBox::~NMessageBox()
{
    destroyEngine();
}

QString NMessageBox::title() const
{
    return m_title;
}

QString NMessageBox::text() const
{
    return m_text;
}

int NMessageBox::standardButtons() const
{
    return (int)m_buttons;
}

void NMessageBox::setCheckBoxText(const QString &text)
{
    m_checkBoxText = text;
}

QString NMessageBox::checkBoxText() const
{
    return m_checkBoxText;
}

bool NMessageBox::isCheckBoxChecked() const
{
    return m_checkBoxChecked;
}

void NMessageBox::setCheckBoxChecked(bool checked)
{
    m_checkBoxChecked = checked;
}
