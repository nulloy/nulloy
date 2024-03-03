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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.0

import Nulloy 1.0
import NWaveformBar 1.0
import NSvgImage 1.0

ApplicationWindow {
  id: mainWindow
  visible: true
  flags: Qt.FramelessWindowHint
  signal tooltipRequested(string text)

  color: "#4F545B"

  property string svgSource: "skins/slim/design.svg"

  // tmp workaround for compositor:
  Timer {
    id: onCompletedTimer
    interval: 100
    repeat: false
    onTriggered: {
      update();
    }
  }
  Component.onCompleted: {
    onCompletedTimer.start();
  }

  Connections {
    target: Qt.application
    function onAboutToQuit() {
    //settings.setValue("SlimSkin/TopSplit", topSplit.height);
    }
  }

  MouseArea {
    anchors.fill: parent
    acceptedButtons: Qt.RightButton
    onClicked: {
      player.showContextMenu(Qt.point(mouseX + mainWindow.x - oldMainWindow.x, mouseY + mainWindow.y - oldMainWindow.y));
    }
  }

  NResizeHandler {
    target: mainWindow
    border: 6

    NBorder {
      size: 1
      color: "#646A73"
    }

    ColumnLayout {
      id: masterLayout
      spacing: 0
      anchors.fill: parent
      anchors.topMargin: 5
      anchors.bottomMargin: 6
      anchors.leftMargin: 6
      anchors.rightMargin: 6

      Item {
        id: titleBar
        Layout.preferredHeight: 17
        Layout.fillWidth: true
        NMoveHandler {
          target: mainWindow
        }
        RowLayout {
          anchors.fill: parent
          spacing: 0

          NSvgImage {
            Layout.fillHeight: true
            Layout.preferredWidth: 20
            source: svgSource
            elementId: "icon"
          }

          NFadeOut {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Text {
              id: textItem
              text: oldMainWindow.windowTitle
              color: "#D1D8DB"
              font.bold: true
              font.pixelSize: 12
              anchors.centerIn: parent.width >= textItem.width ? parent : undefined
              anchors.left: parent.width >= textItem.width ? undefined : parent.left
            }

            layer.enabled: true
            layer.effect: DropShadow {
              verticalOffset: 1
              color: "#363b42"
              radius: 0
              samples: 0
            }
          }

          NSvgButton {
            Layout.preferredWidth: 24
            Layout.preferredHeight: 16
            onClicked: mainWindow.showMinimized()
            source: svgSource
            elementIdNormal: "minimize-normal"
            elementIdHover: "minimize-hover"
            elementIdPress: "minimize-press"
          }
          NSvgButton {
            Layout.preferredWidth: 24
            Layout.preferredHeight: 16
            onClicked: mainWindow.close()
            source: svgSource
            elementIdNormal: "close-normal"
            elementIdHover: "close-hover"
            elementIdPress: "close-press"
          }
        }
      }

      Item {
        Layout.fillWidth: true
        Layout.preferredHeight: 4
      }

      SplitView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        orientation: Qt.Vertical

        handle: NSvgImage {
          Layout.fillHeight: true
          implicitHeight: 7
          source: svgSource
          elementId: "splitter"
        }

        RowLayout {
          id: topSplit
          SplitView.minimumHeight: 20
          SplitView.preferredHeight: settings.value("SlimSkin/Splitter")[0]

          //SplitView.preferredHeight: settings.value("SlimSkin/TopSplit")

          spacing: 0

          Rectangle {
            anchors.fill: parent
            color: "#3B4047"
          }

          NBorder {
            size: 1
            color: "#2c3034"
          }

          MouseArea {
            anchors.fill: parent
            onWheel: {
              if (wheel.angleDelta.y < 0) {
                volumeSlider.decrease();
              } else {
                volumeSlider.increase();
              }
              volumeSlider.moved();
            }
          }

          NSvgButton {
            Layout.preferredWidth: 27
            Layout.preferredHeight: 27
            source: svgSource
            elementIdNormal: "prev-normal"
            elementIdHover: "prev-hover"
            elementIdPress: "prev-press"
            //onClicked: playlistWidget.playPrevItem();
          }
          NSvgButton {
            Layout.preferredWidth: 27
            Layout.preferredHeight: 27
            source: svgSource
            elementIdNormal: playbackEngine.state == N.PlaybackPlaying ? "pause-normal" : "play-normal"
            elementIdHover: playbackEngine.state == N.PlaybackPlaying ? "pause-hover" : "play-hover"
            elementIdPress: playbackEngine.state == N.PlaybackPlaying ? "pause-press" : "play-press"
            onClicked: {
              if (playbackEngine.state == N.PlaybackPlaying) {
                playbackEngine.pause();
              } else {
                playbackEngine.play();
              }
            }
          }
          NSvgButton {
            Layout.preferredWidth: 27
            Layout.preferredHeight: 27
            source: svgSource
            elementIdNormal: "next-normal"
            elementIdHover: "next-hover"
            elementIdPress: "next-press"
            //onClicked: playlistWidget.playNextItem();
          }

          Item {
            id: waveformContainer
            Layout.fillHeight: true
            Layout.fillWidth: true

            property real position: playbackEngine.position

            Item {
              anchors.margins: 1
              anchors.fill: parent

              NWaveformBar {
                id: waveformBar
                anchors.fill: parent
                visible: false
              }

              MouseArea {
                anchors.fill: parent
                onPressed: {
                  let position = mouseX / parent.width;
                  playbackEngine.position = position;
                  // to avoid waiting for positionChanged signal:
                  playbackEngine.positionChanged(position);
                }
              }

              Item {
                id: leftSide
                anchors.fill: parent
                LinearGradient {
                  anchors {
                    top: parent.top
                    bottom: parent.bottom
                    left: parent.left
                  }
                  width: parent.width * waveformContainer.position
                  gradient: Gradient {
                    GradientStop {
                      position: 0.0
                      color: playbackEngine.state == N.PlaybackPlaying ? "#FFF657" : "#9BA0AA"
                    }
                    GradientStop {
                      position: 1.0
                      color: playbackEngine.state == N.PlaybackPlaying ? "#FFA519" : "#686E79"
                    }
                  }
                }
                visible: false
              }

              OpacityMask {
                anchors.fill: parent
                maskSource: waveformBar
                source: leftSide
              }

              Item {
                id: rightSide
                anchors.fill: parent
                LinearGradient {
                  anchors {
                    top: parent.top
                    bottom: parent.bottom
                    right: parent.right
                  }
                  width: parent.width * (1.0 - waveformContainer.position)
                  gradient: Gradient {
                    GradientStop {
                      position: 0.0
                      color: playbackEngine.state == N.PlaybackPlaying ? "#C0CACD" : "#6D717B"
                    }
                    GradientStop {
                      position: 1.0
                      color: playbackEngine.state == N.PlaybackPlaying ? "#919CA1" : "#545860"
                    }
                  }
                }
                visible: false
              }

              OpacityMask {
                anchors.fill: parent
                maskSource: waveformBar
                source: rightSide
              }

              Rectangle {
                id: groove
                height: parent.height
                width: 1
                color: playbackEngine.state == N.PlaybackPlaying ? "#FF7B00" : "#B0B5BF"
                x: parent.width * waveformContainer.position
              }

              NTrackInfo {
                anchors.margins: 2
                itemDelegate: Text {
                  color: "#D1D8DB"
                  leftPadding: 2
                  rightPadding: 2
                  font.bold: true
                  font.pixelSize: 11
                  text: itemText
                  Rectangle {
                    visible: itemText != ""
                    anchors.fill: parent
                    color: "#C8242B2F"
                    radius: 2
                    z: -1
                  }
                }
                onTooltipRequested: mainWindow.tooltipRequested(text)
              }
            }
          }

          Item {
            Layout.fillHeight: true
            Layout.preferredWidth: 7

            Slider {
              id: volumeSlider
              orientation: Qt.Vertical
              anchors.fill: parent
              anchors.margins: 1

              stepSize: 0.02
              value: playbackEngine.volume
              onMoved: {
                playbackEngine.volume = value;
                mainWindow.tooltipRequested(player.volumeTooltipText(value));
              }

              background: Rectangle {
                anchors.fill: parent
                color: parent.hovered || parent.pressed ? "#F0B000" : "#636970"

                Rectangle {
                  width: parent.width
                  height: volumeSlider.visualPosition * parent.height
                  color: "#3B4047"
                }
              }

              handle: Rectangle {
                x: 0
                y: volumeSlider.visualPosition * (parent.height - height)
                implicitWidth: parent.width
                implicitHeight: parent.width
                color: parent.hovered || parent.pressed ? "#ff7b00" : "#9C9EA0"
              }
            }
          }
        }

        NPlaylist {
          id: playlist
          model: playlistController.model()
          itemHeight: 20
          itemSpacing: -2
          dropIndicatorColor: "#b6c0c6"
          itemDelegate: Item {
            NFadeOut {
              anchors.fill: parent
              anchors.topMargin: 2
              anchors.leftMargin: 4
              Text {
                text: itemData.text
                font.bold: itemData.isCurrent
                font.pixelSize: 12
                color: itemData.isFailed ? "#555961" : (itemData.isFocused || itemData.isSelected ? (itemData.isCurrent ? "#FFFFFF" : "#e9f2f7") : (itemData.isCurrent ? "#D6DFE3" : "#B6C0C6"))
              }
            }

            Item {
              anchors.fill: parent
              anchors.margins: 1
              z: -1
              Rectangle {
                anchors.fill: parent
                visible: itemData.isSelected
                color: "#3E484F"
                opacity: 0.7
              }

              Rectangle {
                anchors.fill: parent
                visible: itemData.isHovered
                color: "#3e5268"
                opacity: 0.7
              }

              Rectangle {
                anchors.fill: parent
                visible: itemData.isFocused
                color: "transparent"
                border.color: "#6D7E8A"
                border.width: 1

                Rectangle {
                  anchors.fill: parent
                  visible: itemData.isFocused
                  color: "#315063"
                  opacity: 0.3
                }
              }
            }
          }

          scrollbarWidth: 6
          scrollbarPadding: 0
          scrollbarContentItem: Rectangle {
            color: (scrollbarHoverHandler.hovered || scrollbar.pressed) ? "#E0A500" : "#636970"
            HoverHandler {
              id: scrollbarHoverHandler
              acceptedDevices: PointerDevice.Mouse
            }
          }

          NBorder {
            size: 1
            color: "#13181C"
          }

          Rectangle {
            anchors.fill: parent
            color: "#242B2F"
            z: -1
          }
        }
      }
    }
  }
}
