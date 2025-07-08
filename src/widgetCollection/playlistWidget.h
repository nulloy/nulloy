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

#ifndef N_PLAYLIST_WIDGET_H
#define N_PLAYLIST_WIDGET_H

#include <QList>
#include <QListWidget>
#include <QPointer>

#include "global.h"

class NPlaylistDataItem;
class NPlaylistWidgetItem;
class NTrackInfoReader;
class NPlaybackEngineInterface;
class QContextMenuEvent;
class QDropEvent;
class QMenu;
class QString;
class QStringList;

class NPlaylistWidget : public QListWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor failed_text_color READ failedTextColor WRITE setFailedTextColor)
    Q_PROPERTY(QColor playing_text_color READ playingTextColor WRITE setPlayingTextColor)
    Q_PROPERTY(QColor file_drop_border_color READ fileDropBorderColor WRITE setFileDropBorderColor)
    Q_PROPERTY(QBrush file_drop_background READ fileDropBackground WRITE setFileDropBackground)
    Q_PROPERTY(int file_drop_radius READ fileDropRadius WRITE setFileDropRadius)

private:
    NPlaylistWidgetItem *m_playingItem;
    QMap<int, NPlaylistWidgetItem *> m_itemMap;
    NTrackInfoReader *m_trackInfoReader;
    NPlaybackEngineInterface *m_playbackEngine;
    QTimer *m_processVisibleItemsTimer;
    bool m_repeatMode;

    void addItem(NPlaylistWidgetItem *item);
    void paintEvent(QPaintEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void resizeEvent(QResizeEvent *event);
    bool refreshItemData(NPlaylistWidgetItem *item, QString titleFormat, bool force = false);
    NPlaylistWidgetItem *nextItem(NPlaylistWidgetItem *) const;
    NPlaylistWidgetItem *prevItem(NPlaylistWidgetItem *item) const;
    void resetPlayingItem();
    bool revealInFileManager(const QString &file, QString *error) const;

protected:
    void wheelEvent(QWheelEvent *event);

protected slots:
    void rowsInserted(const QModelIndex &parent, int start, int end);

private slots:
    void startProcessVisibleItemsTimer();

    void on_playbackEngine_mediaChanged(const QString &file, int id);
    void on_playbackEngine_prepareNextMediaRequested();
    void on_playbackEngine_mediaFinished(const QString &file, int id);
    void on_playbackEngine_mediaFailed(const QString &file, int id);

public:
    NPlaylistWidget(QWidget *parent = 0);
    ~NPlaylistWidget();

    QStringList selectedFiles() const;
    NPlaylistWidgetItem *itemAtRow(int row) const;
    NPlaylistWidgetItem *playingItem() const;
    int playingRow() const;
    Q_INVOKABLE bool hasPlayingItem() const;
    Q_INVOKABLE bool repeatMode() const;

    void setTrackInfoReader(NTrackInfoReader *reader);

public slots:
    void playRow(int row);
    void playItem(NPlaylistWidgetItem *item);
    void playNextItem();
    void playPrevItem();

    void setPlayingItem(NPlaylistWidgetItem *item); // does not start playback

    void addFiles(const QStringList &files);
    void addItems(const QList<NPlaylistDataItem> &dataItems);
    void setFiles(const QStringList &files);
    void setItems(const QList<NPlaylistDataItem> &dataItems);
    bool setPlaylist(const QString &file);
    void shufflePlaylist();
    void processVisibleItems();
    void updateTrackIndexes();
    void calculateDuration();
    void setRepeatMode(bool enable);
    void removeSelected();
    void removeFiles(const QStringList &files);

signals:
    void addMoreRequested();
    void contextMenuRequested(const QPoint &pos);

    void repeatModeChanged(bool enable);

    void itemsChanged();
    void playingItemChanged();
    void durationChanged(int seconds);
    void playlistFinished();

    // DRAG & DROP >>
public:
    enum DragStart
    {
        DragStartInside,
        DragStartOutside
    };
    Q_ENUM(DragStart)
    enum DropEnd
    {
        DropEndInside,
        DropEndOutside
    };
    Q_ENUM(DropEnd)
private:
    DragStart m_dragStart;
    DropEnd m_dropEnd;
    QPointer<QDrag> m_itemDrag;
    bool m_fileDrop;
    QList<QUrl> m_mimeDataUrls;
    QStringList mimeTypes() const;
    QMimeData *mimeData(const QList<QListWidgetItem *> items) const;
    bool dropMimeData(int index, const QMimeData *data, Qt::DropAction action);

protected:
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    // << DRAG & DROP

    // STYLESHEET PROPERTIES >>
private:
    QColor m_failedTextColor;
    QColor m_playingTextColor;
    QColor m_fileDropBorderColor;
    QBrush m_fileDropBackground;
    int m_fileDropRadius;

public:
    QColor failedTextColor() const;
    void setFailedTextColor(QColor color);

    QColor playingTextColor() const;
    void setPlayingTextColor(QColor color);

    QColor fileDropBorderColor() const;
    void setFileDropBorderColor(QColor color);

    QBrush fileDropBackground() const;
    void setFileDropBackground(QBrush brush);

    int fileDropRadius() const;
    void setFileDropRadius(int radius);
    // << STYLESHEET PROPERTIES
};

#endif
