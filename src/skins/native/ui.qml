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
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.4
import QtGraphicalEffects 1.0
import QtQuick.Window 2.2

import Nulloy 1.0
import NSvgImage 1.0

Rectangle {
  SystemPalette {
    id: systemPalette
  }

  color: systemPalette.base
  Layout.minimumWidth: 470

  property string svgSource: "design.svg"
  property string settingsPrefix: "NativeSkin/"

  Component.onCompleted: {
    mainWindow.flags = 0;
  }

  component SvgButton: Button {
    id: button
    property alias source: image.source
    property alias elementId: image.elementId
    NSvgImage {
      id: image
      anchors.fill: parent
      anchors.topMargin: button.pressed ? 2 : 0
      colorOverlay: button.checked ? systemPalette.highlight : systemPalette.buttonText
    }
  }

  Connections {
    target: Qt.application
    function onAboutToQuit() {
      NSettings.setValue(settingsPrefix + "Splitter", splitter.states);
    }
  }

  NSizeGrip {
    parent: Window.contentItem
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    width: 18
    height: 18
    NSvgImage {
      anchors.right: parent.right
      anchors.bottom: parent.bottom
      anchors.margins: 2
      width: 10
      height: 10
      source: svgSource
      elementId: "sizegrip"
      colorOverlay: systemPalette.text
    }
  }

  NSplitter {
    id: splitter
    anchors.fill: parent

    states: NSettings.value(settingsPrefix + "Splitter", [150, 1])

    handleDelegate: Rectangle {
      height: 5
      color: systemPalette.window
      NSvgImage {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        implicitHeight: 4
        source: svgSource
        elementId: "splitter"
        colorOverlay: systemPalette.text
      }
      Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: systemPalette.shadow
      }
    }

    Item {
      z: 1

      Rectangle {
        anchors.fill: parent
        color: systemPalette.window
      }

      MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.NoButton
        onWheel: {
          if (wheel.angleDelta.y < 0) {
            volumeSlider.value -= 0.02;
          } else {
            volumeSlider.value += 0.02;
          }
        }
      }

      ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 10
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 0

        RowLayout {
          Layout.fillHeight: true
          Layout.fillWidth: true
          Layout.minimumHeight: 50
          spacing: 5

          NCoverImage {
            Layout.fillHeight: true
            growHorizontally: true
            states: NSettings.value(settingsPrefix + "Splitter", [200, 200])

            Rectangle {
              anchors.fill: parent
              border.color: systemPalette.highlight
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
              opacity: 0.0

              waveform.color: "#4E9A06"
              waveform.borderColor: "#8AE234"
              waveform.borderWidth: 1.0 / Screen.devicePixelRatio

              grooveDelegate: Item {}

              dropAreaDelegate: Rectangle {
                color: Qt.lighter(systemPalette.highlight, 1.5)
                opacity: 0.5
              }

              Rectangle {
                anchors.fill: parent
                color: "#3F4F61"
              }

              Item {
                id: leftSide
                anchors.fill: parent
                Rectangle {
                  anchors {
                    top: parent.top
                    bottom: parent.bottom
                    left: parent.left
                  }
                  width: parent.width * NPlaybackEngine.position
                  color: NPlaybackEngine.state == N.PlaybackPlaying ? "#0080FF" : "#CE8419"
                }
              }

              Blend {
                anchors.fill: parent
                source: waveformSlider.waveform
                foregroundSource: leftSide
                mode: "hardLight"
              }
            }

            Rectangle {
              id: waveformRoundedMask
              anchors.fill: parent
              color: "white"
              radius: 3
              visible: false
            }

            OpacityMask {
              anchors.fill: parent
              maskSource: waveformRoundedMask
              source: waveformSlider
            }

            NTrackInfoView {
              anchors.margins: 2

              itemDelegate: Text {
                id: delegate
                color: "#FFFFFF"
                leftPadding: 3
                rightPadding: 3
                font.bold: true
                font.pixelSize: (rowIndex == 1 && columnIndex == 1) ? 12 : 11
                Rectangle {
                  parent: delegate.parent.parent
                  anchors.fill: delegate
                  color: "#000000"
                  opacity: 0.7
                  radius: 3
                  z: -1
                }
              }
              onTooltipRequested: NPlayer.showToolTip(text)
            }
          }
        }

        Item {
          Layout.minimumHeight: 9
        }

        RowLayout {
          id: playbackControls
          Layout.fillWidth: true
          Layout.minimumHeight: 30
          Layout.maximumHeight: 30

          visible: NSettings.value("ShowPlaybackControls")
          Connections {
            target: NSettings
            function onValueChanged(key, value) {
              if (key === "ShowPlaybackControls") {
                playbackControls.visible = value;
              }
            }
          }

          SvgButton {
            Layout.preferredWidth: 50
            Layout.fillHeight: true
            onClicked: NPlaylistController.playPrevRow()
            source: svgSource
            elementId: "prev"
          }

          SvgButton {
            Layout.preferredWidth: 50
            Layout.fillHeight: true
            onClicked: {
              if (NPlaybackEngine.state == N.PlaybackPlaying) {
                NPlaybackEngine.pause();
              } else {
                NPlaybackEngine.play();
              }
            }
            source: svgSource
            elementId: NPlaybackEngine.state == N.PlaybackPlaying ? "pause" : "play"
          }

          SvgButton {
            Layout.preferredWidth: 50
            Layout.fillHeight: true
            onClicked: NPlaybackEngine.stop()
            source: svgSource
            elementId: "stop"
          }

          SvgButton {
            Layout.preferredWidth: 50
            Layout.fillHeight: true
            onClicked: NPlaylistController.playNextRow()
            source: svgSource
            elementId: "next"
          }

          Item {
            Layout.maximumWidth: 25
            Layout.minimumWidth: 2
            Layout.fillWidth: true
            //Layout.horizontalStretchFactor: 2 // Qt6 only
          }

          RowLayout {
            Layout.maximumHeight: 25

            SvgButton {
              id: repeatButton
              Layout.preferredWidth: 30
              Layout.fillHeight: true
              source: svgSource
              elementId: "repeat"
              checkable: true
              checked: NPlaylistController.repeatMode
              onClicked: NPlaylistController.repeatMode = checked
            }

            SvgButton {
              Layout.preferredWidth: 30
              Layout.fillHeight: true
              onClicked: NPlaylistController.shuffleRows()
              source: svgSource
              elementId: "shuffle"
            }
          }

          Item {
            Layout.minimumWidth: 15
            Layout.fillWidth: true
          }

          Item {
            Layout.preferredHeight: 20
            Layout.preferredWidth: 120

            Slider {
              id: volumeSlider
              orientation: Qt.Horizontal
              anchors.fill: parent
              stepSize: 0.01
              value: NPlaybackEngine.volume

              onValueChanged: {
                NPlaybackEngine.volume = value;
                NPlayer.showToolTip(NPlayer.volumeTooltipText(value));
              }
            }
          }
        }

        Item {
          Layout.minimumHeight: 5
          Layout.maximumHeight: 5
        }
      }
    }

    NPlaylist {
      anchors.topMargin: 3
      Layout.minimumHeight: 80

      itemHeight: 20
      itemDelegate: Item {
        property bool isAlternate: itemData.index % 2 == 1

        Item {
          id: itemBackground
          anchors.fill: parent
          anchors.margins: 1

          Rectangle {
            anchors.fill: parent
            visible: isAlternate
            color: systemPalette.alternateBase
          }

          Rectangle {
            anchors.fill: parent
            visible: itemData.isSelected
            color: systemPalette.highlight
          }

          Rectangle {
            anchors.fill: parent
            visible: itemData.isHovered
            anchors.margins: 1
            color: systemPalette.shadow
            opacity: 0.3
          }

          Rectangle {
            anchors.fill: parent
            visible: itemData.isFocused
            radius: 2
            opacity: 0.5
            color: "transparent"
            border.color: Qt.lighter(systemPalette.highlight, 1.5)

            Rectangle {
              anchors.fill: parent
              opacity: 0.3
              gradient: Gradient {
                GradientStop {
                  position: 0.0
                  color: Qt.lighter(systemPalette.highlightedText, 1.5)
                }
                GradientStop {
                  position: 1.0
                  color: "transparent"
                }
              }
            }
          }
        }

        Text {
          anchors.topMargin: 2
          anchors.leftMargin: 4
          text: itemData.text
          font.bold: itemData.isPlaying
          font.pixelSize: 13
          color: itemData.isFailed ? "#ff0000" : (itemData.isSelected ? systemPalette.highlightedText : systemPalette.text)
        }
      }

      dropIndicatorColor: systemPalette.text
      dropAreaDelegate: Rectangle {
        anchors.fill: parent
        anchors.topMargin: 1
        anchors.bottomMargin: 2
        color: Qt.lighter(systemPalette.highlight, 1.5)
        opacity: 0.5
      }

      scrollbarPadding: 0
      scrollbarBottomPadding: 15
      scrollbarContentItem: Rectangle {
        color: "transparent"
        border.color: systemPalette.text
        opacity: 0.2
        radius: 2
        Rectangle {
          color: systemPalette.text
          anchors.fill: parent
          opacity: (scrollbarHoverHandler.hovered || scrollbar.pressed) ? 0.5 : 0.3
        }
        HoverHandler {
          id: scrollbarHoverHandler
          acceptedDevices: PointerDevice.Mouse
        }
      }
    }
  }
}
