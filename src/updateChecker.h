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

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

class NUpdateChecker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString version READ version NOTIFY versionChanged)

public:
    static NUpdateChecker &instance();
    QString version() const;
    void setParentWindow(QObject *parentWindow);

public slots:
    void checkOnline();

private slots:
    void on_finished(QNetworkReply *reply);

signals:
    void versionChanged(const QString &version);

private:
    QString m_version;
    NUpdateChecker();
    QNetworkAccessManager *m_networkManager;
    QObject *m_parentWindow;
};
