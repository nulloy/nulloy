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

#include "action.h"
#include "player.h"
#include "pluginLoader.h"
#include "tagEditorDialog.h"
#include "trackInfoReader.h"
#include "utils.h"

#include <QFileInfo>
#include <QKeyEvent>
#include <QTimer>

NPlaylistController::NPlaylistController(NPlaybackEngineInterface &playbackEngine,
                                         NTrackInfoReader &reader, NSettings &settings,
                                         QObject *parent)
    : QObject(parent), m_playbackEngine(playbackEngine), m_trackInfoReader(reader),
      m_settings(settings)
{
    m_model = new NPlaylistModel(this);
    m_rowsPerPage = 1;
    m_firstVisibleRow = 0;

    m_processVisibleRowsTimer = new QTimer(this);
    m_processVisibleRowsTimer->setSingleShot(true);
    connect(m_processVisibleRowsTimer, &QTimer::timeout, this,
            &NPlaylistController::processVisibleRows);

    connect(&m_playbackEngine, SIGNAL(nextMediaRequested()), this,
            SLOT(on_playbackEngine_prepareNextMediaRequested()), Qt::BlockingQueuedConnection);
    connect(&m_playbackEngine, SIGNAL(mediaFailed(const QString &, int)), this,
            SLOT(on_playbackEngine_mediaFailed(const QString &, int)));
    connect(&m_playbackEngine, SIGNAL(mediaChanged(const QString &, int)), this,
            SLOT(on_playbackEngine_mediaChanged(const QString &, int)));
    connect(&m_playbackEngine, SIGNAL(mediaFinished(const QString &, int)), this,
            SLOT(on_playbackEngine_mediaFinished(const QString &, int)));
}

void NPlaylistController::loadSettings()
{
    QString newFormat = m_settings.value("PlaylistTrackInfo").toString();
    if (m_textFormat != newFormat) {
        m_textFormat = newFormat;
        startProcessVisibleRowsTimer();
    }
}

void NPlaylistController::setRowsPerPage(int rows)
{
    m_rowsPerPage = rows;
    startProcessVisibleRowsTimer();
}

void NPlaylistController::setFirstVisibleRow(int row)
{
    m_firstVisibleRow = row;
    startProcessVisibleRowsTimer();
}

NPlaylistModel *NPlaylistController::model() const
{
    return m_model;
}

QStringList NPlaylistController::selectedFiles() const
{
    QStringList files;
    for (int row : m_model->selectedRows()) {
        files << m_model->data(row, NPlaylistModel::FilePathRole).toString();
    }
    return files;
}

void NPlaylistController::mousePress(int row, Qt::KeyboardModifiers modifiers)
{
    if (row == -1) {
        return;
    }

    if (!modifiers) {
        if (!m_model->data(row, NPlaylistModel::IsSelectedRole).toBool()) {
            m_model->setData(NPlaylistModel::IsSelectedRole, false);
            m_model->setData(row, NPlaylistModel::IsSelectedRole, true);
        }
    } else if (modifiers & Qt::ShiftModifier) {
        int oldFocusedRow = m_model->focusedRow();
        if (oldFocusedRow < row) {
            m_model->setData(oldFocusedRow, row, NPlaylistModel::IsSelectedRole, true);
        } else {
            m_model->setData(row, oldFocusedRow, NPlaylistModel::IsSelectedRole, true);
        }
    } else if (modifiers & Qt::ControlModifier) {
        m_model->setData(row, NPlaylistModel::IsSelectedRole,
                         !m_model->data(row, NPlaylistModel::IsSelectedRole).toBool());
    }

    m_model->setFocusedRow(row);
}

void NPlaylistController::mouseRelease(int row, Qt::KeyboardModifiers modifiers)
{
    if (modifiers) {
        return;
    }

    m_model->setData(NPlaylistModel::IsSelectedRole, false);

    if (row == -1) {
        return;
    }

    m_model->setData(row, NPlaylistModel::IsSelectedRole, true);
}

void NPlaylistController::mouseEnter(int row)
{
    m_model->setData(row, NPlaylistModel::IsHoveredRole, true);
}

void NPlaylistController::mouseExit(int row)
{
    m_model->setData(row, NPlaylistModel::IsHoveredRole, false);
}

void NPlaylistController::mouseDoubleClick(int row)
{
    playRow(row);
}

void NPlaylistController::moveSelected(int beforeRow)
{
    m_model->move(m_model->selectedRows(), beforeRow);
}

