TEMPLATE = subdirs
CONFIG += ordered

widgets.subdir = src/widgetCollection
SUBDIRS += widgets

# qmake -config plugins_gstreamer
plugins_gstreamer {
	gstreamer_playback.subdir = src/plugins/playbackEngineGstreamer
	gstreamer_waveform.subdir = src/plugins/waveformBuilderGstreamer
	SUBDIRS += gstreamer_playback gstreamer_waveform
}

# qmake -config plugins_phonon
plugins_phonon {
	phonon_playback.subdir = src/plugins/playbackEnginePhonon
	phonon_waveform.subdir = src/plugins/waveformBuilderPhonon
	SUBDIRS += phonon_playback phonon_waveform
}

# qmake -config plugins_vlc
plugins_vlc {
	vlc_playback.subdir = src/plugins/playbackEngineVlc
	vlc_waveform.subdir = src/plugins/waveformBuilderVlc
	SUBDIRS += vlc_playback vlc_waveform
}

# qmake -config no_plugins
!no_plugins:!plugins_gstreamer:!plugins_phonon:!plugins_vlc {
	plugins.subdir = src/plugins
	SUBDIRS += plugins
}

src.file = src/player.pro
SUBDIRS += src
