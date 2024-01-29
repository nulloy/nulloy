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

#include <QSignalSpy>
#include <QtTest/QtTest>

#include "playbackEngineInterface.h"
#include "playlistWidget.h"
#include "playlistWidgetItem.h"
#include "pluginLoader.h"
#include "settings.h"

#define INPUT_DELAY_MSEC 50
#define PLAY_WAIT_MSEC 100
#define CROSSFADING_POS 0.99 // 100 msec till the end (samples are 10 seconds)
#define PLAYNEXT_WAIT_MSEC 300

Q_DECLARE_METATYPE(NPlaylistWidgetItem *);

class TestPlaylistWidget : public QObject
{
    Q_OBJECT

    NPlaylistWidget *m_playlistWidget{};
    NPlaybackEngineInterface *m_playbackEngine{};

private slots:
    void initTestCase()
    {
        qRegisterMetaType<NPlaylistWidgetItem *>();
        NPluginLoader::init();
    }

    void init()
    {
        NSettings::instance()->clear();
        delete NSettings::instance();

        m_playlistWidget = new NPlaylistWidget;

        m_playbackEngine = dynamic_cast<NPlaybackEngineInterface *>(
            NPluginLoader::getPlugin(N::PlaybackEngine));
        Q_ASSERT(m_playbackEngine);

        connect(m_playbackEngine, SIGNAL(message(N::MessageIcon, const QString &, const QString &)),
                this, SLOT(message(N::MessageIcon, const QString &, const QString &)));
    }

    void cleanup()
    {
        delete m_playlistWidget;
        m_playlistWidget = nullptr;
        m_playbackEngine = nullptr;
    }

    void testPlaylistRemoval()
    {
        m_playlistWidget->show();
        QDir::setCurrent("tests");
        m_playlistWidget->setPlaylist("playlist.m3u");
        QCOMPARE(m_playlistWidget->count(), 10);

        // when deleting last, jumps to the "new" last
        NSettings::instance()->setValue("LoopPlaylist", false);
        m_playlistWidget->playRow(m_playlistWidget->count() - 1); // last row
        QTest::qWait(PLAY_WAIT_MSEC);
        QCOMPARE(m_playlistWidget->playingRow(), m_playlistWidget->count() - 1);
        m_playlistWidget->setCurrentRow(m_playlistWidget->count() - 1);
        // delete selected:
        QTest::keyClick(m_playlistWidget, Qt::Key_Delete, Qt::NoModifier, INPUT_DELAY_MSEC);
        QCOMPARE(m_playlistWidget->count(), 9);
        QCOMPARE(m_playlistWidget->playingRow(), m_playlistWidget->count() - 1);

        // when deleting last + looping enabled, jumps to the first
        NSettings::instance()->setValue("LoopPlaylist", true);
        m_playlistWidget->playRow(m_playlistWidget->count() - 1); // last row
        QTest::qWait(PLAY_WAIT_MSEC);
        QCOMPARE(m_playlistWidget->playingRow(), m_playlistWidget->count() - 1);
        m_playlistWidget->setCurrentRow(m_playlistWidget->count() - 1);
        // delete selected:
        QTest::keyClick(m_playlistWidget, Qt::Key_Delete, Qt::NoModifier, INPUT_DELAY_MSEC);
        QCOMPARE(m_playlistWidget->count(), 8);
        QCOMPARE(m_playlistWidget->playingRow(), 0);

        // when deleting the first, jumps to the "new" first
        m_playlistWidget->setCurrentRow(0);
        // delete selected:
        QTest::keyClick(m_playlistWidget, Qt::Key_Delete, Qt::NoModifier, INPUT_DELAY_MSEC);
        QCOMPARE(m_playlistWidget->count(), 7);
        QCOMPARE(m_playlistWidget->playingRow(), 0);

        // deleting neighbour rows doesn't change the playing item
        {
            m_playlistWidget->playRow(2); // 3rd
            QTest::qWait(PLAY_WAIT_MSEC);
            NPlaylistWidgetItem *item = m_playlistWidget->playingItem();
            // go 2nd
            m_playlistWidget->setCurrentRow(1);
            // select 2nd and 4th:
            // go down and keep old selection:
            QTest::keyClick(m_playlistWidget, Qt::Key_Down, Qt::ControlModifier, INPUT_DELAY_MSEC);
            // go down and keep old selection:
            QTest::keyClick(m_playlistWidget, Qt::Key_Down, Qt::ControlModifier, INPUT_DELAY_MSEC);
            // select current:
            QTest::keyClick(m_playlistWidget, Qt::Key_Space, Qt::ControlModifier, INPUT_DELAY_MSEC);
            QCOMPARE(m_playlistWidget->selectedItems().count(), 2);
            // delete selected:
            QTest::keyClick(m_playlistWidget, Qt::Key_Delete, Qt::NoModifier, INPUT_DELAY_MSEC);
            QCOMPARE(m_playlistWidget->count(), 5);
            QCOMPARE(m_playlistWidget->playingRow(), 1);
            QCOMPARE(item, m_playlistWidget->playingItem());
            QCOMPARE(m_playlistWidget->selectedItems().count(), 1);
            QCOMPARE(m_playlistWidget->currentRow(), 1); // keyboard focus
        }

        // when deleting the current, focus remains + focused becomes new current
        {
            int focused = m_playlistWidget->currentRow();
            int current = m_playlistWidget->playingRow();
            QCOMPARE(focused, current);
            // delete selected:
            QTest::keyClick(m_playlistWidget, Qt::Key_Delete, Qt::NoModifier, INPUT_DELAY_MSEC);
            int newFocused = m_playlistWidget->currentRow();
            int newCurrent = m_playlistWidget->playingRow();
            QCOMPARE(focused, newFocused);
            QCOMPARE(current, newCurrent);
            QCOMPARE(newFocused, newCurrent);
            QCOMPARE(m_playlistWidget->count(), 4);
        }

        // removing all stops playback
        {
            QSignalSpy spy(m_playbackEngine, SIGNAL(mediaChanged(const QString &, int)));
            // select all:
            QTest::keyClick(m_playlistWidget, Qt::Key_A, Qt::ControlModifier, INPUT_DELAY_MSEC);
            // delete selected:
            QTest::keyClick(m_playlistWidget, Qt::Key_Delete, Qt::NoModifier, INPUT_DELAY_MSEC);
            QCOMPARE(m_playlistWidget->count(), 0);

            QCOMPARE(spy.count(), 1);
            QList<QVariant> arguments = spy.takeFirst();
            QVERIFY(arguments.at(0).toString() == "");
        }
    }

