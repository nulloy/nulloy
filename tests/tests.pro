TEMPLATE = app
TARGET = tests
QT += testlib
SOURCES += tests.cpp
INCLUDEPATH += $$SRC_DIR

test_projects = $$files(test*.pri)
for(test_pro, test_projects) {
    include($${test_pro})
}
