/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2025 Sergey Vlasov <sergey@vlasov.me>
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

import QtQuick 2.15
import NWaveformView 1.0

Item {
  property alias waveform: waveform

  property Component grooveDelegate: Rectangle {
    color: "#0074FF"
    width: 1
  }

  property Component dropAreaDelegate: Rectangle {
    anchors.fill: parent
    color: "#0069E032"
  }

  NWaveformView {
    id: waveform
    anchors.fill: parent
    visible: false
  }

  MouseArea {
    anchors.fill: parent
    onPressed: {
      let position = mouseX / parent.width;
      NPlaybackEngine.position = position;
      // to avoid waiting for positionChanged signal:
      NPlaybackEngine.positionChanged(position);
    }
  }

  DropArea {
    id: dropArea
    anchors.fill: parent
    keys: ["text/uri-list"]
    onDropped: {
      NPlaylistController.model().clear();
      NPlaylistController.dropUrls(0, drop.urls);
      NPlaylistController.playRow(0);
    }
  }

  Loader {
    sourceComponent: grooveDelegate
    height: parent.height
    x: parent.width * NPlaybackEngine.position
    z: 1
  }

  Loader {
    sourceComponent: dropAreaDelegate
    visible: dropArea.containsDrag
    anchors.fill: parent
    z: 2
  }
}
