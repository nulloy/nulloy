DEFINES += QXT_STATIC

win32:LIBS      += -luser32
macx:LIBS       += -framework Carbon

INCLUDEPATH += $$PWD $$PWD/../core
DEPENDPATH += $$PWD $$PWD/../core

HEADERS  += qxtglobalshortcut.h qxtglobalshortcut_p.h
SOURCES  += qxtglobalshortcut.cpp

unix:!macx {
	SOURCES += qxtglobalshortcut_x11.cpp

	CONFIG += link_pkgconfig
	PKGCONFIG += x11
}
macx {
	SOURCES += qxtglobalshortcut_mac.cpp
}
win32 {
	SOURCES += qxtglobalshortcut_win.cpp
}
