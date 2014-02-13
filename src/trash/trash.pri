HEADERS += $$PWD/trash.h
SOURCES += $$PWD/trash.cpp
INCLUDEPATH += $$PWD

win32 {
	SOURCES += $$PWD/trash_win.cpp
	LIBS += -ladvapi32 -lshell32
}
mac {
	OBJECTIVE_SOURCES += $$PWD/trash_mac.mm
	LIBS += -framework Foundation -framework Cocoa
}
unix:!mac {
	SOURCES += $$PWD/trash_x11.cpp
}

