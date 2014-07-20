/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2014 Sergey Vlasov <sergey@vlasov.me>
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

#include "global.h"
#include <QListWidget>
#include <QList>
#include <QPointer>

class NPlaylistWidgetItem;
class NTagReaderInterface;
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
	Q_PROPERTY(QColor current_text_color READ currentTextColor WRITE setCurrentTextColor)

private:
	NPlaylistWidgetItem *m_currentItem;
	QMenu *m_contextMenu;
	NTagReaderInterface *m_tagReader;
	NPlaybackEngineInterface *m_playbackEngine;

	QList<NPlaylistWidgetItem *> m_shuffledItems;
	int m_currentShuffledIndex;
	bool m_shuffleMode;
	bool m_repeatMode;

	void contextMenuEvent(QContextMenuEvent *event);
	void setCurrentItem(NPlaylistWidgetItem *item);
	void activateItem(NPlaylistWidgetItem *item);

protected slots:
	void rowsInserted(const QModelIndex &parent, int start, int end);
	void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);

private slots:
	void on_itemActivated(QListWidgetItem *item);
	void on_trashAction_triggered();
	void on_removeAction_triggered();
	void on_revealAction_triggered();

public:
	NPlaylistWidget(QWidget *parent = 0);
	~NPlaylistWidget();

	NPlaylistWidgetItem* item(int row);

	int currentRow();
	void setCurrentRow(int row);
	Q_INVOKABLE bool hasCurrent();
	QModelIndex currentIndex() const;
	QString currentTitle();

	Q_INVOKABLE bool shuffleMode();
	Q_INVOKABLE bool repeatMode();

public slots:
	void playNextRow();
	void playPreviousRow();
	void playRow(int row);

	void addFiles(const QStringList &files);
	void setFiles(const QStringList &files);
	void playFiles(const QStringList &files);
	bool setPlaylist(const QString &file);

	void currentFinished();
	void currentFailed();

	void setShuffleMode(bool enable);
	void setRepeatMode(bool enable);

signals:
	void currentActivated();
	void mediaSet(const QString &file);
	void activateEmptyFail();

	void shuffleModeChanged(bool enable);
	void repeatModeChanged(bool enable);

// DRAG & DROP >>
private:
	QPointer<QDrag> m_drag;
	QList<QUrl> m_mimeDataUrls;
	QStringList mimeTypes() const;
	QMimeData* mimeData(const QList<NPlaylistWidgetItem *> items) const;
	bool dropMimeData(int index, const QMimeData *data, Qt::DropAction action);
#ifdef Q_WS_MAC
	Qt::DropActions supportedDropActions() const;
#endif
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
	QColor m_currentTextColor;

public:
	QColor failedTextColor() const;
	void setFailedTextColor(QColor color);

	QColor currentTextColor() const;
	void setCurrentTextColor(QColor color);
// << STYLESHEET PROPERTIES
};

#endif

