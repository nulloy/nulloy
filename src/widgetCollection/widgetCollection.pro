TEMPLATE = lib
TARGET = widget_collection
DESTDIR = $$PWD

QT += designer gui
CONFIG += plugin static

include(widgetCollection.pri)
INCLUDEPATH += ../platform/

OBJECTS_DIR = $$TMP_DIR
MOC_DIR = $$TMP_DIR

