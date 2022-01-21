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

#include <QtTest/QtTest>

#include "testPlaylistWidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    NPluginLoader::init();
    TestPlaylistWidget test;
    int res = QTest::qExec(&test);
    NPluginLoader::deinit();

    return res;
}
