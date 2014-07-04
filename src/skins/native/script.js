/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2014 Sergey Vlasov <sergey@vlasov.me>
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
		Ui.repeatCheckBox["clicked(bool)"].connect(Ui.playlistWidget["setRepeatMode(bool)"]);
		Ui.playlistWidget["repeatModeChanged(bool)"].connect(Ui.repeatCheckBox["setChecked(bool)"]);
		Ui.repeatCheckBox.setChecked(Ui.playlistWidget.repeatMode());

		Ui.shuffleCheckBox["clicked(bool)"].connect(Ui.playlistWidget["setShuffleMode(bool)"]);
		Ui.playlistWidget["shuffleModeChanged(bool)"].connect(Ui.shuffleCheckBox["setChecked(bool)"]);

		Ui.playButton.clicked.connect(this, "on_playButton_clicked");
		Ui.stopButton.clicked.connect(PlaybackEngine.stop);
		Ui.prevButton.clicked.connect(Ui.playlistWidget.playPreviousRow);
		Ui.nextButton.clicked.connect(Ui.playlistWidget.playNextRow);

		Ui.playButton.setStandardIcon("media-playback-start", ":play.png");
		Ui.stopButton.setStandardIcon("media-playback-stop", ":stop.png");
		Ui.prevButton.setStandardIcon("media-skip-backward", ":prev.png");
		Ui.nextButton.setStandardIcon("media-skip-forward", ":next.png");
		Ui.repeatCheckBox.setStandardIcon("media-playlist-repeat", ":repeat.png");
		Ui.shuffleCheckBox.setStandardIcon("media-playlist-shuffle", ":shuffle.png");

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
		Ui.mainWindow["fullScreenEnabled(bool)"].connect(this, "on_fullScreenEnabled");
		Ui.mainWindow.windowFlags = (Ui.mainWindow.windowFlags | Qt.WindowMinMaxButtonsHint) ^ Qt.Dialog;

		Ui.splitter["splitterMoved(int, int)"].connect(this, "on_splitterMoved");

		if (Q_WS == "mac") {
			Ui.mainWindow.styleSheet = "";
			Ui.mainWindow.setAttribute(Qt.WA_MacBrushedMetal, true);

			Ui.playlistWidget.styleSheet = "";
			Ui.playlistWidget.setAttribute(Qt.WA_MacShowFocusRect, false);

			Ui.dropArea.layout().setContentsMargins(10, 7, 10, 7);
			Ui.dropArea.layout().setSpacing(7);

			var margins = Ui.controlsContainer.layout().contentsMargins();
			margins.right = 7;
			Ui.controlsContainer.layout().setContentsMargins(margins);

			var buttons = new Array(Ui.playButton, Ui.stopButton, Ui.prevButton, Ui.nextButton);
			for (var i = 0; i < buttons.length; ++i) {
				buttons[i].minimumWidth += 15;
				buttons[i].minimumHeight = 32;
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
	if (Settings.value("NativeSkin/Splitter"))
		Ui.splitter.setSizes(Settings.value("NativeSkin/Splitter"));
}

Main.prototype.on_splitterMoved = function(pos, index)
{
	Settings.setValue("NativeSkin/Splitter", Ui.splitter.sizes());
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
		Ui.playButton.setStandardIcon("media-playback-pause", ":pause.png");
	else
		Ui.playButton.setStandardIcon("media-playback-start", ":play.png");

	Ui.waveformSlider.setPausedState(state == N.PlaybackPaused);
}

Main.prototype.on_failed = function()
{
	Ui.playlistWidget.currentFailed();
	Ui.playlistWidget.playNextRow();
}

Main.prototype.on_fullScreenEnabled = function(enabled)
{
	Ui.controlsContainer.setVisible(!enabled);
	Ui.playlistContainer.setVisible(!enabled);
}

