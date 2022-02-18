/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2022 Sergey Vlasov <sergey@vlasov.me>
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

function Main()
{
    try {
        Ui.playButton.setStandardIcon("media-playback-start", ":play.png");
        Ui.stopButton.setStandardIcon("media-playback-stop", ":stop.png");
        Ui.prevButton.setStandardIcon("media-skip-backward", ":prev.png");
        Ui.nextButton.setStandardIcon("media-skip-forward", ":next.png");
        Ui.repeatButton.setStandardIcon("media-playlist-repeat", ":repeat.png");
        Ui.shuffleButton.setStandardIcon("media-playlist-shuffle", ":shuffle.png");

        PlaybackEngine["stateChanged(N::PlaybackState)"].connect(this, "on_stateChanged");

        Ui.mainWindow["fullScreenEnabled(bool)"].connect(this, "on_fullScreenEnabled");
        Ui.mainWindow["showPlaybackControlsEnabled(bool)"].connect(this, "on_showPlaybackControlsEnabled");

        if (!Settings.value("NativeSkin/Splitter")) {
            Settings.setValue("NativeSkin/Splitter", [200, 200]);
        }

        Ui.splitter["splitterMoved(int, int)"].connect(this, "on_splitterMoved");

        Ui.mainWindow.windowFlags = (Ui.mainWindow.windowFlags | Qt.WindowMinMaxButtonsHint) ^ Qt.Dialog;

        if (Q_WS == "mac") {
            Ui.mainWindow.styleSheet = "";
            Ui.mainWindow.setAttribute(Qt.WA_MacBrushedMetal, true);

            Ui.playlistWidget.styleSheet = Ui.playlistWidget.styleSheet + "#playlistWidget QScrollBar { margin-bottom: 0; }";
            Ui.playlistWidget.setAttribute(Qt.WA_MacShowFocusRect, false);

            Ui.splitTop.layout().setContentsMargins(10, 7, 10, 0);
            Ui.splitTop.layout().setSpacing(0);

            var margins = Ui.controlsContainer.layout().contentsMargins();
            margins.right = 7;
            Ui.controlsContainer.layout().setContentsMargins(margins);

            var buttons = new Array(Ui.playButton, Ui.stopButton, Ui.prevButton, Ui.nextButton);
            for (var i = 0; i < buttons.length; ++i) {
                buttons[i].minimumWidth = 60;
                buttons[i].minimumHeight = 32;
            }

            var toolButtons = new Array(Ui.repeatButton, Ui.shuffleButton);
            for (var i = 0; i < toolButtons.length; ++i) {
                toolButtons[i].maximumHeight = 25;
                toolButtons[i].styleSheet = "margin-top: 4px";
            }

            Ui.line.styleSheet = "QFrame:active { background: #8e8e8e; }";
            Ui.line.maximumHeight = 2;

            Ui.mainWindow.setSizeGripEnabled(false);
        } else {
            Ui.mainWindow.setSizeGripEnabled(true);
        }
    } catch (err) {
        print("QtScript: " + err);
    }
}

Main.prototype.afterShow = function()
{
    Ui.splitter.setSizes(Settings.value("NativeSkin/Splitter"));
}

Main.prototype.on_splitterMoved = function(pos, index)
{
    if (Ui.mainWindow.isFullSceen()) {
        Settings.setValue("NativeSkin/SplitterFullScreen", Ui.splitter.sizes());
    } else {
        Settings.setValue("NativeSkin/Splitter", Ui.splitter.sizes());
    }
}

Main.prototype.on_stateChanged = function(state)
{
    if (state == N.PlaybackPlaying)
        Ui.playButton.setStandardIcon("media-playback-pause", ":pause.png");
    else
        Ui.playButton.setStandardIcon("media-playback-start", ":play.png");
}

Main.prototype.on_fullScreenEnabled = function(enabled)
{
    if (enabled) {
        Ui.splitter.setSizes(Settings.value("NativeSkin/SplitterFullScreen"));
    } else {
        Ui.splitter.setSizes(Settings.value("NativeSkin/Splitter"));
    }

    if (Settings.value("ShowPlaybackControls")) {
        Ui.controlsContainer.setVisible(!enabled);
    }
}

Main.prototype.on_showPlaybackControlsEnabled = function(enabled)
{
    if (!Ui.mainWindow.isFullSceen()) {
        Ui.controlsContainer.setVisible(enabled);
    }
}
