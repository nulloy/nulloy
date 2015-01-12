unix:TARGET = plugin_gstreamer
win32:TARGET = PluginGStreamer

include(../plugin.pri)

unix {
	CONFIG += link_pkgconfig
	PKGCONFIG += gstreamer-1.0
}

win32 {
	INCLUDEPATH +=  $(GSTREAMER_1_0_ROOT_X86)/include \
	                $(GSTREAMER_1_0_ROOT_X86)/include/gstreamer-1.0 \
	                $(GSTREAMER_1_0_ROOT_X86)/include/glib-2.0 \
	                $(GSTREAMER_1_0_ROOT_X86)/lib/glib-2.0/include \
	                $(GSTREAMER_1_0_ROOT_X86)/include/libxml2

	LIBS +=         -L$(GSTREAMER_1_0_ROOT_X86)/lib \
	                -lgstreamer-1.0 \
	                -lglib-2.0 \
	                -liconv \
	                -lxml2 \
	                -lgobject-2.0

	DEFINES += LIBXML_STATIC
}

HEADERS += $$files(*.h)
SOURCES += $$files(*.cpp)

gstreamer-tagreader {
	unix:PKGCONFIG += gstreamer-pbutils-1.0
	DEFINES += _N_GSTREAMER_TAGREADER_PLUGIN_
} else {
	HEADERS -= tagReaderGstreamer.h
	SOURCES -= tagReaderGstreamer.cpp
}

