TEMPLATE = lib
CONFIG += plugin

HEADERS += ../../core.h
SOURCES += ../../core.cpp

unix:DESTDIR = ../../../plugins
win32:DESTDIR = ../../../Plugins
OBJECTS_DIR = ../.tmp
MOC_DIR = ../.tmp

INCLUDEPATH += ../..
