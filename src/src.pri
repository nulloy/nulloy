QT += script gui core-private

INCLUDEPATH += $$SRC_DIR $$SRC_DIR/interfaces

HEADERS += $$SRC_DIR/*.h
SOURCES += $$files($$SRC_DIR/*.cpp)
SOURCES -= $$SRC_DIR/main.cpp

FORMS += $$SRC_DIR/*.ui

CONFIG += app_bundle

isEmpty(TMP_DIR):TMP_DIR = ../.tmp
OBJECTS_DIR = $$TMP_DIR
MOC_DIR     = $$TMP_DIR
RCC_DIR     = $$TMP_DIR
UI_DIR      = $$TMP_DIR

include($$SRC_DIR/func.pri)
include(../3rdParty/qxt-696423b-patched/qxtglobalshortcut.pri)
include(../3rdParty/qtsingleapplication-8fd81b2/src/qtsingleapplication.pri)
include(../3rdParty/qtiocompressor-2.3.1/src/qtiocompressor.pri)
include($$SRC_DIR/i18n/i18n.pri)
include($$SRC_DIR/platform/trash.pri)
win32:include($$SRC_DIR/platform/w7TaskBar.pri)
mac:include($$SRC_DIR/platform/macDock.pri)
unix:!mac:include($$SRC_DIR/platform/xcb.pri)

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
    include($$SRC_DIR/skins/skins.pri)
    QT += uitools
    INCLUDEPATH += $$SRC_DIR/widgetCollection
    LIBS += -L$$SRC_DIR/widgetCollection -lwidget_collection
    PRE_TARGETDEPS += $$SRC_DIR/widgetCollection/libwidget_collection.a
    RESOURCES += $$SRC_DIR/native-skin-embedded.qrc
} else {
    DEFINES += _N_NO_SKINS_

    HEADERS -= $$SRC_DIR/skinFileSystem.h   $$SRC_DIR/skinLoader.h
    SOURCES -= $$SRC_DIR/skinFileSystem.cpp $$SRC_DIR/skinLoader.cpp

    HEADERS += $$files($$SRC_DIR/widgetCollection/*.h)
    SOURCES += $$files($$SRC_DIR/widgetCollection/*.cpp)
    HEADERS -= $$SRC_DIR/widgetCollection/widgetCollection.h
    SOURCES -= $$SRC_DIR/widgetCollection/widgetCollection.cpp

    DEPENDPATH +=  $$SRC_DIR/widgetCollection
    INCLUDEPATH += $$SRC_DIR/widgetCollection

    RESOURCES += $$SRC_DIR/no-skins.qrc
    FORMS += $$SRC_DIR/skins/native/form.ui
}

RESOURCES += $$SRC_DIR/icons/icons.qrc
win32:RC_FILE = $$SRC_DIR/icons/icon.rc
mac:ICON = $$SRC_DIR/icons/icon.icns

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

    SIZES = 16 22 24 32 48 64 96 128
    for(size, SIZES) {
        eval(icon$${size}.extra = mkdir $$TMP_DIR/icon-$${size}; cp $$SRC_DIR/icons/icon-$${size}.png $$TMP_DIR/icon-$${size}/$${APP_NAME}.png)
        eval(icon$${size}.files = $$TMP_DIR/icon-$${size}/*)
        eval(icon$${size}.path = $$PREFIX/share/icons/hicolor/$${size}x$${size}/apps)
        eval(INSTALLS += icon$${size})
    }

    desktop.files = ../$${APP_NAME}.desktop
    desktop.path = $$PREFIX/share/applications

    INSTALLS += target desktop
}

