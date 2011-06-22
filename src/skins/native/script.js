/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2011 Sergey Vlasov <sergey@vlasov.me>
**
**  This program can be distributed under the terms of the GNU
**  General Public License version 3.0 as published by the Free
**  Software Foundation and appearing in the file LICENSE.GPL3
**  included in the packaging of this file.  Please review the
**  following information to ensure the GNU General Public License
**  version 3.0 requirements will be met:
**
**  http://www.gnu.org/licenses/gpl-3.0.html
**
*********************************************************************/

function Program(window, playbackEngine)
{
	this.window = window;
	this.playbackEngine = playbackEngine;
	this.playlistWidget = window.findChild("playlistWidget");
	this.dropArea = window.findChild("dropArea");
	this.playButton = window.findChild("playButton");
	this.stopButton = window.findChild("stopButton");
	this.prevButton = window.findChild("prevButton");
	this.nextButton = window.findChild("nextButton");
	this.volumeSlider = window.findChild("volumeSlider");
	this.waveformSlider = window.findChild("waveformSlider");

	this.playButton.clicked.connect(this.playlistWidget.activateCurrent);
	this.stopButton.clicked.connect(this.playbackEngine.stop);
	this.prevButton.clicked.connect(this.playlistWidget.activatePrev);
	this.nextButton.clicked.connect(this.playlistWidget.activateNext);

	this.window.styleSheet = "";
	this.playButton.setStandardIcon("media-playback-start", ":/trolltech/styles/commonstyle/images/media-play-16.png");
	this.stopButton.setStandardIcon("media-playback-stop", ":/trolltech/styles/commonstyle/images/media-stop-16.png");
	this.prevButton.setStandardIcon("media-skip-backward", ":/trolltech/styles/commonstyle/images/media-skip-backward-16.png");
	this.nextButton.setStandardIcon("media-skip-forward", ":/trolltech/styles/commonstyle/images/media-skip-forward-16.png");

	this.volumeSlider.minimum = 0;
	this.volumeSlider.maximum = 100;

	this.waveformSlider.minimum = 0;
	this.waveformSlider.maximum = 10000;

	this.playbackEngine["playStateChanged(bool)"].connect(this, "updatePlayButtonIcon");
	this.playbackEngine["mediaChanged(const QString &)"].connect(this.waveformSlider["drawFile(const QString &)"]);
	this.playbackEngine["finished()"].connect(this.playlistWidget.activateNext);
	this.playbackEngine["failed()"].connect(this, "on_failed");
	this.playlistWidget["mediaSet(const QString &)"].connect(this.playbackEngine["setMedia(const QString &)"]);
	this.playlistWidget["currentActivated()"].connect(this.playbackEngine.play);

	this.volumeSlider["sliderMoved(int)"].connect(this, "on_volumeSlider_sliderMoved");
	this.playbackEngine["volumeChanged(qreal)"].connect(this, "volumeSlider_setValue");

	this.waveformSlider["sliderMoved(int)"].connect(this, "on_waveformSlider_sliderMoved");
	this.playbackEngine["positionChanged(qreal)"].connect(this, "waveformSlide_setValue");

	this.dropArea["filesDropped(const QStringList &)"].connect(this.playlistWidget["activateMediaList(const QStringList &)"]);
	this.window.windowFlags = Qt.Window;

	//print("script loaded");
}

Program.prototype.updatePlayButtonIcon = function(playState)
{
	if (playState) {
		this.playButton.setStandardIcon("media-playback-pause", ":/trolltech/styles/commonstyle/images/media-pause-16.png");
	} else {
		this.playButton.setStandardIcon("media-playback-start", ":/trolltech/styles/commonstyle/images/media-play-16.png");
	}
}

Program.prototype.on_failed = function()
{
	this.playlistWidget.setCurrentFailed();
	this.playlistWidget.activateNext();
}

Program.prototype.on_volumeSlider_sliderMoved = function(value)
{
	this.playbackEngine.setVolume(value / this.volumeSlider.maximum);
}

Program.prototype.volumeSlider_setValue = function(value)
{
	this.volumeSlider.value = Math.round(value * this.volumeSlider.maximum);
}

Program.prototype.on_waveformSlider_sliderMoved = function(value)
{
	this.playbackEngine.setPosition(value / this.waveformSlider.maximum);
}

Program.prototype.waveformSlide_setValue = function(value)
{
	this.waveformSlider.value = Math.round(value * this.waveformSlider.maximum);
}

/* vim: set ts=4 sw=4: */
