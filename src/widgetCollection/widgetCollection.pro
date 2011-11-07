TEMPLATE = lib
TARGET = widget_collection
DESTDIR = $$PWD

CONFIG += designer plugin static

include(widgetCollection.pri)
INCLUDEPATH += ../trash/

OBJECTS_DIR = ../.tmp
MOC_DIR = ../.tmp
