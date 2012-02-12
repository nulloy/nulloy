INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
HEADERS += $$PWD/tagReader.h

taglib {
	message("taglib!!!!!!")
	SOURCES += tagReader_taglib.cpp
	unix {
		CONFIG += link_pkgconfig
		PKGCONFIG += taglib
	}
	win32 {
		INCLUDEPATH += $(TAGLIB_DIR) $(TAGLIB_DIR)/include
		LIBS += -L$(TAGLIB_DIR)/taglib/ -ltag
	}
} else {
	gstreamer {
		SOURCES += tagReader_gstreamer.cpp
		include(../plugins/gstreamer.pri)
	}
}
