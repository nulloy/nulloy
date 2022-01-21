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

#include "coverReaderTaglib.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QImage>
#include <QString>

#include "tagLibFileRef.h"

void NCoverReaderTaglib::init()
{
    if (m_init) {
        return;
    }

    m_init = true;
    NTaglib::_tagRef = NULL;
}

void NCoverReaderTaglib::setSource(const QString &file)
{
    if (NTaglib::_filePath == file) {
        return;
    }
    NTaglib::_filePath = file;

    if (NTaglib::_tagRef) {
        delete NTaglib::_tagRef;
    }
#ifdef WIN32
    NTaglib::_tagRef = new TagLib::FileRef(reinterpret_cast<const wchar_t *>(file.constData()));
#else
    NTaglib::_tagRef = new TagLib::FileRef(file.toUtf8().data());
#endif
}

NCoverReaderTaglib::~NCoverReaderTaglib()
{
    if (!m_init) {
        return;
    }

    if (NTaglib::_tagRef) {
        delete NTaglib::_tagRef;
        NTaglib::_tagRef = NULL;
    }
}

bool NCoverReaderTaglib::isValid() const
{
    return (NTaglib::_tagRef && NTaglib::_tagRef->file() && NTaglib::_tagRef->file()->isValid());
}

QImage NCoverReaderTaglib::fromTagBytes(const TagLib::ByteVector &data) const
{
    QImage image;
    image.loadFromData((const uchar *)data.data(), data.size());
    return image;
}

QImage NCoverReaderTaglib::fromApe(TagLib::APE::Tag *tag) const
{
    const TagLib::APE::ItemListMap &map = tag->itemListMap();

    TagLib::String str = "COVER ART (FRONT)";
    if (!map.contains(str)) {
        return QImage();
    }

    TagLib::String fileName = map[str].toString();
    TagLib::ByteVector item = map[str].value();
    return fromTagBytes(item.mid(fileName.size() + 1));
}

QImage NCoverReaderTaglib::fromAsf(TagLib::ASF::Tag *tag) const
{
    const TagLib::ASF::AttributeListMap &map = tag->attributeListMap();

    TagLib::String str = "WM/Picture";
    if (!map.contains(str)) {
        return QImage();
    }

    const TagLib::ASF::AttributeList &list = map[str];
    if (list.isEmpty()) {
        return QImage();
    }

    TagLib::ASF::Picture pic = list[0].toPicture();
    if (pic.isValid()) {
        return fromTagBytes(pic.picture());
    }

    return QImage();
}

QImage NCoverReaderTaglib::fromFlac(TagLib::FLAC::File *file) const
{
    const TagLib::List<TagLib::FLAC::Picture *> &list = file->pictureList();
    if (list.isEmpty()) {
        return QImage();
    }

    TagLib::FLAC::Picture *pic = list[0];
    return fromTagBytes(pic->data());
}

QImage NCoverReaderTaglib::fromId3(TagLib::ID3v2::Tag *tag) const
{
    const TagLib::ID3v2::FrameList &list = tag->frameList("APIC");
    if (list.isEmpty()) {
        return QImage();
    }

    auto *frame = static_cast<TagLib::ID3v2::AttachedPictureFrame *>(list.front());
    return fromTagBytes(frame->picture());
}

QImage NCoverReaderTaglib::fromMp4(TagLib::MP4::Tag *tag) const
{
    TagLib::String str = "covr";
    if (!tag->itemListMap().contains(str)) {
        return QImage();
    }

    TagLib::MP4::CoverArtList coverList = tag->itemListMap()[str].toCoverArtList();
    if (coverList[0].data().size() > 0) {
        return fromTagBytes(coverList[0].data());
    }

    return QImage();
}

QImage NCoverReaderTaglib::fromVorbis(TagLib::Tag *tag) const
{
    if (auto *comment = dynamic_cast<TagLib::Ogg::XiphComment *>(tag)) {
        TagLib::String str = "COVERART";

        if (!comment->contains(str)) {
            str = "METADATA_BLOCK_PICTURE";
        }

        if (!comment->contains(str)) {
            return QImage();
        }

        TagLib::ByteVector tagBytes = comment->fieldListMap()[str].front().data(
            TagLib::String::Latin1);
        QByteArray base64;
        base64.setRawData(tagBytes.data(), tagBytes.size());
        QImage image;
        image.loadFromData(QByteArray::fromBase64(base64));
        return image;
    }

    return QImage();
}

QImage NCoverReaderTaglib::getImage() const
{
    QImage image;

    if (!isValid()) {
        return image;
    }

    TagLib::File *tagFile = NTaglib::_tagRef->file();

    if (auto *file = dynamic_cast<TagLib::APE::File *>(tagFile)) {
        if (file->APETag()) {
            image = fromApe(file->APETag());
        }
    } else if (auto *file = dynamic_cast<TagLib::ASF::File *>(tagFile)) {
        if (file->tag()) {
            image = fromAsf(file->tag());
        }
    } else if (auto *file = dynamic_cast<TagLib::FLAC::File *>(tagFile)) {
        image = fromFlac(file);

        if (image.isNull() && file->ID3v2Tag()) {
            image = fromId3(file->ID3v2Tag());
        }
    } else if (auto *file = dynamic_cast<TagLib::MP4::File *>(tagFile)) {
        if (file->tag()) {
            image = fromMp4(file->tag());
        }
    } else if (auto *file = dynamic_cast<TagLib::MPC::File *>(tagFile)) {
        if (file->APETag()) {
            image = fromApe(file->APETag());
        }
    } else if (auto *file = dynamic_cast<TagLib::MPEG::File *>(tagFile)) {
        if (file->ID3v2Tag()) {
            image = fromId3(file->ID3v2Tag());
        }

        if (image.isNull() && file->APETag()) {
            image = fromApe(file->APETag());
        }
    } else if (auto *file = dynamic_cast<TagLib::Ogg::Vorbis::File *>(tagFile)) {
        if (file->tag()) {
            image = fromVorbis(file->tag());
        }
    } else if (auto *file = dynamic_cast<TagLib::WavPack::File *>(tagFile)) {
        if (file->APETag()) {
            image = fromApe(file->APETag());
        }
    }

    return image;
}
