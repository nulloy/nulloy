TEMPLATE = app
QT += script

mac:no-app-bundle:CONFIG -= app_bundle

unix:TARGET = nulloy
win32:TARGET = Nulloy
DESTDIR = ..

INCLUDEPATH += . interfaces/

HEADERS += *.h
SOURCES += *.cpp

FORMS += *.ui

isEmpty(TMP_DIR):TMP_DIR = $$PWD/../.tmp
OBJECTS_DIR	= $$TMP_DIR
MOC_DIR     = $$TMP_DIR
RCC_DIR     = $$TMP_DIR
UI_DIR      = $$TMP_DIR

SRC_DIR = $$PWD

include(func.pri)
include(../3rdParty/qxt-0.6.1~reduced/src/gui/qxtglobalshortcut.pri)
include(../3rdParty/qtsingleapplication-8fd81b2/src/qtsingleapplication.pri)
include(../3rdParty/qtiocompressor-2.3.1/src/qtiocompressor.pri)
include(i18n/i18n.pri)
include(trash/trash.pri)
win32:include(ux/w7TaskBar.pri)

# zlib
unix {
	CONFIG += link_pkgconfig
	# zlib
	PKGCONFIG += zlib
	# X11
	!mac:PKGCONFIG += x11
}
win32 {
	LIBS += -L$(ZLIB_DIR)/lib -lzdll
}


# qmake -config no-skins
!no-skins {
	include(skins/skins.pri)
	CONFIG += uitools
	INCLUDEPATH += widgetCollection
	LIBS += -LwidgetCollection -lwidget_collection
	PRE_TARGETDEPS += widgetCollection/libwidget_collection.a
	RESOURCES += native-skin-embedded.qrc
} else {
	DEFINES += _N_NO_SKINS_

	HEADERS -= skinFileSystem.h   skinLoader.h
	SOURCES -= skinFileSystem.cpp skinLoader.cpp
	HEADERS += widgetCollection/dropArea.h \
	           widgetCollection/label.h \
	           widgetCollection/playlistWidget.h \
	           widgetCollection/slider.h \
	           widgetCollection/waveformSlider.h \
	           widgetCollection/playlistWidgetItem.h
	SOURCES += widgetCollection/dropArea.cpp \
	           widgetCollection/label.cpp \
	           widgetCollection/playlistWidget.cpp \
	           widgetCollection/slider.cpp \
	           widgetCollection/waveformSlider.cpp \
	           widgetCollection/playlistWidgetItem.cpp

	DEPENDPATH += widgetCollection/
	INCLUDEPATH += widgetCollection/

	RESOURCES += no-skins.qrc
	FORMS += skins/native/form.ui
}

RESOURCES += icons/icons.qrc
win32:RC_FILE = icons/icon.rc
mac:ICON = icons/icon.icns


include(version.pri)
DEFINES += _N_VERSION_=\""\\\"$${N_VERSION}\\\""\"
win32:DEFINES += _N_TIME_STAMP_=__TIMESTAMP__
unix:DEFINES += _N_TIME_STAMP_=\""\\\"`date +\\\"%a %b %d %T %Y\\\"`\\\""\"
build_pass:CONFIG(static, static|shared) {
	DEFINES += _N_STATIC_BUILD_
} else {
	DEFINES += _N_SHARED_BUILD_
}


# qmake -config no-plugins
!no-plugins {
	HEADERS -= pluginLoader.h
	SOURCES -= pluginLoader.cpp
} else {
	DEFINES += _N_NO_PLUGINS_
}


# qmake -config embed-gstreamer
embed-gstreamer|no-plugins {
	include(plugins/gstreamer.pri)
	DEFINES += _N_GSTREAMER_PLUGINS_BUILTIN_
	HEADERS += plugins/waveformBuilderGstreamer/*.h plugins/playbackEngineGstreamer/*.h
	SOURCES += plugins/waveformBuilderGstreamer/*.cpp plugins/playbackEngineGstreamer/*.cpp
	INCLUDEPATH += plugins/waveformBuilderGstreamer plugins/playbackEngineGstreamer
}


# qmake "PREFIX=/usr"
unix:!mac {
	prefix.path = $$PREFIX
	target.path = $$prefix.path/bin

	system(icons/install-icons.sh $$TMP_DIR/icons)
	icons.files = $$TMP_DIR/icons/*
	icons.path = $$prefix.path

	desktop.files = ../nulloy.desktop
	desktop.path = $$prefix.path/share/applications

	INSTALLS += target icons desktop

	!no-plugins {
		plugins.files = ../plugins/*
		plugins.path = $$prefix.path/lib/nulloy/plugins
		INSTALLS += plugins
	}
}
mac {
	prefix.path = ../$${TARGET}.app

	!no-plugins {
		plugins.files = ../plugins/*
		plugins.path = $$prefix.path/Contents/MacOS/plugins
		INSTALLS += plugins
	}
}
