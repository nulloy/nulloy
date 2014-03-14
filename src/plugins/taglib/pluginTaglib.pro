unix:TARGET = plugin_taglib
win32:TARGET = PluginTagLib

QMAKE_CXXFLAGS += -std=c++0x

include(../plugin.pri)

unix {
	CONFIG += link_pkgconfig
	PKGCONFIG += taglib
}
win32 {
	INCLUDEPATH += $(TAGLIB_DIR)/include
	LIBS += -L$(TAGLIB_DIR)/lib -ltag
	DEFINES += TAGLIB_STATIC
}

INCLUDEPATH += ..

HEADERS += *.h
SOURCES += *.cpp

