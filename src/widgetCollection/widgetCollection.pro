TEMPLATE = lib
TARGET = widget_collection
DESTDIR = $$PWD

CONFIG += designer plugin static

include(widgetCollection.pri)
include(../trash/trash.pri)

OBJECTS_DIR = $$TMP_DIR
MOC_DIR = $$TMP_DIR

