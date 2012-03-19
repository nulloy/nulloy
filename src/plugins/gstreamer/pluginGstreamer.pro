unix:TARGET = plugin_gstreamer
win32:TARGET = PluginGStreamer

include(../common.pri)
include(../gstreamer.pri)

INCLUDEPATH += ..

HEADERS += *.h
SOURCES += *.cpp ../abstractWaveformBuilder.cpp ../../waveformPeaks.cpp

