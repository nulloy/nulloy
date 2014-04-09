unix:TARGET = plugin_phonon
win32:TARGET = PluginPhonon

include(../plugin.pri)

QT += phonon

HEADERS += *.h
SOURCES += *.cpp

