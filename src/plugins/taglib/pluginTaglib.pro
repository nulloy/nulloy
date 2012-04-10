unix:TARGET = plugin_taglib
win32:TARGET = PluginTagLib

include(../plugin.pri)

unix {
	CONFIG += link_pkgconfig
	PKGCONFIG += taglib
}
win32 {
	INCLUDEPATH += $(TAGLIB_DIR) $(TAGLIB_DIR)/include
	LIBS += -L$(TAGLIB_DIR)/taglib/ -ltag
}

INCLUDEPATH += ..

HEADERS += *.h
SOURCES += *.cpp
