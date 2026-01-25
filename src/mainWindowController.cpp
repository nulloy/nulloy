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

#include "mainWindowController.h"
#include "settings.h"

#include <QWindow>

NMainWindowController::NMainWindowController(QWindow &window, QObject *parent)
    : QObject(parent), m_window(window)
{
}

void NMainWindowController::toggleFullScreen()
{
    if (m_window.visibility() == QWindow::FullScreen) {
        m_window.showNormal();
    } else {
        m_window.showFullScreen();
    }
}

void NMainWindowController::loadSettings()
{
    NSettings *settings = NSettings::instance();

    QSize _size(430, 350);
    QStringList sizeList = settings->value("Size").toStringList();
    if (!sizeList.isEmpty() && sizeList.size() >= 2) {
        _size = QSize(sizeList.at(0).toInt(), sizeList.at(1).toInt());
    }
    m_window.resize(_size);

    QStringList posList = settings->value("Position").toStringList();
    if (!posList.isEmpty() && posList.size() >= 2) {
        QPoint _pos(posList.at(0).toInt(), posList.at(1).toInt());
        m_window.setPosition(_pos);
    }

    if (settings->value("Maximized").toBool()) {
        m_window.showMaximized();
    }
}

void NMainWindowController::saveSettings()
{
    NSettings *settings = NSettings::instance();
    settings->setValue("Maximized", m_window.visibility() == QWindow::Maximized);

    if (m_window.visibility() == QWindow::Windowed || m_window.visibility() == QWindow::Hidden) {
        QPoint _pos = m_window.position();
        QSize _size = m_window.size();

        settings->setValue("Position",
                           QStringList() << QString::number(_pos.x()) << QString::number(_pos.y()));
        settings->setValue("Size", QStringList() << QString::number(_size.width())
                                                 << QString::number(_size.height()));
    }
}

void NMainWindowController::show()
{
    m_window.show();
    m_window.requestActivate();
}

void NMainWindowController::setTitle(const QString &title)
{
    m_window.setTitle(title);
}

bool NMainWindowController::isOnTop()
{
    Qt::WindowFlags flags = m_window.flags();
    return (flags & Qt::WindowStaysOnTopHint);
}

void NMainWindowController::setOnTop(bool onTop)
{
    Qt::WindowFlags flags = m_window.flags();
    if (onTop) {
        flags |= Qt::WindowStaysOnTopHint;
    } else {
        flags &= ~Qt::WindowStaysOnTopHint;
    }

    bool visible = m_window.isVisible();
    m_window.setFlags(flags);
    if (visible) {
        m_window.show();
    }
}
