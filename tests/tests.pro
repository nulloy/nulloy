TARGET = test
TEMPLATE = app
CONFIG += qtestlib
DESTDIR = ..

SRC_DIR = ../src
include($$SRC_DIR/src.pri)

SOURCES += *.cpp
HEADERS += *.h
INCLUDEPATH += .

LIST = 01 02 03 04 05 06 07 08 09 10
for(index, LIST){
    _target = $${index}.ogg
    _commands = $$QMAKE_COPY silence.ogg $$_target
    eval($${index}.target = $$_target)
    eval($${index}.commands = $$_commands)

    QMAKE_EXTRA_TARGETS += $${index}
    PRE_TARGETDEPS += $$_target
    system($$_commands)
}
