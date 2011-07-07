TEMPLATE = app
QT += script
CONFIG += uitools

unix:TARGET = nulloy
win32:TARGET = Nulloy
DESTDIR = ..

DEPENDPATH += . ux/
INCLUDEPATH += . ux/

HEADERS += *.h ux/*.h
SOURCES += *.cpp ux/*.cpp

FORMS += *.ui
RESOURCES += *.qrc

OBJECTS_DIR = .tmp
MOC_DIR = .tmp

INCLUDEPATH += widgetCollection
LIBS += -LwidgetCollection -lwidget_collection
PRE_TARGETDEPS += widgetCollection/libwidget_collection.a

win32 {
	RC_FILE = icon.rc
	DEFINES += _N_TIME_STAMP_=__TIMESTAMP__
}
unix {
	DEFINES += _N_TIME_STAMP_=\""\\\"`date +\\\"%a %b %d %T %Y\\\"`\\\""\"
}

build_pass:CONFIG(static, static|shared) {
	DEFINES += _N_STATIC_BUILD_
} else {
	DEFINES += _N_SHARED_BUILD_
}

include(version.pri)
DEFINES += _N_VERSION_=\""\\\"$${VERSION}\\\""\"

include(../3rdParty/qxt-0.6.1~reduced/src/gui/qxtglobalshortcut.pri)
include(../3rdParty/qtsingleapplication-2.6.1/src/qtsingleapplication.pri)
include(../3rdParty/qtiocompressor-2.3.1/src/qtiocompressor.pri)

# qmake -config embed-gstreamer
embed-gstreamer {
	include(plugins/gstreamer.pri)
	DEFINES += _N_GSTREAMER_PLUGINS_BUILTIN_
	HEADERS += plugins/waveformBuilderGstreamer/*.h plugins/playbackEngineGstreamer/*.h
	SOURCES += plugins/waveformBuilderGstreamer/*.cpp plugins/playbackEngineGstreamer/*.cpp
	INCLUDEPATH += plugins/waveformBuilderGstreamer plugins/playbackEngineGstreamer
}

unix {
	silver_skin.target = ../skins/silver.nzs
	silver_skin.depends = skins/silver/*
	silver_skin.commands =	mkdir ../skins && \
							cd .tmp && \
							cp -r ../skins/silver . && \
							cd silver && \
							rm design.svg && \
							zip ../../../skins/silver.nzs *
	QMAKE_EXTRA_TARGETS += silver_skin
	POST_TARGETDEPS += $$silver_skin.target
}

# qmake "PREFIX=/usr"
unix {
	prefix.path = $$PREFIX
	target.path = $$prefix.path/bin

	plugins.files = ../plugins/*
	plugins.path = $$prefix.path/lib/nulloy/plugins

	skins.files = ../skins/*
	skins.path = $$prefix.path/share/nulloy/skins

	icon.files = icon.png
	icon.path = $$prefix.path/share/nulloy

	icon_post.extra = cd "$(INSTALL_ROOT)"$$prefix.path/share/icons && ln -s ../nulloy/icon.png nulloy.png
	icon_post.path = $$prefix.path/share/icons/

	desktop.files = ../nulloy.desktop
	desktop.path = $$prefix.path/share/applications

	INSTALLS += target plugins skins icon icon_post desktop
}

# vim: set ts=4 sw=4: #
