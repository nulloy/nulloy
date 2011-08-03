unix:TARGET = waveform_phonon
win32:TARGET = WaveformPhonon

include(../common.pri)

QT += phonon

INCLUDEPATH += ..

HEADERS += *.h
SOURCES += *.cpp ../abstractWaveformBuilder.cpp ../../waveformPeaks.cpp
