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

#ifndef N_COVER_READER_TAGLIB_H
#define N_COVER_READER_TAGLIB_H

#include <apefile.h>
#include <apetag.h>
#include <asffile.h>
#include <fileref.h>
#include <flacfile.h>
#include <id3v2tag.h>
#include <mp4file.h>
#include <mpcfile.h>
#include <mpegfile.h>
#include <vorbisfile.h>
#include <wavpackfile.h>

#include "coverReaderInterface.h"
#include "plugin.h"

class NCoverReaderTaglib : public NCoverReaderInterface, public NPlugin
{
    Q_OBJECT
    Q_INTERFACES(NCoverReaderInterface NPlugin)

private:
    QImage fromTagBytes(const TagLib::ByteVector &data) const;
    QImage fromApe(TagLib::APE::Tag *tag) const;
    QImage fromAsf(TagLib::ASF::Tag *tag) const;
    QImage fromFlac(TagLib::FLAC::File *file) const;
    QImage fromId3(TagLib::ID3v2::Tag *tag) const;
    QImage fromMp4(TagLib::MP4::Tag *tag) const;
    QImage fromVorbis(TagLib::Tag *tag) const;

public:
    NCoverReaderTaglib(QObject *parent = 0) : NCoverReaderInterface(parent) {}
    ~NCoverReaderTaglib();

    void init();
    QString interfaceString() const { return NCoverReaderInterface::interfaceString(); }
    N::PluginType type() const { return N::CoverReader; }

    void setSource(const QString &file);
    QImage getImage() const;
    bool isValid() const;
};

#endif
