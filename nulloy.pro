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
	phonon_playback.subdir = src/plugins/playbackEnginePhonon
	phonon_waveform.subdir = src/plugins/waveformBuilderPhonon
	SUBDIRS += phonon_playback phonon_waveform
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
