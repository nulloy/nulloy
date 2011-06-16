TEMPLATE = lib
TARGET = widget_collection
DESTDIR = $$PWD

CONFIG += designer plugin static

include(widgetCollection.pri)

OBJECTS_DIR = ../.tmp
MOC_DIR = ../.tmp
