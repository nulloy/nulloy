unix:TARGET = playback_vlc
win32:TARGET = PlaybackVLC

include(../common.pri)

CONFIG += link_pkgconfig
PKGCONFIG += libvlc vlc-plugin

HEADERS += *.h
SOURCES += *.cpp

