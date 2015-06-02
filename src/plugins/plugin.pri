TEMPLATE = lib
CONFIG += plugin

HEADERS += ../../common.h
SOURCES += ../../common.cpp ../abstractWaveformBuilder.cpp ../../waveformPeaks.cpp

unix:DESTDIR = ../../../plugins
win32:DESTDIR = ../../../Plugins

OBJECTS_DIR = $$TMP_DIR
MOC_DIR = $$TMP_DIR

INCLUDEPATH += .. ../.. ../../interfaces
DEPENDPATH += ../..

message($${_PRO_FILE_PWD_})
unix {
	mac {
		target.path = ../../../$${APP_NAME}.app/Contents/MacOS/plugins
	} else {
		target.path = $$PREFIX/lib/$$APP_NAME/plugins
	}
	INSTALLS += target
}
