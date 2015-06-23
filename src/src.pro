TEMPLATE = app
QT += script

mac:no-app-bundle:CONFIG -= app_bundle

TARGET = $$APP_NAME
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
include(../3rdParty/qxt-696423b-patched/qxtglobalshortcut.pri)
include(../3rdParty/qtsingleapplication-8fd81b2/src/qtsingleapplication.pri)
include(../3rdParty/qtiocompressor-2.3.1/src/qtiocompressor.pri)
include(i18n/i18n.pri)
include(trash/trash.pri)
win32:include(ux/w7TaskBar.pri)
mac:include(ux/macDock.pri)

# zlib
!mac {
	CONFIG += link_pkgconfig
	PKGCONFIG += zlib
} else {
	LIBS += -L/usr/lib -lz
}

unix:!mac:PKGCONFIG += x11

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
	HEADERS += widgetCollection/label.h \
	           widgetCollection/playlistWidget.h \
	           widgetCollection/slider.h \
	           widgetCollection/waveformSlider.h \
	           widgetCollection/playlistWidgetItem.h
	SOURCES += widgetCollection/label.cpp \
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

no-update-check:DEFINES += _N_NO_UPDATE_CHECK_

include(version.pri)
DEFINES += _N_VERSION_=\""\\\"$${N_VERSION}\\\""\"
build_pass:CONFIG(static, static|shared) {
	DEFINES += _N_STATIC_BUILD_
} else {
	DEFINES += _N_SHARED_BUILD_
}


# qmake "PREFIX=/usr"
unix:!mac {
	target.path = $$PREFIX/bin

	system(icons/install-icons.sh $$TMP_DIR/icons)
	icons.files = $$TMP_DIR/icons/*
	icons.path = $$PREFIX

	desktop.files = ../$${APP_NAME}.desktop
	desktop.path = $$PREFIX/share/applications

	INSTALLS += target icons desktop
}

