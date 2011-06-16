/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2011 Sergey Vlasov <sergey@vlasov.me>
**
**  This skin package including all images, cascading style sheets,
**  UI forms, and JavaScript files are released under
**  Attribution-ShareAlike Unported License 3.0 (CC-BY-SA 3.0).
**  Please review the following information to ensure the CC-BY-SA 3.0
**  License requirements will be met:
**
**  http://creativecommons.org/licenses/by-sa/3.0/
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
	this.playlistToggleButton = window.findChild("playlistToggleButton");
	this.shadowWidget = window.findChild("shadowWidget");
	this.closeButton = window.findChild("closeButton");
	this.minimizeButton = window.findChild("minimizeButton");
	this.titleLabel = window.findChild("titleLabel");

	this.playButton.clicked.connect(this.playbackEngine.play);
	this.stopButton.clicked.connect(this.playbackEngine.stop);
	this.prevButton.clicked.connect(this.playlistWidget.activatePrev);
	this.nextButton.clicked.connect(this.playlistWidget.activateNext);

	this.volumeSlider.minimum = 0;
	this.volumeSlider.maximum = 100;

	this.waveformSlider.minimum = 0;
	this.waveformSlider.maximum = 10000;

	this.playbackEngine["playStateChanged(bool)"].connect(this, "updatePlayButtonIcon");
	this.playbackEngine["mediaChanged(const QString &)"].connect(this.waveformSlider["drawFile(const QString &)"]);
	this.playbackEngine["finished()"].connect(this.playlistWidget.activateNext);
	this.playlistWidget["itemActivated2(const QString &)"].connect(this, "play");

	this.volumeSlider["sliderMoved(int)"].connect(this, "on_volumeSlider_sliderMoved");
	this.playbackEngine["volumeChanged(qreal)"].connect(this, "volumeSlider_setValue");

	this.waveformSlider["sliderMoved(int)"].connect(this, "on_waveformSlider_sliderMoved");
	this.playbackEngine["positionChanged(qreal)"].connect(this, "waveformSlide_setValue");

	this.dropArea["filesDropped(const QStringList &)"].connect(this.playlistWidget["activateMediaList(const QStringList &)"]);

	this.window.windowFlags = Qt.Window | Qt.FramelessWindowHint;

	this.closeButton.clicked.connect(this.window.close);
	this.minimizeButton.clicked.connect(this.window.minimize);

	this.titleLabel.shadowEnabled = true;
	this.titleLabel.setShadowColor("#FFFFFF");
	this.titleLabel.setShadowOffset(0, 1);

	this.window["newTitle(const QString &)"].connect(this, "setTitle");
	this.window.resized.connect(this, "on_resized");

/*	this.playlistToggleButton.clicked.connect(this, "on_playlistToggleButtonClicked");
	this.playlistToggleButton.setParent(this.playlistWidget);
	this.playlistToggleButton.setParent(this.dropArea);
	this.playlistToggleButton.show();*/
	this.playlistToggleButton.hide();

	this.shadowWidget.setParent(this.dropArea);
	this.shadowWidget.setParent(this.playlistWidget);
	this.shadowWidget.show();

	//print("script loaded");
}

Program.prototype.updatePlayButtonIcon = function(playState)
{
	if (playState) {
		this.playButton.styleSheet = "qproperty-icon: url(pause.png)";
	} else {
		this.playButton.styleSheet = "qproperty-icon: url(play.png)";
	}
}

Program.prototype.on_resized = function()
{
	this.playlistToggleButton.move(this.playlistToggleButton.parentWidget().width -
									this.playlistToggleButton.width - 30,
									this.playlistToggleButton.parentWidget().height -
									this.playlistToggleButton.height);

	this.shadowWidget.resize(this.playlistWidget.width, this.shadowWidget.height);
}

Program.prototype.play = function(path)
{
	this.playbackEngine.setMedia(path);
	this.playbackEngine.play();
}

Program.prototype.on_playlistToggleButtonClicked = function()
{
	if (this.playlistWidget.visible) {
		this.playlistWidget.hide();
		this.window.maximumHeight = 0;
	} else {
		this.playlistWidget.show();
		this.window.maximumHeight = 0xFFFFFF;
	}
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

Program.prototype.setTitle = function(title)
{
	this.titleLabel.text = title;
	this.titleLabel.toolTip = title;
}

/* vim: set ts=4 sw=4: */
