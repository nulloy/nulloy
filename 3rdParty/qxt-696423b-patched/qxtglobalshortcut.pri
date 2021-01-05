DEFINES += QXT_STATIC
QT += gui-private

win32:LIBS += -luser32
macx:LIBS += -framework Carbon

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += $$PWD/qxtglobalshortcut.h $$PWD/qxtglobalshortcut_p.h
SOURCES += $$PWD/qxtglobalshortcut.cpp

unix:!macx {
    SOURCES += $$PWD/qxtglobalshortcut_x11.cpp

    CONFIG += link_pkgconfig
    PKGCONFIG += x11
}
macx {
    SOURCES += $$PWD/qxtglobalshortcut_mac.cpp
}
win32 {
    SOURCES += $$PWD/qxtglobalshortcut_win.cpp
}
