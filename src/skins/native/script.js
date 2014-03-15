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
		this.coverWidget = this.mainWindow.findChild("coverWidget");

		this.repeatCheckBox = this.mainWindow.findChild("repeatCheckBox");
		this.repeatCheckBox["clicked(bool)"].connect(this.playlistWidget["setRepeatMode(bool)"]);
		this.playlistWidget["repeatModeChanged(bool)"].connect(this.repeatCheckBox["setChecked(bool)"]);
		this.repeatCheckBox.setChecked(this.playlistWidget.repeatMode());

		this.shuffleCheckBox = this.mainWindow.findChild("shuffleCheckBox");
		this.shuffleCheckBox["clicked(bool)"].connect(this.playlistWidget["setShuffleMode(bool)"]);
		this.playlistWidget["shuffleModeChanged(bool)"].connect(this.shuffleCheckBox["setChecked(bool)"]);

		this.playButton.clicked.connect(this, "on_playButton_clicked");
		this.stopButton.clicked.connect(this.playbackEngine.stop);
		this.prevButton.clicked.connect(this.playlistWidget.playPreviousRow);
		this.nextButton.clicked.connect(this.playlistWidget.playNextRow);

		this.playButton.setStandardIcon("media-playback-start", ":play.png");
		this.stopButton.setStandardIcon("media-playback-stop", ":stop.png");
		this.prevButton.setStandardIcon("media-skip-backward", ":prev.png");
		this.nextButton.setStandardIcon("media-skip-forward", ":next.png");
		this.repeatCheckBox.setStandardIcon("media-playlist-repeat", ":repeat.png");
		this.shuffleCheckBox.setStandardIcon("media-playlist-shuffle", ":shuffle.png");

		this.volumeSlider.minimum = 0;
		this.volumeSlider.maximum = 100;

		this.waveformSlider.minimum = 0;
		this.waveformSlider.maximum = 10000;

		this.playbackEngine["stateChanged(N::PlaybackState)"].connect(this, "on_stateChanged");
		this.playbackEngine["mediaChanged(const QString &)"].connect(this.waveformSlider["drawFile(const QString &)"]);
		this.playbackEngine["mediaChanged(const QString &)"].connect(this.coverWidget["setSource(const QString &)"]);
		this.playbackEngine["finished()"].connect(this.playlistWidget.currentFinished);
		this.playbackEngine["failed()"].connect(this, "on_failed");
		this.playlistWidget["mediaSet(const QString &)"].connect(this.playbackEngine["setMedia(const QString &)"]);
		this.playlistWidget["currentActivated()"].connect(this.playbackEngine.play);

		this.volumeSlider["sliderMoved(int)"].connect(this, "on_volumeSlider_sliderMoved");
		this.playbackEngine["volumeChanged(qreal)"].connect(this, "volumeSlider_setValue");

		this.waveformSlider["sliderMoved(int)"].connect(this, "on_waveformSlider_sliderMoved");
		this.playbackEngine["positionChanged(qreal)"].connect(this, "waveformSlider_setValue");

		this.dropArea["filesDropped(const QStringList &)"].connect(this.playlistWidget["playFiles(const QStringList &)"]);
		this.mainWindow.windowFlags = (this.mainWindow.windowFlags | Qt.WindowMinMaxButtonsHint) ^ Qt.Dialog;

		this.splitter = this.mainWindow.findChild("splitter");
		this.splitter["splitterMoved(int, int)"].connect(this, "on_splitterMoved");

		if (Q_WS == "mac") {
			this.mainWindow.styleSheet = "";
			this.mainWindow.setAttribute(Qt.WA_MacBrushedMetal, true);

			var playlistWidget = this.mainWindow.findChild("playlistWidget");
			playlistWidget.styleSheet = "";
			playlistWidget.setAttribute(Qt.WA_MacShowFocusRect, false);

			this.dropArea.layout().setContentsMargins(10, 7, 10, 7);
			this.dropArea.layout().setSpacing(7);

			var buttonsLayout = this.mainWindow.findChild("buttonsLayout");
			var margins = buttonsLayout.contentsMargins();
			margins.right = 7;
			buttonsLayout.setContentsMargins(margins);

			var buttons = new Array(this.playButton, this.stopButton, this.prevButton, this.nextButton);
			for (var i = 0; i < buttons.length; ++i) {
				buttons[i].minimumWidth += 15;
				buttons[i].minimumHeight = 32;
			}

			var line = this.mainWindow.findChild("line");
			line.styleSheet = "QFrame:active { background: #8e8e8e; }";
			line.maximumHeight = 2;

			this.mainWindow.setSizeGripEnabled(false);
		} else {
			this.mainWindow.setSizeGripEnabled(true);
		}
	} catch (err) {
		print("QtScript: " + err);
	}
}

Program.prototype.afterShow = function()
{
	if (this.player.settings().value("NativeSkin/Splitter"))
		this.splitter.setSizes(this.player.settings().value("NativeSkin/Splitter"));
}

Program.prototype.on_splitterMoved = function(pos, index)
{
	this.player.settings().setValue("NativeSkin/Splitter", this.splitter.sizes());
}

Program.prototype.on_playButton_clicked = function()
{
	if (!this.playlistWidget.hasCurrent())
		this.playlistWidget.playRow(0);
	else
		this.playbackEngine.play(); // toggle play/pause
}

Program.prototype.on_stateChanged = function(state)
{
	if (state == N.PlaybackPlaying)
		this.playButton.setStandardIcon("media-playback-pause", ":pause.png");
	else
		this.playButton.setStandardIcon("media-playback-start", ":play.png");

	this.waveformSlider.setPausedState(state == N.PlaybackPaused);
}

Program.prototype.on_failed = function()
{
	this.playlistWidget.currentFailed();
	this.playlistWidget.playNextRow();
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

Program.prototype.waveformSlider_setValue = function(value)
{
	this.waveformSlider.value = Math.round(value * this.waveformSlider.maximum);
}

