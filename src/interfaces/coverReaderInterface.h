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

#ifndef N_COVER_ART_READER_INTERFACE_H
#define N_COVER_ART_READER_INTERFACE_H

#include <QObject>

class QString;

#define COVERREADER_INTERFACE "Nulloy/NCoverReaderInterface/0.7"

class NCoverReaderInterface : public QObject
{
public:
    NCoverReaderInterface(QObject *parent = 0) : QObject(parent) {}
    virtual ~NCoverReaderInterface() {}

    static QString interfaceString() { return COVERREADER_INTERFACE; }

    virtual void setSource(const QString &file) = 0;
    virtual QImage getImage() const = 0;
    virtual bool isValid() const = 0;
};

Q_DECLARE_INTERFACE(NCoverReaderInterface, COVERREADER_INTERFACE)

#endif
