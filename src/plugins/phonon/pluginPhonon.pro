unix:TARGET = plugin_phonon
win32:TARGET = PluginPhonon

include(../plugin.pri)

QT += phonon

INCLUDEPATH += ..

HEADERS += *.h
SOURCES += *.cpp ../abstractWaveformBuilder.cpp ../../waveformPeaks.cpp

