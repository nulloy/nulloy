unix:TARGET = waveform_vlc
win32:TARGET = WaveformVLC

include(../common.pri)

CONFIG += link_pkgconfig
PKGCONFIG += libvlc vlc-plugin

HEADERS += *.h
SOURCES += *.cpp ../../waveformBuilderInterface.cpp ../../waveformPeaks.cpp