    void testAutoPlay()
    {
        m_playbackEngine->stop();

        QSignalSpy spy(m_playbackEngine, SIGNAL(mediaChanged(const QString &, int)));
        int count = 0;
        int row = 0;

        m_playlistWidget->show();
        QDir::setCurrent("tests");
        m_playlistWidget->setPlaylist("playlist.m3u");

        m_playlistWidget->playRow(row);
        QTest::qWait(PLAY_WAIT_MSEC);
        ++count;
        QCOMPARE(spy.count(), count);
        m_playbackEngine->setPosition(CROSSFADING_POS);
        QTest::qWait(PLAYNEXT_WAIT_MSEC);
        ++row;
        ++count;
        QCOMPARE(m_playlistWidget->playingRow(), row);
        QCOMPARE(spy.count(), count);
    }

    void testRepeat()
    {
        m_playbackEngine->stop();

        QSignalSpy spy(m_playbackEngine, SIGNAL(mediaChanged(const QString &, int)));
        int count = 0;
        int row = 0;

        m_playlistWidget->setRepeatMode(true);
        m_playlistWidget->show();
        QDir::setCurrent("tests");
        m_playlistWidget->setPlaylist("playlist.m3u");

        m_playlistWidget->playRow(row);
        QTest::qWait(PLAY_WAIT_MSEC);
        ++count;
        QCOMPARE(spy.count(), count);
        m_playbackEngine->setPosition(CROSSFADING_POS);
        QTest::qWait(PLAYNEXT_WAIT_MSEC);
        ++count;
        QCOMPARE(spy.count(), count);
        QCOMPARE(m_playlistWidget->playingRow(), row);

        // select all:
        QTest::keyClick(m_playlistWidget, Qt::Key_A, Qt::ControlModifier, INPUT_DELAY_MSEC);
        // unselect current:
        QTest::keyClick(m_playlistWidget, Qt::Key_Space, Qt::ControlModifier, INPUT_DELAY_MSEC);
        // delete selected:
        QTest::keyClick(m_playlistWidget, Qt::Key_Delete, Qt::NoModifier, INPUT_DELAY_MSEC);
        QCOMPARE(m_playlistWidget->count(), 1);
        QCOMPARE(m_playlistWidget->playingRow(), row);
        QCOMPARE(spy.count(), count);

        m_playbackEngine->setPosition(CROSSFADING_POS);
        QTest::qWait(PLAYNEXT_WAIT_MSEC);
        ++count;
        QCOMPARE(spy.count(), count);
        QCOMPARE(m_playlistWidget->playingRow(), row);
    }

    void message(N::MessageIcon, const QString &, const QString &msg)
    {
        QFAIL(msg.toUtf8().constData());
    }
};

QTEST_MAIN(TestPlaylistWidget)
#include "testPlaylistWidget.moc"
