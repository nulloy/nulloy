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
		Ui.splitter["splitterMoved(int, int)"].connect(this, "on_splitterMoved");

		Ui.mainWindow["newTitle(const QString &)"].connect(this, "setTitle");
		Ui.mainWindow["fullScreenEnabled(bool)"].connect(this, "on_fullScreenEnabled");
		Ui.mainWindow["maximizeEnabled(bool)"].connect(this, "on_maximizeEnabled");

		Ui.mainWindow.windowFlags = (Ui.mainWindow.windowFlags | Qt.FramelessWindowHint | Qt.WindowCloseButtonHint) ^ (Qt.WindowTitleHint | Qt.Dialog);
		Ui.waveformSlider.minimumHeight = 30;

		if (Q_WS == "mac") {
			Ui.mainWindow.setAttribute(Qt.WA_MacBrushedMetal, true);
			Ui.playlistWidget.setAttribute(Qt.WA_MacShowFocusRect, false);
			Ui.titleLabel.setFontSize(12);
		}

		if (WS_WM_BUTTON_DIRECTION == "left") {
			Ui.titleWidget.layout().insertWidget(0, Ui.minimizeButton);
			Ui.titleWidget.layout().insertWidget(0, Ui.closeButton);
			Ui.titleWidget.layout().insertWidget(10, Ui.iconLabel);
		}
	} catch (err) {
		print("QtScript: " + err);
	}
}

Main.prototype.afterShow = function()
{
	if (Settings.value("PixieSkin/Splitter"))
		Ui.splitter.setSizes(Settings.value("PixieSkin/Splitter"));
	else
		Ui.splitter.setSizes([30, 0]);
}

Main.prototype.on_splitterMoved = function(pos, index)
{
	Settings.setValue("PixieSkin/Splitter", Ui.splitter.sizes());
}

Main.prototype.setTitle = function(title)
{
	Ui.titleLabel.text = title;
	Ui.titleLabel.toolTip = title;
}

Main.prototype.on_stateChanged = function(state)
{
	if (state == N.PlaybackPlaying)
		Ui.playButton.styleSheet = "#playButton{background-image:url(pause.png)} #playButton:hover{background-image:url(pause-hover.png)} #playButton:pressed{background-image:url(pause-press.png)}";
	else
		Ui.playButton.styleSheet = "";
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
	Ui.borderWidget.styleSheet = enabled ? "" : "#borderWidget { border: none; }";
}
