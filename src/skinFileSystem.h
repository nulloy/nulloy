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

#ifndef N_SKIN_FILE_SYSTEM_H
#define N_SKIN_FILE_SYSTEM_H

#include <QAbstractFileEngineHandler>
#include <QAbstractFileEngine>
#include <QByteArray>
#include <QHash>
#include <QString>

class NSkinFileSystem : public QAbstractFileEngineHandler
{
public:
	QAbstractFileEngine *create(const QString &fileName) const;
	static bool init();
	static void addFile(const QString &filePath, const QByteArray &ba);
	static QString prefix();

private:
	NSkinFileSystem();
	static QHash<QString, QByteArray> m_fileHash;
	static QString m_prefix;
	static NSkinFileSystem *m_instance;
};

#endif

/* vim: set ts=4 sw=4: */
