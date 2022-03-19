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

#ifndef N_TAG_READER_INTERFACE_H
#define N_TAG_READER_INTERFACE_H

#include "global.h"
#include <QMap>
#include <QObject>

#define TAGREADER_INTERFACE "Nulloy/NTagReaderInterface/0.9.5"

class NTagReaderInterface : public QObject
{
public:
    NTagReaderInterface(QObject *parent = 0) : QObject(parent) {}
    virtual ~NTagReaderInterface() {}

    static QString interfaceString() { return TAGREADER_INTERFACE; }

    virtual void setSource(const QString &file) = 0;
    virtual void setEncoding(const QString &encoding) { Q_UNUSED(encoding); }
    virtual QString getTag(QChar ch) const = 0;
    virtual N::Tag tagFromKey(const QString &) const { return N::UnknownTag; }
    virtual QString tagToKey(N::Tag) const { return ""; }

    virtual bool isWriteSupported() const { return false; }
    virtual QMap<QString, QStringList> getTags() const { return QMap<QString, QStringList>(); }
    virtual QMap<QString, QStringList> setTags(const QMap<QString, QStringList> &) {}
};

Q_DECLARE_INTERFACE(NTagReaderInterface, TAGREADER_INTERFACE)

#endif
