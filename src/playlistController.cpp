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

#include "playlistController.h"

#include "playlistModel.h"

NPlaylistController::NPlaylistController(QObject *parent) : QObject(parent)
{
    m_model = new NPlaylistModel(this);
}

NPlaylistModel *NPlaylistController::model() const
{
    return m_model;
}

void NPlaylistController::mousePress(int row, Qt::KeyboardModifiers modifiers)
{
    if (!modifiers) {
        if (!m_model->data(row, NPlaylistModel::IsSelectedRole).toBool()) {
            m_model->setDataAll(false, NPlaylistModel::IsSelectedRole);
            m_model->setData(row, true, NPlaylistModel::IsSelectedRole);
        }
    } else if (modifiers & Qt::ShiftModifier) {
        int oldFocusedIndex = m_model->focusedRow();
        if (oldFocusedIndex != -1) {
            if (oldFocusedIndex < row) {
                m_model->setDataRange(oldFocusedIndex, row, true, NPlaylistModel::IsSelectedRole);
            } else {
                m_model->setDataRange(row, oldFocusedIndex, true, NPlaylistModel::IsSelectedRole);
            }
        }
    } else if (modifiers & Qt::ControlModifier) {
        m_model->setData(row, !m_model->data(row, NPlaylistModel::IsSelectedRole).toBool(),
                         NPlaylistModel::IsSelectedRole);
    }

    m_model->setData(row, true, NPlaylistModel::IsFocusedRole);
}

void NPlaylistController::mouseRelease(int row, Qt::KeyboardModifiers modifiers)
{
    if (!modifiers) {
        m_model->setDataAll(false, NPlaylistModel::IsSelectedRole);
        m_model->setData(row, true, NPlaylistModel::IsSelectedRole);
    }
}

void NPlaylistController::mouseEnter(int row)
{
    m_model->setData(row, true, NPlaylistModel::IsHoveredRole);
}

void NPlaylistController::mouseExit(int row)
{
    m_model->setData(row, false, NPlaylistModel::IsHoveredRole);
}

void NPlaylistController::mouseDoubleClick(int row)
{
    emit rowActivated(row);
}
