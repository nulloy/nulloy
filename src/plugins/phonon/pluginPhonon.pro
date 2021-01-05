unix:TARGET = plugin_phonon
win32:TARGET = PluginPhonon

include(../plugin.pri)

QT += phonon4qt5

HEADERS += *.h
SOURCES += *.cpp

