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

#include <QObject>

class QString;

#define TAGREADER_INTERFACE "Nulloy/NTagReaderInterface/0.9"

class NTagReaderInterface : public QObject
{
public:
    NTagReaderInterface(QObject *parent = 0) : QObject(parent) {}
    virtual ~NTagReaderInterface() {}

    static QString interfaceString() { return TAGREADER_INTERFACE; }

    virtual QString toString(const QString &file, const QString &format,
                             const QString &encoding = QString("UTF-8")) = 0;
};

Q_DECLARE_INTERFACE(NTagReaderInterface, TAGREADER_INTERFACE)

#endif
