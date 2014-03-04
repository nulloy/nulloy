TEMPLATE = lib
CONFIG += plugin

HEADERS += ../../common.h
SOURCES += ../../common.cpp

unix:DESTDIR = ../../../plugins
win32:DESTDIR = ../../../Plugins

OBJECTS_DIR = $$TMP_DIR
MOC_DIR = $$TMP_DIR

INCLUDEPATH += ../.. ../../interfaces

