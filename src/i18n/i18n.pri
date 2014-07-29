# build
unix:MV_CMD = mv
win32:MV_CMD = move
isEmpty(LRELEASE):LRELEASE = lrelease

QMS_SRC_DIR = $$SRC_DIR/i18n
qms.depends = $$QMS_SRC_DIR/*.ts

QMS_DEST_DIR = $$SRC_DIR/../i18n
mkdir($$QMS_DEST_DIR)
qms.target = $$QMS_DEST_DIR/*.qm

qms.commands = $$LRELEASE $$qms.depends && \
               $$MV_CMD $$fixSlashes($$QMS_SRC_DIR/*.qm) $$fixSlashes($$QMS_DEST_DIR)

QMAKE_EXTRA_TARGETS += qms
PRE_TARGETDEPS += $$qms.target
system($$qms.commands)

# install
unix {
	translations.files = ../i18n/*
	mac {
		prefix.path = ../$${TARGET}.app
		translations.path = $$prefix.path/Contents/MacOS/i18n
	} else {
		prefix.path = $$PREFIX
		translations.path = $$prefix.path/share/nulloy/i18n
	}
	INSTALLS += translations
}

# to sync *.ts with sources:
# $ lupdate -locations relative src -ts src/i18n/*.ts
