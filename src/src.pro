TEMPLATE = app
TARGET = $$APP_NAME
DESTDIR = ..

SRC_DIR = $$PWD
include($$SRC_DIR/src.pri)
DEFINES += N_LIBDIR=\""\\\"$${LIBDIR}\\\""\"
SOURCES += $$SRC_DIR/main.cpp
