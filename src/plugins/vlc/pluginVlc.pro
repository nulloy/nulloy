unix:TARGET = plugin_vlc
win32:TARGET = PluginVLC

include(../plugin.pri)

CONFIG += link_pkgconfig
PKGCONFIG += libvlc vlc-plugin

HEADERS += *.h
SOURCES += *.cpp

