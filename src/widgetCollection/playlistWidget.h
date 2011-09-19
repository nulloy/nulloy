/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2011 Sergey Vlasov <sergey@vlasov.me>
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
#include <QListWidget>

class NPlaylistWidget : public QListWidget
{
	Q_OBJECT

private:
	NPlaylistItem *m_currentItem;

	void setCurrentItem(NPlaylistItem *item);
	void activateItem(NPlaylistItem *item);
	NPlaylistItem* createItemFromPath(const QString &path);
	bool dropMimeData(int index, const QMimeData *data, Qt::DropAction action);
	QStringList mimeTypes() const;
#ifdef Q_WS_MAC
	Qt::DropActions supportedDropActions() const;
#endif
	QMimeData* mimeData(const QList<NPlaylistItem *> items) const;
	void keyPressEvent(QKeyEvent *e);

public:
	NPlaylistWidget(QWidget *parent = 0);
	~NPlaylistWidget();

	NPlaylistItem* item(int row);

	QStringList mediaList();
	int currentRow();

public slots:
	void activateFirst();
	void activateNext();
	void activatePrev();
	void activateCurrent();
	void setCurrentFailed();
	void setCurrentRow(int row);
	void activateRow(int row);
	void appendMediaList(const QStringList &pathList);
	void setMediaList(const QStringList &pathList);
	void activateMediaList(const QStringList &pathList) { setMediaList(pathList); activateFirst(); }
	void setMediaListFromPlaylist(const QString &path);
	void writePlaylist(const QString &file);

private slots:
	void on_itemActivated(QListWidgetItem *item);
	void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);

signals:
	void currentActivated();
	void mediaSet(const QString &path);
	void closed();
};

#endif

/* vim: set ts=4 sw=4: */
