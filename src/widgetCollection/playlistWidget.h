/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2022 Sergey Vlasov <sergey@vlasov.me>
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
    Q_PROPERTY(QColor active_text_color READ activeTextColor WRITE setActiveTextColor)
    Q_PROPERTY(QColor file_drop_border_color READ fileDropBorderColor WRITE setFileDropBorderColor)
    Q_PROPERTY(QBrush file_drop_background READ fileDropBackground WRITE setFileDropBackground)
    Q_PROPERTY(int file_drop_radius READ fileDropRadius WRITE setFileDropRadius)

private:
    NPlaylistWidgetItem *m_activeItem;
    QMenu *m_contextMenu;
    NTrackInfoReader *m_trackInfoReader;
    NPlaybackEngineInterface *m_playbackEngine;
    QTimer *m_processVisibleItemsTimer;

    QList<NPlaylistWidgetItem *> m_shuffledItems;
    int m_activeShuffledIndex;
    bool m_shuffleMode;
    bool m_repeatMode;

    void paintEvent(QPaintEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void resizeEvent(QResizeEvent *event);
    void setActiveItem(NPlaylistWidgetItem *item);
    void activateItem(NPlaylistWidgetItem *item);
    void formatItemTitle(NPlaylistWidgetItem *item, QString titleFormat, bool force = false);
    void resetActiveItem();
    bool revealInFileManager(const QString &file, QString *error) const;

protected:
    void wheelEvent(QWheelEvent *event);

protected slots:
    void rowsInserted(const QModelIndex &parent, int start, int end);

private slots:
    void on_itemActivated(QListWidgetItem *item);
    void on_trashAction_triggered();
    void on_removeAction_triggered();
    void on_revealAction_triggered();
    void on_tagEditorAction_triggered();
    void startProcessVisibleItemsTimer();

public:
    NPlaylistWidget(QWidget *parent = 0);
    ~NPlaylistWidget();

    NPlaylistWidgetItem *item(int row, bool loop = false) const;
    NPlaylistWidgetItem *nextItem() const;

    void setTrackInfoReader(NTrackInfoReader *reader);

    NPlaylistWidgetItem *activeItem() const;
    int activeRow() const;
    void setActiveRow(int row);
    Q_INVOKABLE bool hasActive() const;
    QModelIndex activeIndex() const;
    QString activeTitle() const;

    Q_INVOKABLE bool shuffleMode() const;
    Q_INVOKABLE bool repeatMode() const;

public slots:
    void activateNextItem();
    void activatePrevItem();
    void activateRow(int row);

    void addFiles(const QStringList &files);
    void addItems(const QList<NPlaylistDataItem> &dataItems);
    void setFiles(const QStringList &files);
    void setItems(const QList<NPlaylistDataItem> &dataItems);
    bool setPlaylist(const QString &file);
    void processVisibleItems();
    void updateTrackIndexes();

    void activeFinished();
    void activeFailed();

    void setShuffleMode(bool enable);
    void setRepeatMode(bool enable);

signals:
    void currentActivated();
    void mediaChanged(const QString &file);
    void tagEditorRequested(const QString &file);
    void addMoreRequested();

    void shuffleModeChanged(bool enable);
    void repeatModeChanged(bool enable);

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
    QColor m_activeTextColor;
    QColor m_fileDropBorderColor;
    QBrush m_fileDropBackground;
    int m_fileDropRadius;

public:
    QColor failedTextColor() const;
    void setFailedTextColor(QColor color);

    QColor activeTextColor() const;
    void setActiveTextColor(QColor color);

    QColor fileDropBorderColor() const;
    void setFileDropBorderColor(QColor color);

    QBrush fileDropBackground() const;
    void setFileDropBackground(QBrush brush);

    int fileDropRadius() const;
    void setFileDropRadius(int radius);
    // << STYLESHEET PROPERTIES
};

#endif
