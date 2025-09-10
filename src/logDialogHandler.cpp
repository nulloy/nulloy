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

#include "logDialogHandler.h"
#include "settings.h"

NLogDialogHandler::NLogDialogHandler(QObject *parentWindow)
    : NDialogHandler(QUrl::fromLocalFile("src/logDialog.qml"), parentWindow)
{
    connect(this, &NDialogHandler::setupContext, [this](QQmlContext *context) {
        context->setContextProperty("NLogDialogHandler", this);
        context->setContextProperty("NSettings", NSettings::instance());
    });
    connect(this, &NDialogHandler::closed, [this]() {
        m_text.clear();
        m_oldTitle.clear();
    });
}

QString NLogDialogHandler::text() const
{
    return m_text;
}

void NLogDialogHandler::showMessage(N::MessageIcon icon, const QString &title, const QString &msg)
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
    emit textChanged();

    if (!NSettings::instance()->value("DisplayLogDialog").toBool()) {
        return;
    }

    showDialog();
}
