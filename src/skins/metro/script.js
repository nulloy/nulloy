/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2014 Sergey Vlasov <sergey@vlasov.me>
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

function Main()
{
	try {
		Ui.repeatCheckBox["clicked(bool)"].connect(Ui.playlistWidget["setRepeatMode(bool)"]);
		Ui.playlistWidget["repeatModeChanged(bool)"].connect(Ui.repeatCheckBox["setChecked(bool)"]);
		Ui.repeatCheckBox.setChecked(Ui.playlistWidget.repeatMode());

		Ui.shuffleCheckBox["clicked(bool)"].connect(Ui.playlistWidget["setShuffleMode(bool)"]);
		Ui.playlistWidget["shuffleModeChanged(bool)"].connect(Ui.shuffleCheckBox["setChecked(bool)"]);

		Ui.playButton.clicked.connect(this, "on_playButton_clicked");
		Ui.stopButton.clicked.connect(PlaybackEngine.stop);
		Ui.prevButton.clicked.connect(Ui.playlistWidget.playPreviousRow);

		Ui.titleWidget.enableDoubleClick();
		Ui.titleWidget.doubleClicked.connect(Ui.mainWindow.toggleMaximize);

		Ui.nextButton.clicked.connect(Ui.playlistWidget.playNextRow);

		Ui.volumeSlider.minimum = 0;
		Ui.volumeSlider.maximum = 100;

		Ui.waveformSlider.minimum = 0;
		Ui.waveformSlider.maximum = 10000;

		PlaybackEngine["stateChanged(N::PlaybackState)"].connect(this, "on_stateChanged");
		PlaybackEngine["mediaChanged(const QString &)"].connect(Ui.waveformSlider["drawFile(const QString &)"]);
		PlaybackEngine["mediaChanged(const QString &)"].connect(Ui.coverWidget["setSource(const QString &)"]);
		PlaybackEngine["finished()"].connect(Ui.playlistWidget.currentFinished);
		PlaybackEngine["failed()"].connect(this, "on_failed");
		Ui.playlistWidget["mediaSet(const QString &)"].connect(PlaybackEngine["setMedia(const QString &)"]);
		Ui.playlistWidget["currentActivated()"].connect(PlaybackEngine.play);

		Ui.volumeSlider["sliderMoved(qreal)"].connect(PlaybackEngine["setVolume(qreal)"]);
		PlaybackEngine["volumeChanged(qreal)"].connect(Ui.volumeSlider["setValue(qreal)"]);

		Ui.waveformSlider["sliderMoved(qreal)"].connect(PlaybackEngine["setPosition(qreal)"]);
		PlaybackEngine["positionChanged(qreal)"].connect(Ui.waveformSlider["setValue(qreal)"]);

		Ui.dropArea["filesDropped(const QStringList &)"].connect(Ui.playlistWidget["playFiles(const QStringList &)"]);
		Ui.mainWindow.windowFlags = (Ui.mainWindow.windowFlags | Qt.FramelessWindowHint | Qt.WindowCloseButtonHint) ^ (Qt.WindowTitleHint | Qt.Dialog);

		Ui.closeButton.clicked.connect(Ui.mainWindow.close);
		Ui.minimizeButton.clicked.connect(Ui.mainWindow.showMinimized);

		Ui.mainWindow["newTitle(const QString &)"].connect(this, "setTitle");
		Ui.mainWindow["fullScreenEnabled(bool)"].connect(this, "on_fullScreenEnabled");
		Ui.mainWindow["maximizeEnabled(bool)"].connect(this, "on_maximizeEnabled");
		Ui.mainWindow.resized.connect(this, "on_resized");

		Ui.splitter["splitterMoved(int, int)"].connect(this, "on_splitterMoved");

		if (Q_WS == "mac") {
			Ui.mainWindow.setAttribute(Qt.WA_MacBrushedMetal, true);
			Ui.playlistWidget.setAttribute(Qt.WA_MacShowFocusRect, false);
			Ui.titleLabel.setFontSize(12);
			Ui.sizeGrip.setParent(Ui.borderWidget);
		} else {
			Ui.sizeGrip.hide();
			Ui.mainWindow.setSizeGripEnabled(true);
		}

		if (WS_WM_BUTTON_DIRECTION == "left") {
			Ui.titleWidget.layout().insertWidget(0, Ui.minimizeButton);
			Ui.titleWidget.layout().insertWidget(0, Ui.closeButton);
			Ui.titleWidget.layout().insertWidget(10, Ui.themeButton);
			Ui.titleWidget.layout().insertWidget(10, Ui.iconLabel);
		}
	} catch (err) {
		print("QtScript: " + err);
	}
}

