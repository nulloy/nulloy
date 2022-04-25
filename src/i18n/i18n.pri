# build
isEmpty(LRELEASE):LRELEASE = lrelease

QMS_SRC_DIR = $$SRC_DIR/i18n
qms.depends = $$QMS_SRC_DIR/*.ts

unix:QMS_DEST_DIR = $$PROJECT_DIR/i18n
mac:QMS_DEST_DIR = $$PROJECT_DIR/$${APP_NAME}.app/Contents/MacOS/i18n
win32:QMS_DEST_DIR = $$PROJECT_DIR/i18n

system($$QMAKE_MKDIR $$fixSlashes($$QMS_DEST_DIR))
qms.target = $$QMS_DEST_DIR/*.qm

qms.commands = $$LRELEASE $$qms.depends && \
               $$QMAKE_MOVE $$fixSlashes($$QMS_SRC_DIR/*.qm) $$fixSlashes($$QMS_DEST_DIR)

QMAKE_EXTRA_TARGETS += qms
PRE_TARGETDEPS += $$qms.target
system($$qms.commands)


# install
unix:!mac {
    translations.files = ../i18n/*
    translations.path = $$PREFIX/share/$$APP_NAME/i18n
    INSTALLS += translations
}

# to sync *.ts with sources:
# $ lupdate -locations relative src -ts src/i18n/*.ts
