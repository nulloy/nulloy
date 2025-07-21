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

#include "updateChecker.h"

#include "settings.h"

#include <QCoreApplication>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

NUpdateChecker &NUpdateChecker::instance()
{
    static NUpdateChecker _instance;
    return _instance;
}

NUpdateChecker::NUpdateChecker()
{
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &NUpdateChecker::on_finished);
}

QString NUpdateChecker::version() const
{
    return m_version;
}

void NUpdateChecker::setParentWindow(QObject *parentWindow)
{
    m_parentWindow = parentWindow;
}

void NUpdateChecker::checkOnline()
{
    m_version = QString();
    NSettings::instance()->setValue("UpdateIgnore", "");
    m_networkManager->get(QNetworkRequest(
        QUrl("https://static." + QCoreApplication::organizationDomain() + "/release/version")));
}

void NUpdateChecker::on_finished(QNetworkReply *reply)
{
    if (!reply->error()) {
        m_version = reply->readAll().simplified();

        emit versionChanged(m_version);

        if (NSettings::instance()->value("UpdateIgnore", "").toString() == m_version) {
            return;
        }

        if (QCoreApplication::applicationVersion() < m_version) {
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Update"));
            msgBox.setText(QCoreApplication::applicationName() + " " + m_version + " " +
                           tr("released!") + "<br><br>" + "<a href='https://" +
                           QCoreApplication::organizationDomain() + "/download'>https://" +
                           QCoreApplication::organizationDomain() + "/download</a>");
            msgBox.setStandardButtons(QMessageBox::Ignore);
            msgBox.setWindowModality(Qt::ApplicationModal);
            msgBox.setIcon(QMessageBox::Information);

            const int width = 300;
            const int height = 150;
            msgBox.resize(width, height);

            if (m_parentWindow) {
                msgBox.move(m_parentWindow->property("x").toInt() +
                                (m_parentWindow->property("width").toInt() - width) / 2,
                            m_parentWindow->property("y").toInt() +
                                (m_parentWindow->property("height").toInt() - height) / 2);
            }

            msgBox.exec();
        }

        NSettings::instance()->setValue("UpdateIgnore", m_version);
    }

    reply->deleteLater();
}
