TEMPLATE = subdirs
CONFIG += ordered

unix:isEmpty(N_CONFIG_SUCCESS) {
    error(Please run configure.)
}

# qmake -config no-skins
!no-skins {
	widgets.subdir = src/widgetCollection
	SUBDIRS += widgets
}

# qmake -config gstreamer
gstreamer {
	gstreamer.file = src/plugins/gstreamer/pluginGstreamer.pro
	SUBDIRS += gstreamer
}

# qmake -config phonon
phonon {
	phonon.file = src/plugins/phonon/pluginPhonon.pro
	SUBDIRS += phonon
}

# qmake -config vlc
vlc {
	vlc.file = src/plugins/vlc/pluginVlc.pro
	SUBDIRS += vlc
}

# qmake -config plugins-all
plugins-all {
	plugins.subdir = src/plugins
	SUBDIRS += plugins
}

src.file = src/player.pro
SUBDIRS += src

# vim: set ts=4 sw=4: #
