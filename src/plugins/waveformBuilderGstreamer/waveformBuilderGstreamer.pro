unix:TARGET = waveform_gstreamer
win32:TARGET = WaveformGStreamer

include(../common.pri)
include(../gstreamer.pri)

INCLUDEPATH += ..

HEADERS += *.h
SOURCES += *.cpp ../abstractWaveformBuilder.cpp ../../waveformPeaks.cpp
