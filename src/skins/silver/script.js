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

        Ui.shadowWidget.setParent(Ui.splitTop);
        Ui.shadowWidget.setParent(Ui.playlistWidget);
        Ui.shadowWidget.show();

        if (!Settings.value("SilverSkin/Splitter")) {
            Settings.setValue("SilverSkin/Splitter", [200, 200]);
        }

        if (!Settings.value("SilverSkin/SplitterFullScreen")) {
            Settings.setValue("SilverSkin/SplitterFullScreen", [999999, 0]);
        }

        Ui.splitter["splitterMoved(int, int)"].connect(this, "on_splitterMoved");

        Ui.mainWindow.windowFlags = (Ui.mainWindow.windowFlags | Qt.FramelessWindowHint | Qt.WindowCloseButtonHint) ^ (Qt.WindowTitleHint | Qt.Dialog);

        if (Q_WS == "mac") {
            Ui.mainWindow.setAttribute(Qt.WA_MacBrushedMetal, true);
            Ui.playlistWidget.setAttribute(Qt.WA_MacShowFocusRect, false);
            Ui.borderWidget.layout().setContentsMargins(0, 0, 0, 0);
            Ui.borderWidget.styleSheet = "#borderWidget { background: #6e6e6e; }"
            Ui.titleLabel.setFontSize(12);
            Ui.sizeGrip.setParent(Ui.borderWidget);
        } else {
            Ui.sizeGrip.hide();
            Ui.mainWindow.setSizeGripEnabled(true);
        }

        this.marginsBkp_ = Ui.borderWidget.layout().contentsMargins();

        if (WS_WM_BUTTON_DIRECTION == "left") {
            Ui.titleWidget.layout().insertWidget(0, Ui.closeWrapperOuter);
            Ui.titleWidget.layout().insertWidget(1, Ui.minimizeWrapperOuter);
            Ui.titleWidget.layout().insertWidget(5, Ui.iconLabel);
        }

        this.undecoratedSpacing_ = 2;
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
    if (Settings.value("SilverSkin/PlaylistVisible", true) == 'false') {
        Settings.setValue("SilverSkin/PlaylistVisible", false);
        Ui.playlistWidget.hide();
        Ui.mainWindow.minimumHeight = Ui.mainWindow.maximumHeight = this.maximumHeight;
        Ui.mainWindow.resize(Ui.mainWindow.width, Ui.mainWindow.minimumHeigh);
    }

    Ui.splitter.setSizes(Settings.value("SilverSkin/Splitter"));
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

    Ui.shadowWidget.resize(Ui.playlistWidget.width, Ui.shadowWidget.height);
}

Main.prototype.on_splitterMoved = function(pos, index)
{
    if (Ui.mainWindow.isFullSceen()) {
        Settings.setValue("SilverSkin/SplitterFullScreen", Ui.splitter.sizes());
    } else {
        Settings.setValue("SilverSkin/Splitter", Ui.splitter.sizes());
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
        Ui.splitter.setSizes(Settings.value("SilverSkin/SplitterFullScreen"));
    } else {
        Ui.splitter.setSizes(Settings.value("SilverSkin/Splitter"));
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
    if (!enabled) {
        Ui.borderWidget.layout().setContentsMargins(0, 0, 0, 0);
    } else {
        Ui.borderWidget.layout().setContentsMargins(this.marginsBkp_);
    }
}
