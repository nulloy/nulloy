/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2015 Sergey Vlasov <sergey@vlasov.me>
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

#ifndef N_COVER_READER_TAGLIB_H
#define N_COVER_READER_TAGLIB_H

#include "plugin.h"
#include "coverReaderInterface.h"

#include <apefile.h>
#include <apetag.h>
#include <asffile.h>
#include <fileref.h>
#include <flacfile.h>
#include <id3v2tag.h>
#include <mpcfile.h>
#include <mp4file.h>
#include <mpegfile.h>
#include <vorbisfile.h>
#include <wavpackfile.h>

class NCoverReaderTaglib : public NCoverReaderInterface, public NPlugin
{
	Q_OBJECT
	Q_INTERFACES(NCoverReaderInterface NPlugin)

private:
	QImage fromTagBytes(const TagLib::ByteVector &data);
	QImage fromApe(TagLib::APE::Tag *tag);
	QImage fromAsf(TagLib::ASF::Tag *tag);
	QImage fromFlac(TagLib::FLAC::File *file);
	QImage fromId3(TagLib::ID3v2::Tag *tag);
	QImage fromMp4(TagLib::MP4::Tag *tag);
	QImage fromVorbis(TagLib::Tag *tag);

public:
	NCoverReaderTaglib(QObject *parent = 0) : NCoverReaderInterface(parent) {}
	~NCoverReaderTaglib();

	void init();
	QString interfaceString() { return NCoverReaderInterface::interfaceString(); }
	N::PluginType type() { return N::CoverReader; }

	void setSource(const QString &file);
	QImage getImage();
	bool isValid();
};

#endif

