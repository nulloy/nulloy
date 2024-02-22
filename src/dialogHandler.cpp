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

#include "dialogHandler.h"

#include <QQmlApplicationEngine>

NDialogHandler::NDialogHandler(const QUrl &url, QObject *parentWindow)
    : QObject(parentWindow), m_url(url)
{
    m_qmlEngine = new QQmlApplicationEngine(this);
}

NDialogHandler::~NDialogHandler() {}

int NDialogHandler::parentCenterX()
{
    QObject *parentWindow = parent();
    if (!parentWindow) {
        return 0;
    }
    return parentWindow->property("x").toInt() + parentWindow->property("width").toInt() / 2;
}

int NDialogHandler::parentCenterY()
{
    QObject *parentWindow = parent();
    if (!parentWindow) {
        return 0;
    }
    return parentWindow->property("y").toInt() + parentWindow->property("height").toInt() / 2;
}

void NDialogHandler::showDialog()
{
    QQmlContext *context = m_qmlEngine->rootContext();
    context->setContextProperty("NDialogHandler", this);

    emit beforeShown(context);

    m_qmlEngine->load(m_url);

    QObject *root = m_qmlEngine->rootObjects().first();
    QObject::connect(root, SIGNAL(closed()), this, SLOT(on_closed()));
    root->setProperty("visible", true);
    emit afterShown(root);
}

void NDialogHandler::on_closed()
{
    if (!m_qmlEngine) {
        return;
    }
    m_qmlEngine->deleteLater();
    m_qmlEngine = nullptr;
}
