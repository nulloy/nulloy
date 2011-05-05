unix:TARGET = waveform_gstreamer
win32:TARGET = WaveformGStreamer

include(../common.pri)
include(../gstreamer.pri)

HEADERS += *.h
SOURCES += *.cpp ../../arguments.cpp ../../waveformBuilderInterface.cpp ../../waveformPeaks.cpp
