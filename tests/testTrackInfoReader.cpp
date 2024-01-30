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

#include <QMap>
#include <QtTest/QtTest>

#include "tagReaderInterface.h"
#include "trackInfoReader.h"

class NTagReaderMock : public NTagReaderInterface
{
    Q_OBJECT

public:
    QMap<QChar, QString> tags;

    NTagReaderMock(QObject *parent = 0) : NTagReaderInterface(parent)
    {
        tags['a'] = "<a>";
        tags['t'] = "<t>";
        tags['A'] = "<A>";
        tags['c'] = "<c>";
        tags['g'] = "<g>";
        tags['y'] = "<y>";
        tags['n'] = "<n>";
        tags['b'] = "<b>";
        tags['D'] = ""; // set by test
        tags['B'] = "<B>";
        tags['s'] = "<s>";
        tags['H'] = "<H>";
    }

    QString getTag(QChar ch) const override
    {
        if (tags.contains(ch)) {
            return tags[ch];
        } else {
            return QString('%') + ch;
        }
    }

    void setSource(const QString &) override {}
};

class TrackInfoReaderTest : public QObject
{
    Q_OBJECT

private:
    NTrackInfoReader *m_infoReader;
    NTagReaderMock *m_tagReader;

private slots:
    void init()
    {
        m_tagReader = new NTagReaderMock();
        m_infoReader = new NTrackInfoReader(m_tagReader);
        m_infoReader->setSource("/music/some.mp3");
    }

    void cleanup()
    {
        delete m_infoReader;
        delete m_tagReader;
    }

    void test1()
    {
        // special characters escaped:
        QCOMPARE(m_infoReader->toString("\\{test \\\\\\%\\\\ \\|\\}"), "{test \\%\\ |}");
    }

    void test2()
    {
        QCOMPARE(m_infoReader->toString("{%a - %t|%F}"), "<a> - <t>");
        // with extra alternatives:
        QCOMPARE(m_infoReader->toString("{%a - %t|||%F|||}"), "<a> - <t>");
    }

    void test3()
    {
        m_tagReader->tags['a'] = ""; // artist not set
        QCOMPARE(m_infoReader->toString("{%a - %t|%F}"), "some.mp3");
        // with extra alternatives:
        QCOMPARE(m_infoReader->toString("{%a - %t|hello|%F}"), "hello");
        m_infoReader->setSource(""); // file not set
        QCOMPARE(m_infoReader->toString("{%a - %t|%F}"), "");
    }

    void test4()
    {
        m_tagReader->tags['D'] = "10";              // 10 seconds
        m_infoReader->setSource("/music/some.mp3"); // to re-cache the duration value
        QCOMPARE(m_infoReader->toString("%F{ (%d)}"), "some.mp3 (0:10)");
    }

    void test5()
    {
        m_tagReader->tags['D'] = "";                // duration unknown
        m_infoReader->setSource("/music/some.mp3"); // to re-cache the duration value
        QCOMPARE(m_infoReader->toString("%F{ (%d)}"), "some.mp3");
    }

    void test6()
    {
        m_tagReader->tags['D'] = "60"; // 1 minute
        m_infoReader->setSource("");   // to re-cache the duration value
        m_infoReader->updatePlaybackPosition(34);
        QCOMPARE(m_infoReader->toString("%d"), "1:00");
        QCOMPARE(m_infoReader->toString("%T"), "0:34");
        QCOMPARE(m_infoReader->toString("%r"), "0:26");

        m_tagReader->tags['D'] = QString::number(60 * 60); // 1 hour
        m_infoReader->setSource("");                       // to re-cache the duration value
        QCOMPARE(m_infoReader->toString("%d"), "1:00:00");

        m_tagReader->tags['D'] = QString::number(60 * 60 * 72); // 72 hours
        m_infoReader->setSource("");                            // to re-cache the duration value
        QCOMPARE(m_infoReader->toString("%d"), "72:00:00");
    }

    void test7()
    {
        QCOMPARE(m_infoReader->toString("{%B kbps/%s kHz|{%B kbps}{%s kHz}}"), "<B> kbps/<s> kHz");
    }

    void test8()
    {
        m_tagReader->tags['B'] = ""; // unknown bitrate
        QCOMPARE(m_infoReader->toString("{%B kbps/%s kHz|{%B kbps}{%s kHz}}"), "<s> kHz");
    }

    void test9()
    {
        m_tagReader->tags['s'] = ""; // unknown sample rate
        QCOMPARE(m_infoReader->toString("{%B kbps/%s kHz|{%B kbps}{%s kHz}}"), "<B> kbps");
    }

    void test10()
    {
        m_tagReader->tags['B'] = ""; // unknown bitrate
        m_tagReader->tags['s'] = ""; // unknown sample rate
        QCOMPARE(m_infoReader->toString("{%B kbps/%s kHz|{%B kbps}{%s kHz}}"), "");
    }

    void test11()
    {
        QCOMPARE(m_infoReader->toString("{\"%a - %t\" - |\"%F\" - }Nulloy"),
                 "\"<a> - <t>\" - Nulloy");
    }

    void test12()
    {
        m_tagReader->tags['a'] = ""; // artist not set
        QCOMPARE(m_infoReader->toString("{\"%a - %t\" - |\"%F\" - }Nulloy"),
                 "\"some.mp3\" - Nulloy");
    }

    void test13()
    {
        m_tagReader->tags['a'] = ""; // artist not set
        m_infoReader->setSource(""); // file not set
        QCOMPARE(m_infoReader->toString("{\"%a - %t\" - |\"%F\" - }Nulloy"), "Nulloy");
    }
};

QTEST_MAIN(TrackInfoReaderTest)
#include "testTrackInfoReader.moc"