Main.prototype.afterShow = function()
{
	if (Settings.value("MetroSkin/Splitter"))
		Ui.splitter.setSizes(Settings.value("MetroSkin/Splitter"));

	this.darkTheme = Ui.mainWindow.styleSheet;
	this.lightTheme = readFile("light.css");

	maskImage("shuffle.png", "#000000", 0.4);
	maskImage("repeat.png",  "#000000", 0.4);

	Ui.themeButton["clicked(bool)"].connect(this, "loadTheme");
	Ui.themeButton.checked = (Settings.value("MetroSkin/LightTheme") == "true");
	this.loadTheme(Ui.themeButton.checked);
}

Main.prototype.loadTheme = function(useLightTheme)
{
	if (useLightTheme == true) {
		var styleSheet = this.darkTheme + this.lightTheme
		var color = "#3D3D3D";
	} else {
		var styleSheet = this.darkTheme;
		var color = "#FFFFFF";
	}

	maskImage("prev.png", color);
	maskImage("stop.png", color);
	maskImage("play.png", color);
	maskImage("pause.png", color);
	maskImage("next.png", color);
	maskImage("close.png", color);
	maskImage("minimize.png", color);
	maskImage("shuffle.png", color);
	maskImage("repeat.png",  color);
	maskImage("icon.png",  color);
	maskImage("theme.png",  color);

	Ui.mainWindow.styleSheet = styleSheet;
	Ui.playButton.styleSheet = Ui.playButton.styleSheet;

	Settings.setValue("MetroSkin/LightTheme", useLightTheme);
}

Main.prototype.on_playButton_clicked = function()
{
	if (!Ui.playlistWidget.hasCurrent())
		Ui.playlistWidget.playRow(0);
	else
		PlaybackEngine.play(); // toggle play/pause
}

Main.prototype.on_stateChanged = function(state)
{
	if (state == N.PlaybackPlaying)
		Ui.playButton.styleSheet = "qproperty-icon: url(pause.png)";
	else
		Ui.playButton.styleSheet = "qproperty-icon: url(play.png)";

	Ui.waveformSlider.setPausedState(state == N.PlaybackPaused);
}

Main.prototype.on_failed = function()
{
	Ui.playlistWidget.currentFailed();
	Ui.playlistWidget.playNextRow();
}

Main.prototype.on_resized = function()
{
	if (Q_WS == "mac") {
		Ui.sizeGrip.move(Ui.sizeGrip.parentWidget().width -
		                 Ui.sizeGrip.width - 5,
		                 Ui.sizeGrip.parentWidget().height -
		                 Ui.sizeGrip.height - 4);
	}
}

Main.prototype.on_splitterMoved = function(pos, index)
{
	Settings.setValue("MetroSkin/Splitter", Ui.splitter.sizes());
}

Main.prototype.setTitle = function(title)
{
	Ui.titleLabel.text = title;
	Ui.titleLabel.toolTip = title;
}

Main.prototype.on_fullScreenEnabled = function(enabled)
{
	Ui.controlsContainer.setVisible(!enabled);
	Ui.titleWidget.setVisible(!enabled);
	Ui.playlistWidget.setVisible(!enabled);

	this.setBorderVisible(!enabled);
}

Main.prototype.on_maximizeEnabled = function(enabled)
{
	this.setBorderVisible(!enabled);
}

Main.prototype.setBorderVisible = function(enabled)
{
	if (enabled)
		Ui.borderWidget.layout().setContentsMargins(1, 1, 1, 1);
	else
		Ui.borderWidget.layout().setContentsMargins(0, 0, 0, 0);
}
