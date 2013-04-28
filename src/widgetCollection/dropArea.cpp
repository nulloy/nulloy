/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2013 Sergey Vlasov <sergey@vlasov.me>
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

#include "dropArea.h"

#include "core.h"
#include <QUrl>
#include <QFile>

NDropArea::NDropArea(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f) {}

void NDropArea::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData() && event->mimeData()->hasUrls() && !event->mimeData()->urls().isEmpty())
		event->acceptProposedAction();
	else
		event->ignore();
}

void NDropArea::dragMoveEvent(QDragMoveEvent *event)
{
	event->acceptProposedAction();
}

void NDropArea::dragLeaveEvent(QDragLeaveEvent *event)
{
	event->accept();
}

void NDropArea::dropEvent(QDropEvent *event)
{
	const QMimeData *data = event->mimeData();
	if (data->hasUrls()) {
		QStringList files;
		foreach (QUrl url, data->urls())
			files << NCore::dirListRecursive(url.toLocalFile());
		emit filesDropped(files);
	}

	event->acceptProposedAction();
}

QStringList NDropArea::mimeTypes() const
{
	QStringList qstrList;
	qstrList.append("text/uri-list");
	return qstrList;
}

/* vim: set ts=4 sw=4: */
