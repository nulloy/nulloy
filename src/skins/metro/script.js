/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2022 Sergey Vlasov <sergey@vlasov.me>
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
        Ui.mainWindow["showPlaybackControlsEnabled(bool)"].connect(this, "on_showPlaybackControlsEnabled");
        Ui.mainWindow["maximizeEnabled(bool)"].connect(this, "on_maximizeEnabled");
        Ui.mainWindow.resized.connect(this, "on_resized");

        if (!Settings.value("MetroSkin/Splitter")) {
            Settings.setValue("MetroSkin/Splitter", [200, 200]);
        }

        if (!Settings.value("MetroSkin/SplitterFullScreen")) {
            Settings.setValue("MetroSkin/SplitterFullScreen", [999999, 0]);
        }

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
            Ui.titleWidget.layout().insertWidget(10, Ui.menuButton);
            Ui.titleWidget.layout().insertWidget(10, Ui.iconLabel);
        }

        this.marginsBkp_ = Ui.borderWidget.layout().contentsMargins();

        this.undecoratedSpacing_ = 6;
        Ui.splitTop.layout().insertSpacing(0, 0);

        if (WS_WM_TILING) {
            Ui.titleWidget.setVisible(false);
            Ui.splitTop.layout().setSpacingAt(0, this.undecoratedSpacing_);
        }
    } catch (err) {
        print("QtScript: " + err);
    }
}

Main.prototype.afterShow = function()
{
    Ui.splitter.setSizes(Settings.value("MetroSkin/Splitter"));

    this.darkTheme = Ui.mainWindow.styleSheet;
    this.lightTheme = readFile("light.css");

    maskImage("shuffle.png", "#000000", 0.4);
    maskImage("repeat.png",  "#000000", 0.4);

    Ui.themeButton["clicked(bool)"].connect(this, "loadTheme");
    Ui.themeButton.checked = (Settings.value("MetroSkin/LightTheme") == "true");
    this.loadTheme(Ui.themeButton.checked);

    Ui.menuButton["clicked()"].connect(this, "showMenu");
}

Main.prototype.showMenu = function()
{
    var menuPos = Ui.menuButton.pos;
    menuPos.y += Ui.menuButton.height;
    Player.showContextMenu(menuPos);
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
    maskImage("repeat.png", color);
    maskImage("icon.png", color);
    maskImage("theme.png", color);
    maskImage("menu.png", color);

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
    if (Ui.mainWindow.isFullSceen()) {
        Settings.setValue("MetroSkin/SplitterFullScreen", Ui.splitter.sizes());
    } else {
        Settings.setValue("MetroSkin/Splitter", Ui.splitter.sizes());
    }
}

Main.prototype.setTitle = function(title)
{
    Ui.titleLabel.text = title;
    Ui.titleLabel.toolTip = title;
}

Main.prototype.on_fullScreenEnabled = function(enabled)
{
    if (enabled) {
        Ui.splitter.setSizes(Settings.value("MetroSkin/SplitterFullScreen"));
    } else {
        Ui.splitter.setSizes(Settings.value("MetroSkin/Splitter"));
    }

    if (Settings.value("ShowPlaybackControls")) {
        Ui.controlsContainer.setVisible(!enabled);
    }

    Ui.titleWidget.setVisible(!enabled);

    Ui.splitTop.layout().setSpacingAt(0, enabled ? this.undecoratedSpacing_ : 0);

    this.setBorderVisible(!enabled);
}

Main.prototype.on_showPlaybackControlsEnabled = function(enabled)
{
    if (!Ui.mainWindow.isFullSceen()) {
        Ui.controlsContainer.setVisible(enabled);
    }
}

Main.prototype.on_maximizeEnabled = function(enabled)
{
    this.setBorderVisible(!enabled);
}

Main.prototype.setBorderVisible = function(enabled)
{
    print(enabled);
    if (!enabled) {
        Ui.borderWidget.layout().setContentsMargins(0, 0, 0, 0);
    } else {
        Ui.borderWidget.layout().setContentsMargins(this.marginsBkp_);
    }
}
