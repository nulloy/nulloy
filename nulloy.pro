TEMPLATE = subdirs
CONFIG += ordered

# qmake -config no-skins
!no-skins {
	widgets.subdir = src/widgetCollection
	SUBDIRS += widgets
}

# qmake -config gstreamer
gstreamer {
	gstreamer_playback.subdir = src/plugins/playbackEngineGstreamer
	gstreamer_waveform.subdir = src/plugins/waveformBuilderGstreamer
	SUBDIRS += gstreamer_playback gstreamer_waveform
}

# qmake -config phonon
phonon {
	phonon_playback.subdir = src/plugins/playbackEnginePhonon
	phonon_waveform.subdir = src/plugins/waveformBuilderPhonon
	SUBDIRS += phonon_playback phonon_waveform
}

# qmake -config vlc
vlc {
	vlc_playback.subdir = src/plugins/playbackEngineVlc
	vlc_waveform.subdir = src/plugins/waveformBuilderVlc
	SUBDIRS += vlc_playback vlc_waveform
}

# qmake -config plugins-all
plugins-all {
	plugins.subdir = src/plugins
	SUBDIRS += plugins
}

src.file = src/player.pro
SUBDIRS += src

# vim: set ts=4 sw=4: #
