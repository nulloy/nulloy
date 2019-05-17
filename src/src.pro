TEMPLATE = app
TARGET = $$APP_NAME
DESTDIR = ..

SRC_DIR = $$PWD
include($$SRC_DIR/src.pri)
DEFINES += N_LIBDIR=\""\\\"$${N_LIBDIR}\\\""\"
SOURCES += $$SRC_DIR/main.cpp
