TEMPLATE = subdirs
CONFIG += ordered

isEmpty(N_CONFIG_SUCCESS) {
    unix:error("Please run './configure'")
    win32:error("Please run 'configure.bat'")
}

# qmake -config no-skins
!no-skins {
    widgets.subdir = $$SRC_DIR/widgetCollection
    SUBDIRS += widgets
}

# qmake -config gstreamer
gstreamer {
    gstreamer.subdir = $$SRC_DIR/plugins/pluginGstreamer
    SUBDIRS += gstreamer
}

# qmake -config vlc
vlc {
    vlc.subdir = $$SRC_DIR/plugins/pluginVlc
    SUBDIRS += vlc
}

# qmake -config taglib
taglib {
    taglib.subdir = $$SRC_DIR/plugins/pluginTaglib
    SUBDIRS += taglib
}

# qmake -config plugins-all
plugins-all {
    plugins.subdir = $$SRC_DIR/plugins
    SUBDIRS += plugins
}

SUBDIRS += $$SRC_DIR

tests {
    SUBDIRS += tests
}

