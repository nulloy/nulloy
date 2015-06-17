# build
unix:SKIN_DEST_DIR = $$SRC_DIR/../skins
win32:SKIN_DEST_DIR = $$SRC_DIR/../Skins
system($$QMAKE_MKDIR $$fixSlashes($$SKIN_DEST_DIR))

unix {
	ZIP_ADD_CMD = zip -j
	ZIP_DEL_CMD = zip -d
}
win32 {
	ZIP_ADD_CMD = 7z a -tzip
	ZIP_DEL_CMD = 7z d -tzip
}

unix:SKINS =  metro silver slim
win32:SKINS = Metro Silver Slim
for(skin, SKINS) {
	_depends = $$SRC_DIR/skins/$$lower($${skin})/*
	_target = $$SKIN_DEST_DIR/$${skin}.nzs
	_commands = $$ZIP_ADD_CMD $$_target $$_depends && $$ZIP_DEL_CMD $$_target design.svg
	eval($${skin}.depends = $$_depends)
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
