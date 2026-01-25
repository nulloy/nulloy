/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2026 Sergey Vlasov <sergey@vlasov.me>
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

#ifndef N_MAIN_WINDOW_CONTROLLER_H
#define N_MAIN_WINDOW_CONTROLLER_H

#include <QCursor>
#include <QObject>

class QWindow;

class NMainWindowController : public QObject
{
    Q_OBJECT
public:
    explicit NMainWindowController(QWindow &window, QObject *parent = nullptr);

    bool isOnTop();

public slots:
    void show();
    void toggleFullScreen();
    void setTitle(const QString &title);
    void setOnTop(bool onTop);
    void loadSettings();
    void saveSettings();

private:
    QWindow &m_window;
};

#endif
