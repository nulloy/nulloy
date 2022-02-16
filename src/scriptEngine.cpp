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

#include "scriptEngine.h"

#include "global.h"
#include "mainWindow.h"
#include "playbackEngineInterface.h"
#include "player.h"
#include "scriptQtPrototypes.h"
#include "settings.h"

#ifdef Q_OS_LINUX
#include "xcb.h"
#endif

#ifndef _N_NO_SKINS_
#include "skinFileSystem.h"
#endif

NWidgetPrototype widgetPrototype;
NLayoutPrototype layoutPrototype;
NSplitterPrototype splitterPrototype;

Q_DECLARE_METATYPE(QList<QWidget *>)

Q_DECLARE_METATYPE(NPlayer *)
Q_DECLARE_METATYPE(NMainWindow *)
Q_DECLARE_METATYPE(NPlaybackEngineInterface *)
Q_DECLARE_METATYPE(N::PlaybackState)
Q_DECLARE_METATYPE(NSettings *)

template <typename T> QScriptValue qObjectToScriptVlaue(QScriptEngine *engine, T const &obj)
{
    return engine->newQObject(obj);
}

template <typename T> void qObjectFromScriptValue(const QScriptValue &value, T &obj)
{
    obj = qobject_cast<T>(value.toQObject());
}

struct QtMetaObject : private QObject
{
public:
    static const QMetaObject *get() { return &static_cast<QtMetaObject *>(0)->staticQtMetaObject; }
};

template <typename T> QScriptValue enumToScriptValue(QScriptEngine *engine, T const &en)
{
    return engine->newVariant((int)en);
}

template <typename T> void enumFromScriptValue(const QScriptValue &value, T &en)
{
    en = (T)value.toInt32();
}

#ifndef _N_NO_SKINS_
QScriptValue readFile(QScriptContext *context, QScriptEngine *)
{
    if (context->argumentCount() != 1) {
        return QString();
    }

    QString fileName = context->argument(0).toString();

    QFile file(NSkinFileSystem::prefix() + fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    return QLatin1String(file.readAll());
}

QScriptValue maskImage(QScriptContext *context, QScriptEngine *)
{
    if (context->argumentCount() < 2) {
        return QScriptValue();
    }

    QString fileName = context->argument(0).toString();
    QColor color(context->argument(1).toString());

    qsreal opacity = 1.0;
    if (context->argumentCount() == 3) {
        opacity = context->argument(2).toNumber();
    }

    QImage mask(NSkinFileSystem::prefix() + fileName);
    QImage result(mask.size(), QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&result);
    painter.fillRect(result.rect(), color);

    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);

    // hack for solid bitmaps
    color.setAlpha(243);
    painter.fillRect(0, 0, 1, 1, color);

    painter.drawImage(mask.rect(), mask);

    if (opacity != 1.0) {
        QImage copy(result);
        painter.fillRect(copy.rect(), Qt::transparent);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.setOpacity(opacity);
        painter.drawImage(copy.rect(), copy);
    }

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    result.save(&buffer, "PNG");
    NSkinFileSystem::addFile(fileName, byteArray);

    return QScriptValue();
}

QScriptValue addApplicationFont(QScriptContext *context, QScriptEngine *)
{
    if (context->argumentCount() != 1) {
        return -1;
    }

    QString fileName = context->argument(0).toString();

    QFile file(NSkinFileSystem::prefix() + fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return -1;
    }
    QByteArray byteArray = file.readAll();

    return QFontDatabase::addApplicationFontFromData(byteArray);
}
#endif

NScriptEngine::NScriptEngine(NPlayer *player) : QScriptEngine(player)
{
    QScriptValue global = globalObject();

    QScriptValue Qt = newQMetaObject(QtMetaObject::get());
    global.setProperty("Qt", Qt);

    global.setProperty("QT_VERSION", QT_VERSION);

    QString ws;
#if defined Q_OS_MAC
    ws = "mac";
#elif defined Q_OS_WIN
    ws = "win";
#elif defined Q_OS_LINUX
    ws = "x11";
#endif
    global.setProperty("Q_WS", ws);
    global.setProperty("WS_WM_TILING", false);

    QString direction = "right";
#if defined Q_OS_MAC
    direction = "left";
#elif defined Q_OS_LINUX
    QString wmName = NXcb::wmName();
    if (!wmName.isEmpty()) {
        qDebug() << "detected window manager:" << wmName;
        if (wmName == "Marco") {
            QProcess dconf;
            dconf.start("dconf read /org/mate/marco/general/button-layout");
            dconf.waitForStarted();
            dconf.waitForFinished();
            if (dconf.readAll().endsWith(":'\n")) {
                direction = "left";
            }
        } else if (wmName == "Metacity") {
            QProcess gconftool;
            gconftool.start("gconftool-2 --get /apps/metacity/general/button_layout");
            gconftool.waitForStarted();
            gconftool.waitForFinished();
            if (gconftool.readAll().endsWith(":\n")) {
                direction = "left";
            }
        }
        global.setProperty("WS_WM_TILING", wmName == "i3");
    } else {
        qWarning() << "window manager cannot be detected";
    }
#endif

    global.setProperty("WS_WM_BUTTON_DIRECTION", direction);

    qScriptRegisterMetaType(this, NMarginsPrototype::toScriptValue,
                            NMarginsPrototype::fromScriptValue);
    qScriptRegisterMetaType(this, NPointPrototype::toScriptValue, NPointPrototype::fromScriptValue);
    setDefaultPrototype(qMetaTypeId<QWidget *>(), newQObject(&widgetPrototype));
    setDefaultPrototype(qMetaTypeId<QLayout *>(), newQObject(&layoutPrototype));
    setDefaultPrototype(qMetaTypeId<QSplitter *>(), newQObject(&splitterPrototype));
    qScriptRegisterSequenceMetaType<QList<QWidget *>>(this);

    qScriptRegisterMetaType<N::PlaybackState>(this, enumToScriptValue, enumFromScriptValue);
    QScriptValue N = newQMetaObject(&N::staticMetaObject);
    global.setProperty("N", N);

    qScriptRegisterMetaType<NMainWindow *>(this, qObjectToScriptVlaue, qObjectFromScriptValue);
    QScriptValue ui = newObject();
    global.setProperty("Ui", ui);
    NMainWindow *mainWindow = player->mainWindow();
    QList<QWidget *> widgets = mainWindow->findChildren<QWidget *>();
    foreach (QWidget *widget, widgets)
        ui.setProperty(widget->objectName(), newQObject(widget));
    ui.setProperty(mainWindow->objectName(), newQObject(mainWindow));

    qScriptRegisterMetaType<NPlayer *>(this, qObjectToScriptVlaue, qObjectFromScriptValue);
    global.setProperty("Player", newQObject(player));

    qScriptRegisterMetaType<NPlaybackEngineInterface *>(this, qObjectToScriptVlaue,
                                                        qObjectFromScriptValue);
    global.setProperty("PlaybackEngine", newQObject(player->playbackEngine()));

    qScriptRegisterMetaType<NSettings *>(this, qObjectToScriptVlaue, qObjectFromScriptValue);
    global.setProperty("Settings", newQObject(NSettings::instance()));

#ifndef _N_NO_SKINS_
    globalObject().setProperty("readFile", newFunction(readFile));
    globalObject().setProperty("maskImage", newFunction(maskImage));
    globalObject().setProperty("addApplicationFont", newFunction(addApplicationFont));
#endif
}
