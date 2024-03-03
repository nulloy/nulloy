/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2024 Sergey Vlasov <sergey@vlasov.me>
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
import NImage 1.0
import QtQuick.Window 2.2
import QtGraphicalEffects 1.0

NImage {
  id: item
  objectName: "coverImage"
  property alias containsMouse: mouseArea.containsMouse

  MouseArea {
    id: mouseArea
    anchors.fill: parent
    hoverEnabled: true

    onClicked: {
      fadeInAnimator.start();
    }
  }

  Item {
    id: popupWrapper
    parent: Window.contentItem
    anchors.fill: parent
    visible: false
    opacity: 0.0
    z: 1

    property int imageMargin: 50
    property int imageWidth: popupImage.implicitWidth - imageMargin * 2
    property int imageHeight: popupImage.implicitHeight - imageMargin * 2

    OpacityAnimator {
      id: fadeInAnimator
      target: popupWrapper
      from: 0.0
      to: 1.0
      duration: 150
      easing.type: Easing.OutQuad
      onStarted: {
        popupWrapper.visible = true;
        popupWrapper.focus = true;
      }
    }

    OpacityAnimator {
      id: fadeOutAnimator
      target: popupWrapper
      from: 1.0
      to: 0.0
      duration: 150
      easing.type: Easing.OutQuad
      onStopped: {
        popupWrapper.visible = false;
        popupWrapper.focus = false;
      }
    }

    Keys.onPressed: event => {
      if (event.key === Qt.Key_Escape) {
        fadeOutAnimator.start();
      }
    }

    Rectangle {
      anchors.fill: parent
      color: "black"
      opacity: 0.6
      MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
          fadeOutAnimator.start();
        }
      }
    }

    RectangularGlow {
      anchors.centerIn: parent
      width: popupWrapper.imageWidth
      height: popupWrapper.imageHeight
      color: "black"
      opacity: 0.6
      glowRadius: 30
      spread: 0.2
    }

    Rectangle {
      x: Math.ceil((parent.width - width) / 2)
      y: Math.ceil((parent.height - height) / 2)
      width: popupWrapper.imageWidth + 2
      height: popupWrapper.imageHeight + 2
      border.color: "white"
      color: "transparent"
      Rectangle {
        anchors.fill: parent
        anchors.margins: 1
        color: "black"
        opacity: 0.6
      }
    }

    NImage {
      id: popupImage
      anchors.fill: parent
      margin: popupWrapper.imageMargin
      image: item.image
      upscale: true
    }
  }
}
