TEMPLATE = lib
CONFIG += plugin

HEADERS += ../../rcDir.h
SOURCES += ../../rcDir.cpp

unix:DESTDIR = ../../../plugins
win32:DESTDIR = ../../../Plugins
OBJECTS_DIR = ../.tmp
MOC_DIR = ../.tmp

INCLUDEPATH += ../..
