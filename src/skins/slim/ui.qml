/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2024 Sergey Vlasov <sergey@vlasov.me>
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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.0

import Nulloy 1.0
import NSvgImage 1.0

Rectangle {
  color: "#4F545B"

  Rectangle {
    anchors.fill: parent
    color: "transparent"
    border.color: "#646A73"
    z: 1
  }

  Layout.minimumWidth: 300

  property string svgSource: "design.svg"
  property string settingsPrefix: "SlimSkin/"
  property color textColor: "#B6C0C6"
  property color textFailedColor: "#555961"
  property color textPlayingColor: "#D6DFE3"
  property color textSelectedColor: "#E9F2F7"
  property color textSelectedPlayingColor: "#FFFFFF"
  property color dropAreaBorderColor: "#E0A500"
  property color dropAreaFillColor: "#32E0A500"
  property color scrollbarColor: "#636970"
  property color scrollbarActiveColor: "#E0A500"

  Connections {
    target: Qt.application
    function onAboutToQuit() {
    //NSettings.setValue(settingsPrefix + "Splitter", splitter.states);
    }
  }

  ColumnLayout {
    anchors.fill: parent
    spacing: 0

    RowLayout {
      id: titleBar
      Layout.topMargin: 4
      Layout.leftMargin: 6
      Layout.rightMargin: 6
      Layout.minimumHeight: 16
      Layout.fillWidth: true
      Layout.fillHeight: false
      spacing: 8

      NSvgImage {
        Layout.fillHeight: true
        Layout.preferredWidth: 18
        source: svgSource
        elementId: "icon"
      }

      NWindowTitle {
        Layout.fillWidth: true
        Layout.fillHeight: true

        itemDelegate: Text {
          color: "#D1D8DB"
          font.bold: true
          font.pixelSize: 12
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
        Layout.preferredWidth: 20
        Layout.preferredHeight: 18
        onClicked: mainWindow.showMinimized()
        source: svgSource
        elementId: "minimize"
        elementIdHovered: "minimize-hover"
        elementIdPressed: "minimize-press"
      }

      NSvgButton {
        Layout.preferredWidth: 20
        Layout.preferredHeight: 18
        onClicked: mainWindow.close()
        source: svgSource
        elementId: "close"
        elementIdHovered: "close-hover"
        elementIdPressed: "close-press"
      }
    }

    NSplitter {
      id: splitter
      Layout.fillWidth: true
      Layout.fillHeight: true

      // FIXME: temporal adjustment for compatibility with the old skin:
      states: [parseInt(NSettings.value(settingsPrefix + "Splitter")[0]) + 4]

      handleDelegate: NSvgImage {
        height: 7
        source: svgSource
        elementId: "splitter"
      }

      Item {
        MouseArea {
          anchors.fill: parent
          acceptedButtons: Qt.NoButton
          onWheel: {
            if (wheel.angleDelta.y < 0) {
              volumeSlider.decrease();
            } else {
              volumeSlider.increase();
            }
            volumeSlider.moved();
          }
        }

        ColumnLayout {
          anchors.fill: parent
          anchors.topMargin: 4
          anchors.leftMargin: 6
          anchors.rightMargin: 6
          spacing: 5

          Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumHeight: 30

            Rectangle {
              anchors.fill: parent
              color: "#3B4047"
              border.color: "#2c3034"
            }

            RowLayout {
              anchors.fill: parent
              spacing: 0

              NSvgButton {
                Layout.preferredWidth: 27
                Layout.preferredHeight: 27
                source: svgSource
                elementId: "prev"
                elementIdHovered: "prev-hover"
                elementIdPressed: "prev-press"
                onClicked: NPlaylistController.playPrevRow()
              }
              NSvgButton {
                Layout.preferredWidth: 27
                Layout.preferredHeight: 27
                source: svgSource
                elementId: NPlaybackEngine.state == N.PlaybackPlaying ? "pause" : "play"
                elementIdHovered: NPlaybackEngine.state == N.PlaybackPlaying ? "pause-hover" : "play-hover"
                elementIdPressed: NPlaybackEngine.state == N.PlaybackPlaying ? "pause-press" : "play-press"
                onClicked: {
                  if (NPlaybackEngine.state == N.PlaybackPlaying) {
                    NPlaybackEngine.pause();
                  } else {
                    NPlaybackEngine.play();
                  }
                }
              }
              NSvgButton {
                Layout.preferredWidth: 27
                Layout.preferredHeight: 27
                source: svgSource
                elementId: "next"
                elementIdHovered: "next-hover"
                elementIdPressed: "next-press"
                onClicked: NPlaylistController.playNextRow()
              }

              NCoverImage {
                margin: 1
                Layout.fillHeight: true
                growHorizontally: true

                Rectangle {
                  anchors.fill: parent
                  anchors.margins: 1
                  border.color: "#FFA519"
                  visible: parent.containsMouse
                  color: "transparent"
                }
              }

              Item {
                Layout.fillHeight: true
                Layout.fillWidth: true

                NWaveformSlider {
                  id: waveformSlider
                  anchors.fill: parent
                  anchors.topMargin: 1
                  anchors.bottomMargin: 1

                  grooveDelegate: Rectangle {
                    width: 1
                    color: NPlaybackEngine.state == N.PlaybackPlaying ? "#FF7B00" : "#B0B5BF"
                  }

                  dropAreaDelegate: Rectangle {
                    color: dropAreaFillColor
                    border.color: dropAreaBorderColor
                    anchors.fill: parent
                    anchors.topMargin: 3
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
                      width: parent.width * NPlaybackEngine.position
                      gradient: Gradient {
                        GradientStop {
                          position: 0.0
                          color: NPlaybackEngine.state == N.PlaybackPlaying ? "#FFF657" : "#9BA0AA"
                        }
                        GradientStop {
                          position: 1.0
                          color: NPlaybackEngine.state == N.PlaybackPlaying ? "#FFA519" : "#686E79"
                        }
                      }
                    }
                    visible: false
                  }

                  OpacityMask {
                    anchors.fill: parent
                    maskSource: waveformSlider.waveform
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
                      width: parent.width * (1.0 - NPlaybackEngine.position)
                      gradient: Gradient {
                        GradientStop {
                          position: 0.0
                          color: NPlaybackEngine.state == N.PlaybackPlaying ? "#C0CACD" : "#6D717B"
                        }
                        GradientStop {
                          position: 1.0
                          color: NPlaybackEngine.state == N.PlaybackPlaying ? "#919CA1" : "#545860"
                        }
                      }
                    }
                    visible: false
                  }

                  OpacityMask {
                    anchors.fill: parent
                    maskSource: waveformSlider.waveform
                    source: rightSide
                  }
                }

                NTrackInfoView {
                  anchors.margins: 2

                  itemDelegate: Text {
                    id: delegate
                    color: "#D1D8DB"
                    leftPadding: 2
                    rightPadding: 2
                    font.bold: true
                    font.pixelSize: (rowIndex == 1 && columnIndex == 1) ? 12 : 11
                    Rectangle {
                      parent: delegate.parent.parent
                      anchors.fill: delegate
                      color: "#C8242B2F"
                      radius: 2
                      z: -1
                    }
                  }
                  onTooltipRequested: NPlayer.showToolTip(text)
                }
              }

              Item {
                Layout.fillHeight: true
                Layout.preferredWidth: 7

                NVolumeSlider {
                  id: volumeSlider
                  orientation: Qt.Vertical
                  anchors.fill: parent
                  anchors.margins: 1

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
                    width: parent.width
                    height: parent.width
                    color: parent.hovered || parent.pressed ? "#ff7b00" : "#9C9EA0"
                  }
                }
              }
            }
          }
        }
      }

      NPlaylist {
        Layout.minimumHeight: 80
        anchors.margins: 6
        anchors.topMargin: 0
        clip: true

        itemHeight: 20
        itemSpacing: -2
        itemDelegate: Item {
          Item {
            anchors.fill: parent
            anchors.margins: 1

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

              Rectangle {
                anchors.fill: parent
                anchors.margins: 1
                visible: itemData.isFocused
                color: "#315063"
                opacity: 0.3
              }
            }
          }

          Text {
            anchors.topMargin: 2
            anchors.leftMargin: 4
            text: itemData.text
            font.bold: itemData.isPlaying
            font.pixelSize: 12
            color: itemData.isFailed ? textFailedColor : (itemData.isFocused || itemData.isSelected ? (itemData.isPlaying ? textSelectedPlayingColor : textSelectedColor) : (itemData.isPlaying ? textPlayingColor : textColor))
          }
        }

        dropIndicatorColor: textColor
        dropAreaDelegate: Rectangle {
          color: dropAreaFillColor
          border.color: dropAreaBorderColor
          anchors.fill: parent
          anchors.margins: 1
        }

        scrollbarWidth: 7
        scrollbarPadding: 1
        scrollbarSpacing: -1
        scrollbarContentItem: Rectangle {
          color: (scrollbarHoverHandler.hovered || scrollbar.pressed) ? scrollbarActiveColor : scrollbarColor
          HoverHandler {
            id: scrollbarHoverHandler
            acceptedDevices: PointerDevice.Mouse
          }
        }

        Rectangle {
          anchors.fill: parent
          anchors.margins: 6
          anchors.topMargin: 0
          color: "#242B2F"
          z: -1
        }
        Rectangle {
          anchors.fill: parent
          anchors.margins: 6
          anchors.topMargin: 0
          color: "transparent"
          border.color: "#13181C"
          z: 1
        }
        Rectangle {
          anchors.bottom: parent.bottom
          anchors.left: parent.left
          anchors.right: parent.right
          anchors.margins: 1
          height: 5
          color: "#4F545B"
          z: 1
        }
      }
    }
  }
}