void NPlaylistController::removeSelected()
{
    const QList<size_t> selectedRows = m_model->selectedRows();
    size_t rowToFocus = selectedRows.first();

    m_model->remove(selectedRows);

    if (m_model->size() == 0) {
        return;
    }

    rowToFocus = qMin(rowToFocus, static_cast<size_t>(m_model->size() - 1));
    m_model->setData(rowToFocus, NPlaylistModel::IsFocusedRole, true);
    m_model->setData(rowToFocus, NPlaylistModel::IsSelectedRole, true);

    // TODO: update playing row

    processVisibleRows();
}

void NPlaylistController::keyPress(int key, Qt::KeyboardModifiers modifiers)
{
    const QKeyEvent keyEvent(QEvent::KeyPress, key, modifiers);

    if (keyEvent.matches(QKeySequence::SelectAll)) {
        m_model->setData(NPlaylistModel::IsSelectedRole, true);
        return;
    }

    int focusedRow = m_model->focusedRow();

    switch (key) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            playRow(focusedRow);
            return;
        case Qt::Key_Up:
            m_model->setFocusedRowRelative(-1);
            break;
        case Qt::Key_Down:
            m_model->setFocusedRowRelative(+1);
            break;
        case Qt::Key_Home:
            m_model->setFocusedRow(0);
            break;
        case Qt::Key_End:
            m_model->setFocusedRow(m_model->size() - 1);
            break;
        case Qt::Key_PageUp:
            m_model->setFocusedRowRelative(-m_rowsPerPage);
            break;
        case Qt::Key_PageDown:
            m_model->setFocusedRowRelative(+m_rowsPerPage);
            break;
    }

    if (!modifiers) {
        m_model->setData(NPlaylistModel::IsSelectedRole, false);
        m_model->setData(m_model->focusedRow(), NPlaylistModel::IsSelectedRole, true);
        return;
    } else if (modifiers & Qt::ShiftModifier) {
        int newFocusedRow = m_model->focusedRow();
        if (focusedRow < newFocusedRow) {
            m_model->setData(focusedRow, newFocusedRow, NPlaylistModel::IsSelectedRole, true);
        } else {
            m_model->setData(newFocusedRow, focusedRow, NPlaylistModel::IsSelectedRole, true);
        }
        return;
    } else if (modifiers & Qt::ControlModifier) {
        switch (key) {
            case Qt::Key_Space:
                m_model->setData(
                    m_model->focusedRow(), NPlaylistModel::IsSelectedRole,
                    !m_model->data(m_model->focusedRow(), NPlaylistModel::IsSelectedRole).toBool());
                return;
        }
    }
}

void NPlaylistController::keyRelease(int key, Qt::KeyboardModifiers modifiers) {}

void NPlaylistController::dropUrls(int beforeRow, const QList<QUrl> &urls)
{
    QList<NPlaylistModel::DataItem> items;
    for (const QUrl &url : urls) {
        for (const NPlaylistModel::DataItem &item :
             NUtils::processPathsRecursive({url.toLocalFile()})) {
            items << item;
        }
    }

    beforeRow = qBound(0, beforeRow, static_cast<int>(m_model->size()));
    m_model->insert(beforeRow, items);

    processVisibleRows();
}

void NPlaylistController::shuffleRows()
{
    m_model->shuffle();
    processVisibleRows();
}

void NPlaylistController::appendFiles(const QStringList &files)
{
    QList<NPlaylistModel::DataItem> items;
    for (const NPlaylistModel::DataItem &item : NUtils::processPathsRecursive(files)) {
        items << item;
    }
    m_model->append(items);

    processVisibleRows();
}

void NPlaylistController::setFiles(const QStringList &files)
{
    m_model->clear();
    appendFiles(files);
}

void NPlaylistController::removeFiles(const QStringList &files)
{
    if (files.isEmpty()) {
        return;
    }

    QList<size_t> rowsToRemove;
    const size_t size = m_model->size();
    for (size_t i = 0; i < size; ++i) {
        if (files.contains(m_model->data(i, NPlaylistModel::FilePathRole).toString())) {
            rowsToRemove << i;
        }
    }

    m_model->remove(rowsToRemove);

    // TODO: update focus, playing and selected rows
    processVisibleRows();
}

void NPlaylistController::startProcessVisibleRowsTimer()
{
    if (m_processVisibleRowsTimer->isActive()) {
        m_processVisibleRowsTimer->stop();
    }

    m_processVisibleRowsTimer->start(30);
}

void NPlaylistController::processVisibleRows()
{
    for (int i = m_firstVisibleRow; i <= m_firstVisibleRow + m_rowsPerPage; ++i) {
        processRow(i);
    }

    m_model->calculateDuration();
}

