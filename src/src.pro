TEMPLATE = app
TARGET = $$APP_NAME
DESTDIR = $$PROJECT_DIR

include($$SRC_DIR/src.pri)
DEFINES += N_LIBDIR=\""\\\"$${LIBDIR}\\\""\"
SOURCES += $$SRC_DIR/main.cpp
