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

signals:
    void setupContext(QQmlContext *context);
    void setupRoot(QObject *root);
    void closed();

protected:
    void destroyEngine();

private:
    QQmlApplicationEngine *m_qmlEngine;
    bool m_loaded;
    QUrl m_url;
};

#endif
