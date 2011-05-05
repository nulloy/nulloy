infile(config.pri, SOLUTIONS_LIBRARY, yes): CONFIG += qtiocompressor-uselib
TEMPLATE += fakelib
QTIOCOMPRESSOR_LIBNAME = $$qtLibraryTarget(QtSolutions_IOCompressor-2.3)
TEMPLATE -= fakelib
QTIOCOMPRESSOR_LIBDIR = $$PWD/lib
unix:qtiocompressor-uselib:!qtiocompressor-buildlib:QMAKE_RPATHDIR += $$QTIOCOMPRESSOR_LIBDIR
