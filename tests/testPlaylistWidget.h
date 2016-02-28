/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2016 Sergey Vlasov <sergey@vlasov.me>
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

#ifndef TEST_PLAYLIST_WIDGET_H
#define TEST_PLAYLIST_WIDGET_H

#include <QtTest/QtTest>
#include "playlistWidget.h"
#include "settings.h"
#include <QSignalSpy>

#define DELAY 50

class TestPlaylistWidget: public QObject
{
    Q_OBJECT

private slots:
    void testRemoval()
    {
        NPlaylistWidget widget;
        widget.show();

        widget.setPlaylist("tests/playlist.m3u");
        QCOMPARE(widget.count(), 10);

        // when deleting last, jumps to the "new" last
        NSettings::instance()->setValue("LoopPlaylist", false);
        widget.playRow(widget.count() - 1); // last row
        QCOMPARE(widget.currentRow(), widget.count() - 1);
        reinterpret_cast<QListWidget *>(&widget)->setCurrentRow(widget.count() - 1);
        QTest::keyClick(&widget, Qt::Key_Delete, 0, DELAY);
        QCOMPARE(widget.count(), 9);
        QCOMPARE(widget.currentRow(), widget.count() - 1);

        // when deleting last + looping enabled, jumps to the first
        NSettings::instance()->setValue("LoopPlaylist", true);
        widget.playRow(widget.count() - 1); // last row
        QCOMPARE(widget.currentRow(), widget.count() - 1);
        reinterpret_cast<QListWidget *>(&widget)->setCurrentRow(widget.count() - 1);
        QTest::keyClick(&widget, Qt::Key_Delete, 0, DELAY);
        QCOMPARE(widget.count(), 8);
        QCOMPARE(widget.currentRow(), 0);

        // when deleting the first, jumps to the "new" first
        reinterpret_cast<QListWidget *>(&widget)->setCurrentRow(0);
        QTest::keyClick(&widget, Qt::Key_Delete, 0, DELAY);
        QCOMPARE(widget.count(), 7);
        QCOMPARE(widget.currentRow(), 0);

        // deleting neighbour rows doesn't change the playing item
        {
            widget.playRow(2); // 3rd
            NPlaylistWidgetItem* item = widget.item(widget.currentRow());
            // go 2nd
            reinterpret_cast<QListWidget *>(&widget)->setCurrentRow(1);
            // select 2nd and 4th
            QTest::keyClick(&widget, Qt::Key_Down, Qt::ControlModifier, DELAY);
            QTest::keyClick(&widget, Qt::Key_Down, Qt::ControlModifier, DELAY);
            QTest::keyClick(&widget, Qt::Key_Space, Qt::ControlModifier, DELAY);
            QCOMPARE(widget.selectedItems().count(), 2);
            // delete
            QTest::keyClick(&widget, Qt::Key_Delete, 0, DELAY);
            QCOMPARE(widget.count(), 5);
            QCOMPARE(widget.currentRow(), 1);
            QCOMPARE(item, widget.item(widget.currentRow()));
            QCOMPARE(widget.selectedItems().count(), 1);
            QCOMPARE(reinterpret_cast<QListWidget *>(&widget)->currentRow(), 1); // keyboard focus
        }

        // when deleting the current, focus remains + focused becomes new current
        {
            int focused = reinterpret_cast<QListWidget *>(&widget)->currentRow();
            int current = widget.currentRow();
            QCOMPARE(focused, current);
            QTest::keyClick(&widget, Qt::Key_Delete, 0, DELAY);
            int newFocused = reinterpret_cast<QListWidget *>(&widget)->currentRow();
            int newCurrent = widget.currentRow();
            QCOMPARE(focused, newFocused);
            QCOMPARE(current, newCurrent);
            QCOMPARE(newFocused, newCurrent);
            QCOMPARE(widget.count(), 4);
        }

        // removing all stops playback
        {
            QSignalSpy spy(&widget, SIGNAL(setMedia(const QString &)));

            QTest::keyClick(&widget, Qt::Key_A, Qt::ControlModifier, DELAY);
            QTest::keyClick(&widget, Qt::Key_Delete, 0, DELAY);
            QCOMPARE(widget.count(), 0);

            QCOMPARE(spy.count(), 1);
            QList<QVariant> arguments = spy.takeFirst();
            QVERIFY(arguments.at(0).toString() == "");
        }
    }
};

#endif