void NPlaylistController::processRow(size_t row, bool force)
{
    if (!force && m_model->data(row, NPlaylistModel::TextFormatRole).toString() == m_textFormat &&
        m_model->data(row, NPlaylistModel::DurationRole).toInt() != -1) {
        return;
    }

    const QString filePath = m_model->data(row, NPlaylistModel::FilePathRole).toString();
    m_trackInfoReader.setSource(filePath);

    m_model->setData(row, NPlaylistModel::DurationRole, m_trackInfoReader.toString("%D").toInt());
    m_model->setData(row, NPlaylistModel::TextRole, m_trackInfoReader.toString(m_textFormat));
    m_model->setData(row, NPlaylistModel::TextFormatRole, m_textFormat);
}

void NPlaylistController::playRow(int row)
{
    if (row == -1) {
        return;
    }

    m_playbackEngine.setMedia(m_model->data(row, NPlaylistModel::FilePathRole).toString(),
                              m_model->data(row, NPlaylistModel::IdRole).toInt());
    m_playbackEngine.play();
    m_model->setData(row, NPlaylistModel::IsPlayingtRole, true);
}

ssize_t NPlaylistController::nextRow(size_t row) const
{
    if (m_model->size() == 0) {
        return -1;
    }

    ssize_t nextRow = row + 1;
    if (nextRow >= m_model->size()) {
        if (m_settings.value("LoopPlaylist").toBool()) {
            nextRow = 0;
        } else {
            nextRow = -1;
        }
    }

    return nextRow;
}

ssize_t NPlaylistController::prevRow(size_t row) const
{
    if (m_model->size() == 0) {
        return -1;
    }

    ssize_t prevRow = row - 1;
    if (prevRow < 0 && m_settings.value("LoopPlaylist").toBool()) {
        prevRow = m_model->size() - 1;
    }

    return prevRow;
}

void NPlaylistController::playNextRow()
{
    ssize_t newPlayingRow = nextRow(m_model->playingRow());
    if (newPlayingRow < 0) {
        emit addMoreRequested();
        newPlayingRow = nextRow(m_model->playingRow());
    }

    if (newPlayingRow < 0) {
        m_playbackEngine.stop();
        return;
    }

    playRow(newPlayingRow);
}

void NPlaylistController::playPrevRow()
{
    const ssize_t newPlayingRow = prevRow(m_model->playingRow());

    if (newPlayingRow < 0) {
        m_playbackEngine.stop();
        return;
    }

    playRow(newPlayingRow);
}

void NPlaylistController::on_playbackEngine_prepareNextMediaRequested()
{
    const ssize_t playingRow = m_model->playingRow();
    ssize_t nextPlayingRow;
    if (m_settings.value("Repeat").toBool()) {
        nextPlayingRow = playingRow;
    } else {
        nextPlayingRow = nextRow(playingRow);
    }

    if (nextPlayingRow < 0) {
        return;
    }

    m_playbackEngine
        .nextMediaRespond(m_model->data(nextPlayingRow, NPlaylistModel::FilePathRole).toString(),
                          m_model->data(nextPlayingRow, NPlaylistModel::IdRole).toInt());
}

void NPlaylistController::on_playbackEngine_mediaChanged(const QString &file, int id)
{
    const ssize_t oldPlayingRow = m_model->playingRow();
    const ssize_t newPlayingRow = m_model->getRowById(id);

    if (oldPlayingRow >= 0) {
        m_model->setData(oldPlayingRow, NPlaylistModel::PlaybackPositionRole,
                         m_playbackEngine.position());
        m_model->setData(oldPlayingRow, NPlaylistModel::PlaybackCountRole,
                         m_model->data(oldPlayingRow, NPlaylistModel::PlaybackCountRole).toInt() +
                             1);
    }

    if (oldPlayingRow == newPlayingRow) {
        return;
    }

    if (newPlayingRow < 0) {
        m_model->setData(oldPlayingRow, NPlaylistModel::IsPlayingtRole, false);
        return;
    }

    m_model->setData(newPlayingRow, NPlaylistModel::IsPlayingtRole, true);
}

void NPlaylistController::on_playbackEngine_mediaFinished(const QString &, int id)
{
    const ssize_t playingRow = m_model->getRowById(id);
    if (playingRow < 0) {
        return;
    }

    ssize_t newPlayingRow;
    if (m_settings.value("Repeat").toBool()) {
        newPlayingRow = playingRow;
    } else {
        newPlayingRow = nextRow(playingRow);
    }

    if (newPlayingRow < 0) {
        emit playlistFinished();
        return;
    }

    playRow(newPlayingRow);
}

void NPlaylistController::on_playbackEngine_mediaFailed(const QString &file, int id)
{
    const ssize_t oldPlayingRow = m_model->getRowById(id);
    if (oldPlayingRow < 0) {
        return;
    }

    m_model->setData(oldPlayingRow, NPlaylistModel::IsPlayingtRole, true);
    m_model->setData(oldPlayingRow, NPlaylistModel::IsFailedRole, true);
}
