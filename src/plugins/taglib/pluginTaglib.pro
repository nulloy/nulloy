unix:TARGET = plugin_taglib
win32:TARGET = PluginTagLib

!mac:QMAKE_CXXFLAGS += -std=c++0x

include(../plugin.pri)

CONFIG += link_pkgconfig
PKGCONFIG += taglib

HEADERS += *.h
SOURCES += *.cpp

