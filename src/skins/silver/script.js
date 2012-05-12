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

function Program(player)
{
	try {
		this.player = player;
		this.mainWindow = player.mainWindow();
		this.playbackEngine = player.playbackEngine();
		this.playlistWidget = this.mainWindow.findChild("playlistWidget");
		this.dropArea = this.mainWindow.findChild("dropArea");
		this.playButton = this.mainWindow.findChild("playButton");
		this.stopButton = this.mainWindow.findChild("stopButton");
		this.prevButton = this.mainWindow.findChild("prevButton");
		this.nextButton = this.mainWindow.findChild("nextButton");
		this.volumeSlider = this.mainWindow.findChild("volumeSlider");
		this.waveformSlider = this.mainWindow.findChild("waveformSlider");
		this.sizeGrip = this.mainWindow.findChild("sizeGrip");
		this.playlistToggleButton = this.mainWindow.findChild("playlistToggleButton");
		this.shadowWidget = this.mainWindow.findChild("shadowWidget");
		this.closeButton = this.mainWindow.findChild("closeButton");
		this.minimizeButton = this.mainWindow.findChild("minimizeButton");
		this.titleLabel = this.mainWindow.findChild("titleLabel");

		this.playButton.clicked.connect(this.playlistWidget.activateCurrent);
		this.stopButton.clicked.connect(this.playbackEngine.stop);
		this.prevButton.clicked.connect(this.playlistWidget.activatePrev);
		this.nextButton.clicked.connect(this.playlistWidget.activateNext);

		this.volumeSlider.minimum = 0;
		this.volumeSlider.maximum = 100;

		this.waveformSlider.minimum = 0;
		this.waveformSlider.maximum = 10000;

		this.playbackEngine["stateChanged(int)"].connect(this, "on_stateChanged");
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

		this.mainWindow.windowFlags = (this.mainWindow.windowFlags | Qt.FramelessWindowHint | Qt.WindowCloseButtonHint) ^ (Qt.WindowTitleHint | Qt.Dialog);

		this.closeButton.clicked.connect(this.mainWindow.close);
		this.minimizeButton.clicked.connect(this.mainWindow.minimize);

		this.titleLabel.shadowEnabled = true;
		this.titleLabel.setShadowColor("#FFFFFF");
		this.titleLabel.setShadowOffset(0, 1);

		this.mainWindow["newTitle(const QString &)"].connect(this, "setTitle");
		this.mainWindow.resized.connect(this, "on_resized");

		this.playlistToggleButton.clicked.connect(this, "on_playlistToggleButtonClicked");
		this.playlistToggleButton.setParent(this.playlistWidget);
		this.playlistToggleButton.setParent(this.dropArea);

		this.shadowWidget.setParent(this.dropArea);
		this.shadowWidget.setParent(this.playlistWidget);
		this.shadowWidget.show();

		this.maximumHeight = 120;
		if (Q_WS == "mac") {
			this.maximumHeight = 118;

			this.mainWindow.setAttribute(Qt.WA_MacBrushedMetal, true);

			var playlistWidget = this.mainWindow.findChild("playlistWidget");
			playlistWidget.setAttribute(Qt.WA_MacShowFocusRect, false);

			var borderWidget = this.mainWindow.findChild("borderWidget");
			borderWidget.layout().setContentsMargins(0, 0, 0, 0);
			borderWidget.styleSheet = "#borderWidget { background: #6e6e6e; }"

			var dropAreaMargins = this.dropArea.layout().contentsMargins();
			dropAreaMargins.bottom = 5;
			this.dropArea.layout().setContentsMargins(dropAreaMargins);
			this.dropArea.layout().setSpacingAt(3, 6);

			var titleLabel = this.mainWindow.findChild("titleLabel");
			titleLabel.setFontSize(12);

			var titleBarlLayout = this.mainWindow.findChild("titleBarlLayout");
			titleBarlLayout.insertWidget(0, this.closeButton);
			titleBarlLayout.insertWidget(1, this.minimizeButton);
			titleBarlLayout.insertWidget(5, this.mainWindow.findChild("iconLabel"));

			this.sizeGrip.setParent(borderWidget);
		} else {
			this.sizeGrip.hide();
			this.mainWindow.setSizeGripEnabled(true);
		}

		if (Q_WS == "win")
			this.mainWindow.setFramelessShadow(true);
	} catch (err) {
		print("QtScript: " + err);
	}
}

Program.prototype.afterShow = function()
{
	if (this.player.settings().value("SilverSkin/PlaylistVisible", true) == 'false') {
		this.player.settings().setValue("SilverSkin/PlaylistVisible", false);
		this.playlistWidget.hide();
		this.mainWindow.minimumHeight = this.mainWindow.maximumHeight = this.maximumHeight;
		this.mainWindow.resize(this.mainWindow.width, this.mainWindow.minimumHeigh);
	}
}

Program.prototype.on_stateChanged = function(state)
{
	if (state == 1) // NPlaybackEngineInterface::Playing == 1
		this.playButton.styleSheet = "qproperty-icon: url(pause.png)";
	else
		this.playButton.styleSheet = "qproperty-icon: url(play.png)";

	this.waveformSlider.setPausedState(state == 2);
}

Program.prototype.on_failed = function()
{
	this.playlistWidget.setCurrentFailed();
	this.playlistWidget.activateNext();
}

Program.prototype.on_resized = function()
{
	this.playlistToggleButton.move(this.playlistToggleButton.parentWidget().width -
									this.playlistToggleButton.width - 40,
									this.playlistToggleButton.parentWidget().height -
									this.playlistToggleButton.height);

	if (Q_WS == "mac") {
		this.sizeGrip.move(this.sizeGrip.parentWidget().width -
							this.sizeGrip.width - 5,
							this.sizeGrip.parentWidget().height -
							this.sizeGrip.height - 4);
	}

	this.shadowWidget.resize(this.playlistWidget.width, this.shadowWidget.height);
}

Program.prototype.on_playlistToggleButtonClicked = function()
{
	this.player.settings().setValue("SilverSkin/PlaylistVisible", !this.playlistWidget.visible);
	if (this.playlistWidget.visible) {
		this.player.settings().setValue("SilverSkin/OldHeight", this.mainWindow.height);
		this.playlistWidget.hide();
		this.mainWindow.minimumHeight = this.mainWindow.maximumHeight = this.maximumHeight;
		this.mainWindow.resize(this.mainWindow.width, this.mainWindow.minimumHeigh);
	} else {
		var oldHeight = this.player.settings().value("SilverSkin/OldHeight", 250);
		this.playlistWidget.show();
		this.mainWindow.maximumHeight = 0xFFFFFF;
		this.mainWindow.minimumHeight = 200;
		this.mainWindow.resize(this.mainWindow.width, oldHeight);
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
