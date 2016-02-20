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

#include "common.h"
#include "player.h"
#include "settings.h"
#include <qtsingleapplication.h>

#ifndef _N_NO_SKINS_
#include "skinFileSystem.h"
Q_IMPORT_PLUGIN(widget_collection)
#endif

static void print_out(const QString &out)
{
    QTextStream stream(stdout);
    stream << out  << "\n";
}

static void print_err(const QString &err)
{
    QTextStream stream(stderr);
    stream <<  NCore::applicationBasenameName() + ": " + err << "\n";
}

static void print_help()
{
    print_out(
        "Usage:  " + NCore::applicationBasenameName() + " [[option] | [files]]\n"
        "\n"
        "Options:\n"
        "    --next         play next file\n"
        "    --prev         play previous file\n"
        "    --stop         stop playback\n"
        "    --pause        pause playback\n"
        "    --version      print version\n"
        "    -h, --help     print this message\n"
    );
}

static void print_try()
{
    print_out("Try `" +  NCore::applicationBasenameName() + " --help' for more information");
}

int main(int argc, char *argv[])
{
#ifdef Q_WS_MAC
    // https://bugreports.qt-project.org/browse/QTBUG-32789
    if (QSysInfo::MacintoshVersion > QSysInfo::MV_10_8)
        QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");

    // https://bugreports.qt-project.org/browse/QTBUG-40833
    if (QSysInfo::MacintoshVersion > QSysInfo::MV_10_9)
        QFont::insertSubstitution(".Helvetica Neue DeskInterface", "Helvetica Neue");
#endif

    QtSingleApplication instance(argc, argv);
    instance.setApplicationName("Nulloy");
    instance.setApplicationVersion(QString(_N_VERSION_));
    instance.setOrganizationDomain("nulloy.com");
    instance.setQuitOnLastWindowClosed(false);

    QStringList argList = instance.arguments();
    argList.takeFirst();
    QStringList files;
    QStringList options;
    foreach (QString arg, argList) {
        if (arg.startsWith("-")) {
            if (arg == "-h") {
                print_help();
                return 0;
            } else if (arg.startsWith("--")) {
                if (
                    arg == "--next" ||
                    arg == "--prev" ||
                    arg == "--stop" ||
                    arg == "--pause")
                {
                    options << arg;
                } else if (arg == "--version") {
                    print_out(instance.applicationVersion());
                    return 0;
                } else if (arg == "--help") {
                    print_help();
                    return 0;
                } else {
                    print_err("unrecognized option '" + arg + "'");
                    print_try();
                    return 1;
                }
            } else {
                print_err("unrecognized option '" + arg + "'");
                print_try();
                return 1;
            }
        } else {
            files << arg;
        }
    }

    // construct message
    QString msg = (options + files).join(MSG_SPLITTER);
    if (NSettings::instance()->value("SingleInstance").toBool()) {
        // try to send it to an already running instrance
        if (instance.sendMessage(msg))
            return 0; // return if delivered
    }

    // for Qt core plugins
#if defined(Q_WS_WIN)
    instance.addLibraryPath(instance.applicationDirPath() + "/Plugins/");
#elif defined(Q_WS_MAC)
    instance.addLibraryPath(instance.applicationDirPath() + "/plugins/");
#endif

#ifndef _N_NO_SKINS_
    NSkinFileSystem::init();
#endif

    NPlayer p;
    QObject::connect(&instance, SIGNAL(messageReceived(const QString &)),
                     &p, SLOT(readMessage(const QString &)));
    QObject::connect(&instance, SIGNAL(aboutToQuit()),
                     &p, SLOT(quit()));

    if (NSettings::instance()->value("RestorePlaylist").toBool())
        p.loadDefaultPlaylist();

    // manually read the message
    if (!msg.isEmpty())
        p.readMessage(msg);

    instance.installEventFilter(&p);

    return instance.exec();
}

