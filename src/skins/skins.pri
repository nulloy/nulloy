# build
unix:SKIN_DEST_DIR = $$SRC_DIR/../skins
win32:SKIN_DEST_DIR = $$SRC_DIR/../Skins
system($$QMAKE_MKDIR $$fixSlashes($$SKIN_DEST_DIR))

unix:SKINS =  metro silver slim
win32:SKINS = Metro Silver Slim
for(skin, SKINS) {
    _depends = $$SRC_DIR/skins/$$lower($${skin})
    _target = $$SKIN_DEST_DIR/$${skin}.nzs
    _commands = zip -j -X -x\*design.svg $$_target $$_depends/*
    win32!unix_mingw:_commands = 7z a -tzip -w$$_depends -x\!design.svg $$_target $$_depends/*
    eval($${skin}.depends = $$_depends/*)
    eval($${skin}.target = $$_target)
    eval($${skin}.commands = $$_commands)

    QMAKE_EXTRA_TARGETS += $${skin}
    PRE_TARGETDEPS += $$_target
    system($$_commands)
}

# install
unix {
    skins.files = ../skins/*
    mac {
        skins.path = ../$${APP_NAME}.app/Contents/MacOS/skins
    } else {
        skins.path = $$PREFIX/share/$$APP_NAME/skins
    }
    INSTALLS += skins
}
