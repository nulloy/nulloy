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

#include "logDialog.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QSpacerItem>
#include <QTextBrowser>
#include <QVBoxLayout>

#include "settings.h"

NLogDialog::NLogDialog(QWidget *parent) : QDialog(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);

    m_textBrowser = new QTextBrowser;
    m_textBrowser->setStyleSheet("QTextBrowser { background: transparent; }");
    m_textBrowser->setFrameShape(QFrame::NoFrame);
    layout->addWidget(m_textBrowser);

    QHBoxLayout *hLayout = new QHBoxLayout;
    layout->addLayout(hLayout);

    m_checkBox = new QCheckBox("Don't show this dialog anymore");
    hLayout->addWidget(m_checkBox);

    hLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));

    QPushButton *closeButton = new QPushButton("Close");
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
    hLayout->addWidget(closeButton);

    setWindowTitle(QCoreApplication::applicationName() + " Log");

    setMinimumWidth(500);
}

NLogDialog::~NLogDialog() {}

void NLogDialog::showMessage(N::MessageIcon icon, const QString &title, const QString &msg)
{
    if (!m_text.isEmpty()) {
        m_text.append("<br>");
    }

    if (m_oldTitle != title) {
        if (!m_text.isEmpty()) {
            m_text.append("<br>");
        }
        m_text.append("<b>" + title + "</b><br>");
        m_oldTitle = title;
    }

    switch (icon) {
        case N::Critical:
            m_text.append("<span style=\"background-color: #ff0000\">Error</span>: ");
            break;
        case N::Warning:
            m_text.append("<span style=\"background-color: #ffaa00\">Warning</span>: ");
            break;
        default:
            break;
    }

    m_text.append(msg);
    m_textBrowser->setHtml(m_text);

    QTextCursor cur = m_textBrowser->textCursor();
    cur.movePosition(QTextCursor::End);
    m_textBrowser->setTextCursor(cur);

    m_checkBox->setChecked(!NSettings::instance()->value("DisplayLogDialog").toBool());

    if (!NSettings::instance()->value("DisplayLogDialog").toBool()) {
        return;
    }

    showNormal();
    activateWindow();
}

void NLogDialog::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);

    NSettings::instance()->setValue("DisplayLogDialog", !m_checkBox->isChecked());

    m_textBrowser->clear();
    m_text.clear();
    m_oldTitle.clear();
}
