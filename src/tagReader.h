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

#ifndef N_TAG_READER_H
#define N_TAG_READER_H

#include <QString>

namespace TagLib {
class FileRef;
}

class NTagReader
{
private:
	QString m_path;
	TagLib::FileRef *m_tagRef;

public:
	NTagReader(const QString &file);
	~NTagReader();
	QString toString(const QString &format);
	bool isValid();
};

#endif

/* vim: set ts=4 sw=4: */
