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

#include <QGridLayout>
#include <QResizeEvent>
#include <QSpacerItem>

#include "messageBox.h"

NMessageBox::NMessageBox(int width, QObject &centerInTarget, QWidget *parent)
    : QMessageBox(parent), m_centerInWindow{centerInTarget}
{
    m_width = width;
    m_resized = false;
}

void NMessageBox::resizeEvent(QResizeEvent *event)
{
    QMessageBox::resizeEvent(event);

    if (!m_resized) {
        QSpacerItem *spacer = new QSpacerItem(m_width, 0, QSizePolicy::Minimum,
                                              QSizePolicy::Expanding);
        QGridLayout *layout = (QGridLayout *)this->layout();
        layout->addItem(spacer, layout->rowCount(), 0, 1, layout->columnCount());

        move(m_centerInWindow.property("x").toInt() +
                 (m_centerInWindow.property("width").toInt() - m_width) / 2,
             m_centerInWindow.property("y").toInt() +
                 (m_centerInWindow.property("height").toInt() - event->size().height()) / 2);

        m_resized = true;
    }
}
