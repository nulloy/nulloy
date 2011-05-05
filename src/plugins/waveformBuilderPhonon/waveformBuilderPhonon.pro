unix:TARGET = waveform_phonon
win32:TARGET = WaveformPhonon

include(../common.pri)

QT += phonon

HEADERS += *.h
SOURCES += *.cpp ../../arguments.cpp ../../waveformBuilderInterface.cpp ../../waveformPeaks.cpp
