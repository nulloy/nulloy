TEMPLATE = app
TARGET = $$APP_NAME
DESTDIR = ..

SRC_DIR = $$PWD
include($$SRC_DIR/src.pri)
SOURCES += $$SRC_DIR/main.cpp
