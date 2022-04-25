TEMPLATE = lib
TARGET = widget_collection
DESTDIR = $$PWD

QT += designer gui
CONFIG += plugin static

include(widgetCollection.pri)
INCLUDEPATH += $$SRC_DIR/platform/
