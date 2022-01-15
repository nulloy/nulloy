/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2018 Sergey Vlasov <sergey@vlasov.me>
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
        }

    } catch (err) {
        print("QtScript: " + err);
    }

    Ui.titleWidget.setVisible(false);

    Ui.playButton.hide();
    Ui.prevButton.hide();
    Ui.nextButton.hide();
}

Main.prototype.afterShow = function()
{
    if (Settings.value("SlimSkin/Splitter"))
        Ui.splitter.setSizes(Settings.value("SlimSkin/Splitter"));
    else
        Ui.splitter.setSizes([30, 0]);

    Ui.menuButton["clicked()"].connect(this, "showMenu");
}

Main.prototype.showMenu = function()
{
    var menuPos = Ui.menuButton.pos;
    menuPos.y += Ui.menuButton.height;
    Player.showContextMenu(menuPos);
}

Main.prototype.on_splitterMoved = function(pos, index)
{
    Settings.setValue("SlimSkin/Splitter", Ui.splitter.sizes());
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
