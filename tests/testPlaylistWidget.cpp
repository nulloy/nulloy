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

#define DELAY 50
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

        connect(m_playbackEngine, SIGNAL(aboutToFinish()), m_playlistWidget,
                SLOT(playingFinished()), Qt::BlockingQueuedConnection);
        connect(m_playbackEngine, SIGNAL(finished()), m_playlistWidget, SLOT(playingFinished()));
        connect(m_playbackEngine, SIGNAL(message(N::MessageIcon, const QString &, const QString &)),
                this, SLOT(message(N::MessageIcon, const QString &, const QString &)));
        connect(m_playlistWidget, &NPlaylistWidget::itemPlayingStarted,
                [this](NPlaylistWidgetItem *item) {
                    if (item) {
                        m_playbackEngine->setMedia(item->data(N::PathRole).toString());
                        m_playbackEngine->play();
                        QTest::qWait(100); // give playback engine time to start playback
                    } else {
                        m_playbackEngine->setMedia("");
                    }
                });
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
        QCOMPARE(m_playlistWidget->playingRow(), m_playlistWidget->count() - 1);
        m_playlistWidget->setCurrentRow(m_playlistWidget->count() - 1);
        QTest::keyClick(m_playlistWidget, Qt::Key_Delete, 0, DELAY);
        QCOMPARE(m_playlistWidget->count(), 9);
        QCOMPARE(m_playlistWidget->playingRow(), m_playlistWidget->count() - 1);

        // when deleting last + looping enabled, jumps to the first
        NSettings::instance()->setValue("LoopPlaylist", true);
        m_playlistWidget->playRow(m_playlistWidget->count() - 1); // last row
        QCOMPARE(m_playlistWidget->playingRow(), m_playlistWidget->count() - 1);
        m_playlistWidget->setCurrentRow(m_playlistWidget->count() - 1);
        QTest::keyClick(m_playlistWidget, Qt::Key_Delete, 0, DELAY);
        QCOMPARE(m_playlistWidget->count(), 8);
        QCOMPARE(m_playlistWidget->playingRow(), 0);

        // when deleting the first, jumps to the "new" first
        m_playlistWidget->setCurrentRow(0);
        QTest::keyClick(m_playlistWidget, Qt::Key_Delete, 0, DELAY);
        QCOMPARE(m_playlistWidget->count(), 7);
        QCOMPARE(m_playlistWidget->playingRow(), 0);

        // deleting neighbour rows doesn't change the playing item
        {
            m_playlistWidget->playRow(2); // 3rd
            NPlaylistWidgetItem *item = m_playlistWidget->playingItem();
            // go 2nd
            m_playlistWidget->setCurrentRow(1);
            // select 2nd and 4th
            QTest::keyClick(m_playlistWidget, Qt::Key_Down, Qt::ControlModifier, DELAY);
            QTest::keyClick(m_playlistWidget, Qt::Key_Down, Qt::ControlModifier, DELAY);
            QTest::keyClick(m_playlistWidget, Qt::Key_Space, Qt::ControlModifier, DELAY);
            QCOMPARE(m_playlistWidget->selectedItems().count(), 2);
            // delete
            QTest::keyClick(m_playlistWidget, Qt::Key_Delete, 0, DELAY);
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
            QTest::keyClick(m_playlistWidget, Qt::Key_Delete, 0, DELAY);
            int newFocused = m_playlistWidget->currentRow();
            int newCurrent = m_playlistWidget->playingRow();
            QCOMPARE(focused, newFocused);
            QCOMPARE(current, newCurrent);
            QCOMPARE(newFocused, newCurrent);
            QCOMPARE(m_playlistWidget->count(), 4);
        }

        // removing all stops playback
        {
            QSignalSpy spy(m_playlistWidget, &NPlaylistWidget::itemPlayingStarted);

            QTest::keyClick(m_playlistWidget, Qt::Key_A, Qt::ControlModifier, DELAY);
            QTest::keyClick(m_playlistWidget, Qt::Key_Delete, 0, DELAY);
            QCOMPARE(m_playlistWidget->count(), 0);

            QCOMPARE(spy.count(), 1);
            QList<QVariant> arguments = spy.takeFirst();
            QVERIFY(arguments.at(0).toString() == "");
        }
    }

    void testAutoPlay()
    {
        m_playbackEngine->stop();

        QSignalSpy spy(m_playlistWidget, &NPlaylistWidget::itemPlayingStarted);
        int count = 0;
        int row = 0;

        m_playlistWidget->show();
        QDir::setCurrent("tests");
        m_playlistWidget->setPlaylist("playlist.m3u");

        m_playlistWidget->playRow(row);
        ++count;
        QCOMPARE(spy.count(), count);
        m_playbackEngine->setPosition(0.8);
        QTest::qWait(500);
        ++row;
        ++count;
        QCOMPARE(m_playlistWidget->playingRow(), row);
        QCOMPARE(spy.count(), count);
    }

    void testRepeat()
    {
        m_playbackEngine->stop();

        QSignalSpy spy(m_playlistWidget, &NPlaylistWidget::itemPlayingStarted);
        int count = 0;
        int row = 0;

        m_playlistWidget->setRepeatMode(true);
        m_playlistWidget->show();
        QDir::setCurrent("tests");
        m_playlistWidget->setPlaylist("playlist.m3u");

        m_playlistWidget->playRow(row);
        ++count;
        QCOMPARE(spy.count(), count);
        m_playbackEngine->setPosition(0.8);
        QTest::qWait(500);
        ++count;
        QCOMPARE(spy.count(), count);
        QCOMPARE(m_playlistWidget->playingRow(), row);

        QTest::keyClick(m_playlistWidget, Qt::Key_A, Qt::ControlModifier, DELAY);
        QTest::keyClick(m_playlistWidget, Qt::Key_Space, Qt::ControlModifier, DELAY);
        QTest::keyClick(m_playlistWidget, Qt::Key_Delete, 0, DELAY);
        QCOMPARE(m_playlistWidget->count(), 1);
        QCOMPARE(m_playlistWidget->playingRow(), row);
        QCOMPARE(spy.count(), count);

        m_playbackEngine->setPosition(0.8);
        QTest::qWait(500);
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
