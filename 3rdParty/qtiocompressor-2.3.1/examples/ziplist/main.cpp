/****************************************************************************
** 
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
** 
** This file is part of a Qt Solutions component.
**
** Commercial Usage  
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Solutions Commercial License Agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and Nokia.
** 
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
** 
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.1, included in the file LGPL_EXCEPTION.txt in this
** package.
** 
** GNU General Public License Usage 
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
** 
** Please note Third Party Software included with Qt Solutions may impose
** additional restrictions and it is the user's responsibility to ensure
** that they have met the licensing requirements of the GPL, LGPL, or Qt
** Solutions Commercial license and the relevant license of the Third
** Party Software they are using.
** 
** If you are unsure which license is appropriate for your use, please
** contact Nokia at qt-info@nokia.com.
** 
****************************************************************************/

#include <QtCore/QFile>
#include <QtCore/QBuffer>
#include <QtCore/QTextStream>
#include <QtCore/QDebug>
#include <qtiocompressor.h>

/*
  This example demonstrates the use of QtIOCompressor's RawZipFormat
  to read ZIP archive files. It implements a rudimentary parsing of
  the primary ZIP file header, and then uses QtIOCompressor to
  uncompress the actual compressed file data.

  Note: only a subset of the ZIP file format specification is
  supported. For the full specification, see
  http://www.pkware.com/documents/casestudies/APPNOTE.TXT

  Output is a list of the compressed files contained in the archive,
  and their (uncompressed) sizes.
*/





int main(int argc, char **argv)
{
    if (argc != 2) {
        qWarning() << "Usage:" << argv[0] << "<zipfile>";
        return 1;
    }

    QFile file(argv[1]);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file" << argv[1];
        return 1;
    }

    QTextStream sout(stdout);
    // Read all from file and print.
    sout << "Archive: " << argv[1] << endl;
    sout << "Item  Size Name" << endl;
    int item = 0;
    forever {
        // Zip format "local file header" fields:
        quint32 signature, crc, compSize, unCompSize;
        quint16 extractVersion, bitFlag, compMethod, modTime, modDate;
        quint16 nameLen, extraLen;

        QDataStream s(&file);
        s.setByteOrder(QDataStream::LittleEndian);
        s >> signature;
        if (signature != 0x04034b50)   // zip local file header magic number
            break;
        s >> extractVersion >> bitFlag >> compMethod;
        s >> modTime >> modDate >> crc >> compSize >> unCompSize;
        s >> nameLen >> extraLen;

        const QByteArray fileName = file.read(nameLen);
        file.read(extraLen);

        QByteArray compData = file.read(compSize);
        QByteArray unCompData;
        if (compMethod == 0) {
            unCompData = compData;
        }
        else {
            QBuffer compBuf(&compData);
            QtIOCompressor compressor(&compBuf);
            compressor.setStreamFormat(QtIOCompressor::RawZipFormat);
            compressor.open(QIODevice::ReadOnly);
            unCompData = compressor.readAll();
        }

        // unCompData now contains the uncompressed file from the zip archive
        sout << QString("%1 %2 ").arg(1+item++, 3).arg(unCompData.size(), 6)
             << fileName << endl;

        if (fileName.toLower().endsWith(".txt"))
            sout << "   Preview: \""
                 << unCompData.mid(0, unCompData.indexOf('\n')).replace('\r', "")
                 << "\"..." << endl;
    }

    if (!item) {
        qWarning() << "Not a ZIP file!";
        return 1;
    }

    return 0;
}
