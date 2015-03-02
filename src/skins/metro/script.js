/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2015 Sergey Vlasov <sergey@vlasov.me>
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
		Ui.titleWidget.enableDoubleClick();
		Ui.titleWidget.doubleClicked.connect(Ui.mainWindow.toggleMaximize);

		PlaybackEngine["stateChanged(N::PlaybackState)"].connect(this, "on_stateChanged");

		Ui.mainWindow["newTitle(const QString &)"].connect(this, "setTitle");
		Ui.mainWindow["fullScreenEnabled(bool)"].connect(this, "on_fullScreenEnabled");
		Ui.mainWindow["maximizeEnabled(bool)"].connect(this, "on_maximizeEnabled");
		Ui.mainWindow.resized.connect(this, "on_resized");
		Ui.mainWindow["focusChanged(bool)"].connect(this, "on_focusChanged");

		Ui.splitter["splitterMoved(int, int)"].connect(this, "on_splitterMoved");

		Ui.mainWindow.windowFlags = (Ui.mainWindow.windowFlags | Qt.FramelessWindowHint | Qt.WindowCloseButtonHint) ^ (Qt.WindowTitleHint | Qt.Dialog);

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

Main.prototype.on_stateChanged = function(state)
{
	if (state == N.PlaybackPlaying)
		Ui.playButton.styleSheet = "qproperty-icon: url(pause.png)";
	else
		Ui.playButton.styleSheet = "qproperty-icon: url(play.png)";
}

Main.prototype.on_failed = function()
{
	Ui.playlistWidget.currentFailed();
	Ui.playlistWidget.playNextItem();
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
	Ui.borderWidget.styleSheet = enabled ? "" : "#borderWidget { background-color: #3D3D3D; }";
}

Main.prototype.on_focusChanged = function(focused)
{
	this.setBorderVisible(focused);
}

