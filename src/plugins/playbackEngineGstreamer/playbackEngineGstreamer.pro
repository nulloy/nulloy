unix:TARGET = playback_gstreamer
win32:TARGET = PlaybackGStreamer

include(../common.pri)
include(../gstreamer.pri)

HEADERS += *.h
SOURCES += *.cpp
