/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2024 Sergey Vlasov <sergey@vlasov.me>
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
#include <attachedpictureframe.h>

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

QList<QImage> NCoverReaderTaglib::fromApe(TagLib::APE::Tag *tag) const
{
    QList<QImage> images;
    const TagLib::APE::ItemListMap &map = tag->itemListMap();

    for (auto iter = map.begin(); iter != map.end(); ++iter) {
        TagLib::String key = iter->first;
        if (!key.startsWith("COVER ART")) {
            continue;
        }

        TagLib::String fileName = map[key].toString();
        TagLib::ByteVector item = map[key].binaryData();
        images << fromTagBytes(item.mid(fileName.size() + 1));
    }

    return images;
}

QList<QImage> NCoverReaderTaglib::fromAsf(TagLib::ASF::Tag *tag) const
{
    QList<QImage> images;
    const TagLib::ASF::AttributeListMap &map = tag->attributeListMap();

    TagLib::String str = "WM/Picture";
    if (!map.contains(str)) {
        return images;
    }

    const TagLib::ASF::AttributeList &list = map[str];

    for (auto attribute : list) {
        TagLib::ASF::Picture pic = attribute.toPicture();
        if (pic.isValid()) {
            images << fromTagBytes(pic.picture());
        }
    }

    return images;
}

QList<QImage> NCoverReaderTaglib::fromFlac(TagLib::FLAC::File *file) const
{
    QList<QImage> images;
    const TagLib::List<TagLib::FLAC::Picture *> &list = file->pictureList();

    for (TagLib::FLAC::Picture *pic : list) {
        images << fromTagBytes(pic->data());
    }
    return images;
}

QList<QImage> NCoverReaderTaglib::fromId3(TagLib::ID3v2::Tag *tag) const
{
    QList<QImage> images;
    const TagLib::ID3v2::FrameList &list = tag->frameList("APIC");
    if (list.isEmpty()) {
        return images;
    }

    for (auto *frame : list) {
        auto pictureFrame = static_cast<TagLib::ID3v2::AttachedPictureFrame *>(frame);
        images << fromTagBytes(pictureFrame->picture());
    }
    return images;
}

QList<QImage> NCoverReaderTaglib::fromMp4(TagLib::MP4::Tag *tag) const
{
    QList<QImage> images;
    TagLib::String str = "covr";
    if (!tag->itemMap().contains(str)) {
        return images;
    }

    TagLib::MP4::CoverArtList coverList = tag->itemMap()[str].toCoverArtList();
    for (auto coverArt : coverList) {
        images << fromTagBytes(coverArt.data());
    }

    return images;
}

QList<QImage> NCoverReaderTaglib::fromVorbis(TagLib::Tag *tag) const
{
    QList<QImage> images;
    if (auto *comment = dynamic_cast<TagLib::Ogg::XiphComment *>(tag)) {
        TagLib::String str = "COVERART";

        if (!comment->contains(str)) {
            str = "METADATA_BLOCK_PICTURE";
        }

        if (!comment->contains(str)) {
            return images;
        }

        TagLib::ByteVector tagBytes = comment->fieldListMap()[str].front().data(
            TagLib::String::Latin1);
        QByteArray base64;
        base64.setRawData(tagBytes.data(), tagBytes.size());
        QImage image;
        image.loadFromData(QByteArray::fromBase64(base64));
        images << image;
    }

    return images;
}

QList<QImage> NCoverReaderTaglib::getImages() const
{
    QList<QImage> images;

    if (!isValid()) {
        return images;
    }

    TagLib::File *tagFile = NTaglib::_tagRef->file();

    if (auto *file = dynamic_cast<TagLib::APE::File *>(tagFile)) {
        if (file->APETag()) {
            images = fromApe(file->APETag());
        }
    } else if (auto *file = dynamic_cast<TagLib::ASF::File *>(tagFile)) {
        if (file->tag()) {
            images = fromAsf(file->tag());
        }
    } else if (auto *file = dynamic_cast<TagLib::FLAC::File *>(tagFile)) {
        images = fromFlac(file);

        if (images.isEmpty() && file->ID3v2Tag()) {
            images = fromId3(file->ID3v2Tag());
        }
    } else if (auto *file = dynamic_cast<TagLib::MP4::File *>(tagFile)) {
        if (file->tag()) {
            images = fromMp4(file->tag());
        }
    } else if (auto *file = dynamic_cast<TagLib::MPC::File *>(tagFile)) {
        if (file->APETag()) {
            images = fromApe(file->APETag());
        }
    } else if (auto *file = dynamic_cast<TagLib::MPEG::File *>(tagFile)) {
        if (file->ID3v2Tag()) {
            images = fromId3(file->ID3v2Tag());
        }

        if (images.isEmpty() && file->APETag()) {
            images = fromApe(file->APETag());
        }
    } else if (auto *file = dynamic_cast<TagLib::Ogg::Vorbis::File *>(tagFile)) {
        if (file->tag()) {
            images = fromVorbis(file->tag());
        }
    } else if (auto *file = dynamic_cast<TagLib::WavPack::File *>(tagFile)) {
        if (file->APETag()) {
            images = fromApe(file->APETag());
        }
    }

    return images;
}
