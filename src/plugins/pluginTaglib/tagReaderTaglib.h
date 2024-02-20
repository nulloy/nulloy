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

#ifndef N_TAG_READER_TAGLIB_H
#define N_TAG_READER_TAGLIB_H

#include "plugin.h"
#include "tagReaderInterface.h"

#include <fileref.h>
#include <tag.h>
#include <tmap.h>

class QString;

class NTagReaderTaglib : public NTagReaderInterface, public NPlugin
{
    Q_OBJECT
    Q_INTERFACES(NTagReaderInterface NPlugin)

private:
    bool m_isValid;
    bool m_isUtf8;
    QTextCodec *m_codec;

public:
    NTagReaderTaglib(QObject *parent = 0);
    ~NTagReaderTaglib();

    void init();
    QString interfaceString() const { return NTagReaderInterface::interfaceString(); }
    N::PluginType type() const { return N::TagReader; }

    void setSource(const QString &file);
    void setEncoding(const QString &encoding);
    QString getTag(QChar ch) const;
    N::Tag tagFromKey(const QString &key) const;
    QString tagToKey(N::Tag tag) const;

    bool isWriteSupported() const { return true; }
    QMap<QString, QStringList> getTags() const;
    QMap<QString, QStringList> setTags(const QMap<QString, QStringList> &tags);

    QMap<QString, QStringList>
    TMapToQMap(const TagLib::Map<TagLib::String, TagLib::StringList> &tmap) const;
    TagLib::Map<TagLib::String, TagLib::StringList>
    QMapToTMap(const QMap<QString, QStringList> &qmap) const;
};

#endif
