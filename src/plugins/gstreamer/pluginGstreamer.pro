unix:TARGET = plugin_gstreamer
win32:TARGET = PluginGStreamer

include(../plugin.pri)

CONFIG += link_pkgconfig
PKGCONFIG += gstreamer-1.0

HEADERS += $$files(*.h)
SOURCES += $$files(*.cpp)

gstreamer-tagreader {
	PKGCONFIG += gstreamer-pbutils-1.0
	DEFINES += _N_GSTREAMER_TAGREADER_PLUGIN_
} else {
	HEADERS -= tagReaderGstreamer.h
	SOURCES -= tagReaderGstreamer.cpp
}

