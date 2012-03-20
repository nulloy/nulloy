unix:TARGET = plugin_phonon
win32:TARGET = PluginPhonon

include(../common.pri)

QT += phonon

INCLUDEPATH += ..

HEADERS += *.h
SOURCES += *.cpp ../abstractWaveformBuilder.cpp ../../waveformPeaks.cpp

