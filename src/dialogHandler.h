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

#ifndef N_DIALOG_HANDLER_H
#define N_DIALOG_HANDLER_H

#include <QObject>
#include <QQmlContext>

class QQmlApplicationEngine;

class NDialogHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int parentCenterX READ parentCenterX CONSTANT)
    Q_PROPERTY(int parentCenterY READ parentCenterY CONSTANT)

public:
    NDialogHandler(const QUrl &url, QObject *parentWindow = nullptr);
    virtual ~NDialogHandler();

    int parentCenterX();
    int parentCenterY();

public slots:
    void showDialog();

private slots:
    void on_closed();

signals:
    void beforeShown(QQmlContext *context);
    void afterShown(QObject *root);

private:
    QQmlApplicationEngine *m_qmlEngine;
    QUrl m_url;
};

#endif
