# build
unix:SKIN_DEST_DIR = $$SRC_DIR/../skins
win32:SKIN_DEST_DIR = $$SRC_DIR/../Skins
mkdir($$SKIN_DEST_DIR)

metro_skin.depends = $$SRC_DIR/skins/metro/*
unix:metro_skin.target = $$SKIN_DEST_DIR/metro.nzs
win32:metro_skin.target = $$SKIN_DEST_DIR/Metro.nzs

silver_skin.depends = $$SRC_DIR/skins/silver/*
unix:silver_skin.target = $$SKIN_DEST_DIR/silver.nzs
win32:silver_skin.target = $$SKIN_DEST_DIR/Silver.nzs

unix {
	ZIP_ADD_CMD = zip -j
	ZIP_DEL_CMD = zip -d
}
win32 {
	ZIP_ADD_CMD = 7z a -tzip
	ZIP_DEL_CMD = 7z d -tzip
}
metro_skin.commands =  $$ZIP_ADD_CMD $$metro_skin.target $$metro_skin.depends && \
                       $$ZIP_DEL_CMD $$metro_skin.target design.svg
silver_skin.commands = $$ZIP_ADD_CMD $$silver_skin.target $$silver_skin.depends && \
                       $$ZIP_DEL_CMD $$silver_skin.target design.svg

QMAKE_EXTRA_TARGETS += silver_skin metro_skin
PRE_TARGETDEPS += $$silver_skin.target $$metro_skin.target
system($$silver_skin.commands)
system($$metro_skin.commands)


# install
unix {
	skins.files = ../skins/*
	mac {
		prefix.path = ../$${TARGET}.app
		skins.path = $$prefix.path/Contents/MacOS/skins
	} else {
		prefix.path = $$PREFIX
		skins.path = $$prefix.path/share/nulloy/skins
	}
	INSTALLS += skins
}
