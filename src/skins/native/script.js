/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2013 Sergey Vlasov <sergey@vlasov.me>
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

		this.playButton.clicked.connect(this.playlistWidget.activateCurrent);
		this.stopButton.clicked.connect(this.playbackEngine.stop);
		this.prevButton.clicked.connect(this.playlistWidget.activatePrev);
		this.nextButton.clicked.connect(this.playlistWidget.activateNext);

		this.playButton.setStandardIcon("media-playback-start", ":/trolltech/styles/commonstyle/images/media-play-16.png");
		this.stopButton.setStandardIcon("media-playback-stop", ":/trolltech/styles/commonstyle/images/media-stop-16.png");
		this.prevButton.setStandardIcon("media-skip-backward", ":/trolltech/styles/commonstyle/images/media-skip-backward-16.png");
		this.nextButton.setStandardIcon("media-skip-forward", ":/trolltech/styles/commonstyle/images/media-skip-forward-16.png");

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
		this.mainWindow.windowFlags = (this.mainWindow.windowFlags | Qt.WindowMinimizeButtonHint) ^ Qt.Dialog;

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
	this.splitter.setSizes(this.player.settings().value("NativeSkin/Splitter"));
}

Program.prototype.on_splitterMoved = function(pos, index)
{
	this.player.settings().setValue("NativeSkin/Splitter", this.splitter.sizes());
}

Program.prototype.on_stateChanged = function(state)
{
	if (state == 1) // NPlaybackEngineInterface::Playing == 1
		this.playButton.setStandardIcon("media-playback-pause", ":/trolltech/styles/commonstyle/images/media-pause-16.png");
	else
		this.playButton.setStandardIcon("media-playback-start", ":/trolltech/styles/commonstyle/images/media-play-16.png");

	this.waveformSlider.setPausedState(state == 2);
}

Program.prototype.on_failed = function()
{
	this.playlistWidget.setCurrentFailed();
	this.playlistWidget.activateNext();
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

/* vim: set ts=4 sw=4: */
