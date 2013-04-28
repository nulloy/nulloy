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

#ifndef N_TAG_READER_TAGLIB_H
#define N_TAG_READER_TAGLIB_H

#include "pluginElementInterface.h"
#include "tagReaderInterface.h"

#include <QString>
#include <taglib/tag.h>
#include <taglib/fileref.h>

class NTagReaderTaglib : public NTagReaderInterface, public NPluginElementInterface
{
	Q_OBJECT
	Q_INTERFACES(NTagReaderInterface NPluginElementInterface)

private:
	QString m_path;
	TagLib::FileRef *m_tagRef;
	QString parse(const QString &format, bool *success, bool stopOnFail = FALSE);

public:
	NTagReaderTaglib(QObject *parent = 0) : NTagReaderInterface(parent) {}
	~NTagReaderTaglib();

	void init();
	QString interface() { return NTagReaderInterface::interface(); }
	PluginType type() { return TagReader; }

	void setSource(const QString &file);
	QString toString(const QString &format);
	bool isValid();
};

#endif

/* vim: set ts=4 sw=4: */
