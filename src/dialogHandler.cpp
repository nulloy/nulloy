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
    m_loaded = false;
}

NDialogHandler::~NDialogHandler() {}

void NDialogHandler::destroyEngine()
{
    delete m_qmlEngine;
    m_qmlEngine = nullptr;
}

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
    if (!m_loaded) {
        QQmlContext *context = m_qmlEngine->rootContext();
        context->setContextProperty("NDialogHandler", this);
        emit setupContext(context);

        m_qmlEngine->load(m_url);
        m_loaded = true;

        QObject *root = m_qmlEngine->rootObjects().first();
        QObject::connect(root, SIGNAL(closed()), this, SIGNAL(closed()));
        emit setupRoot(root);
    }

    QObject *root = m_qmlEngine->rootObjects().first();
    root->setProperty("visible", true);
}
