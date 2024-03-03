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

#ifndef N_PLAYLIST_CONTROLLER_H
#define N_PLAYLIST_CONTROLLER_H

#include <QObject>
#include <QUrl>

#include "playbackEngineInterface.h"
#include "playlistModel.h"
#include "settings.h"
#include "trackInfoReader.h"

class NPlaylistModel;
class QTimer;

class NPlaylistController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int rowsPerPage WRITE setRowsPerPage)
    Q_PROPERTY(int firstVisibleRow WRITE setFirstVisibleRow)

public:
    NPlaylistController(NPlaybackEngineInterface &playbackEngine, NTrackInfoReader &reader,
                        NSettings &settings, QObject *parent = nullptr);

    Q_INVOKABLE NPlaylistModel *model() const;

    Q_INVOKABLE void mousePress(int row, Qt::KeyboardModifiers modifiers);
    Q_INVOKABLE void mouseRelease(int row, Qt::KeyboardModifiers modifiers);
    Q_INVOKABLE void mouseEnter(int row);
    Q_INVOKABLE void mouseExit(int row);
    Q_INVOKABLE void mouseDoubleClick(int row);

    Q_INVOKABLE void keyPress(int key, Qt::KeyboardModifiers modifiers);
    Q_INVOKABLE void keyRelease(int key, Qt::KeyboardModifiers modifiers);

    Q_INVOKABLE QStringList selectedFiles() const;
    Q_INVOKABLE void moveSelected(int beforeRow);
    Q_INVOKABLE void removeSelected();
    Q_INVOKABLE void removeFiles(const QStringList &files);
    Q_INVOKABLE void dropUrls(int beforeRow, const QList<QUrl> &urls);

    void appendFiles(const QStringList &files);
    void setFiles(const QStringList &files);
    void loadSettings();

signals:
    void contextMenuRequested(const QPoint &pos);
    void playlistFinished();
    void addMoreRequested();

public slots:
    void playRow(int row);
    void playNextRow();
    void playPrevRow();
    void shuffleRows();

private slots:
    void startProcessVisibleRowsTimer();
    void setRowsPerPage(int rows);
    void setFirstVisibleRow(int row);
    void processVisibleRows();

    void on_playbackEngine_prepareNextMediaRequested();
    void on_playbackEngine_mediaChanged(const QString &file, int id);
    void on_playbackEngine_mediaFinished(const QString &file, int id);
    void on_playbackEngine_mediaFailed(const QString &file, int id);

private:
    NPlaybackEngineInterface &m_playbackEngine;
    NTrackInfoReader &m_trackInfoReader;
    NSettings &m_settings;

    NPlaylistModel *m_model;
    int m_rowsPerPage;
    int m_firstVisibleRow;
    QString m_textFormat;
    QTimer *m_processVisibleRowsTimer;

    void processRow(size_t row, bool force = false);
    ssize_t nextRow(size_t row) const;
    ssize_t prevRow(size_t row) const;
};
#endif
