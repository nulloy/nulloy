/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2012 Sergey Vlasov <sergey@vlasov.me>
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

#include "playlistItem.h"
#include "m3uPlaylist.h"
#include "tagReaderInterface.h"

#include <QListWidget>

class NPlaylistWidget : public QListWidget
{
	Q_OBJECT
	Q_PROPERTY(QColor failedTextColor READ getFailedTextColor WRITE setFailedTextColor DESIGNABLE true)
	Q_PROPERTY(QColor currentTextColor READ getCurrentTextColor WRITE setCurrentTextColor DESIGNABLE true)

private:
	NPlaylistItem *m_currentItem;
	QMenu *m_contextMenu;
	NTagReaderInterface *m_tagReader;

	void contextMenuEvent(QContextMenuEvent *event);
	void setCurrentRow(int row);
	void setCurrentItem(NPlaylistItem *item);
	void activateItem(NPlaylistItem *item);
	NPlaylistItem* createItemFromPath(const QString &file);
	NPlaylistItem* createItemFromM3uItem(NM3uItem item);
	bool dropMimeData(int index, const QMimeData *data, Qt::DropAction action);
	QStringList mimeTypes() const;
#ifdef Q_WS_MAC
	Qt::DropActions supportedDropActions() const;
#endif
	QMimeData* mimeData(const QList<NPlaylistItem *> items) const;

public:
	NPlaylistWidget(QWidget *parent = 0);
	~NPlaylistWidget();

	NPlaylistItem* item(int row);

	QStringList mediaList();
	int currentRow();
	QModelIndex currentIndex() const;
	QString currentTitle();
	void setTagReader(NTagReaderInterface *tagReader);

public slots:
	void activateFirst();
	void activateNext();
	void activatePrev();
	void activateCurrent();
	void setCurrentFailed();
	void activateRow(int row);
	void appendMediaList(const QStringList &pathList);
	void setMediaList(const QStringList &pathList);
	void activateMediaList(const QStringList &pathList) { setMediaList(pathList); activateFirst(); }
	void setMediaListFromPlaylist(const QString &file);
	void writePlaylist(const QString &file);

private slots:
	void on_itemActivated(QListWidgetItem *item);
	void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
	void moveToTrash();
	void removeFromPlaylist();

signals:
	void currentActivated();
	void mediaSet(const QString &file);
	void closed();

// STYLESHEET PROPERTIES
private:
	QColor m_failedTextColor;
	QColor m_currentTextColor;

public:
	QColor getFailedTextColor();
	void setFailedTextColor(QColor color);

	QColor getCurrentTextColor();
	void setCurrentTextColor(QColor color);
};

#endif

/* vim: set ts=4 sw=4: */
