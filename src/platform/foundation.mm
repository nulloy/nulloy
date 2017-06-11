/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2017 Sergey Vlasov <sergey@vlasov.me>
**  Copyright (C) 2016 The Qt Company Ltd.
**  Copyright (C) 2014 Samuel Gaist <samuel.gaist@edeltech.ch>
**  Copyright (C) 2014 Petroules Corporation.
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

#import "Cocoa/Cocoa.h"
#import "Foundation/Foundation.h"
#include "foundation.h"

QString QStringFromNSString(const NSString *string)
{
   if (!string)
      return QString();
   QString qstring;
   qstring.resize([string length]);
   [string getCharacters: reinterpret_cast<unichar*>(qstring.data()) range: NSMakeRange(0, [string length])];
   return qstring;
}

NSString *QStringToNSString(const QString &string)
{
   return [NSString stringWithCharacters: reinterpret_cast<const UniChar*>(string.unicode()) length: string.length()];
}

QUrl QUrlFromNSURL(const NSURL *url)
{
   if (!url)
      return QUrl();
   return QUrl(QStringFromNSString([url absoluteString]));
}

NSURL *QUrlToNSURL(const QUrl &url)
{
   return [NSURL URLWithString:QStringToNSString(url.toString())];
}

QUrl filePathURL(const QUrl &url)
{
   QUrl ret = url;
   @autoreleasepool {
      NSURL *nsurl = QUrlToNSURL(url);
      if ([nsurl isFileReferenceURL]) {
         ret = QUrl(QStringFromNSString([nsurl path]));
      }
   }
   return ret;
}
