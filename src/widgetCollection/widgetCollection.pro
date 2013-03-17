TEMPLATE = lib
TARGET = widget_collection
DESTDIR = $$PWD

CONFIG += designer plugin static

include(widgetCollection.pri)
INCLUDEPATH += ../trash/

OBJECTS_DIR = $$TMP_DIR
MOC_DIR = $$TMP_DIR
