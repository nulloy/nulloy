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

Item {
  property alias model: listView.model
  property alias itemSpacing: listView.spacing
  property color dropIndicatorColor: "#000000"
  property int itemHeight: 20
  property int scrollbarWidth: 14
  property int scrollbarPadding: 3
  property int scrollbarLeftPadding: -1
  property int scrollbarRightPadding: -1
  property int scrollbarTopPadding: -1
  property int scrollbarBottomPadding: -1
  property alias scrollbarSpacing: rowLayout.spacing

  property int dragScrollMargin: 20
  property real dragScrollFactor: 0.7
  property real dragScrollSpeed: 0
  property real dragScrollSpeedMin: 1
  property real dragScrollSpeedMax: 40

  function resetContent() {
    // FIXME: large model changes messes up contentY, ListView bug?
    var model = listView.model;
    var contentY = listView.contentY;
    listView.model = null;
    listView.model = model;
    listView.contentY = contentY;
  }

  property Component scrollbarContentItem: Rectangle {
    color: "dimGray"
    radius: 5
  }

  property Component itemDelegate: Text {
    text: itemData.text
    font.bold: itemData.isPlaying
  }

  property Component dropAreaDelegate: Rectangle {
    anchors.fill: parent
    color: "#0069E032"
  }

  function updatefirstVisibleRow() {
    NPlaylistController.firstVisibleRow = Math.max(0, Math.floor(listView.contentY / (itemHeight + itemSpacing)));
  }

  function updateRowsPerPage() {
    NPlaylistController.rowsPerPage = Math.floor(listView.height / (itemHeight + itemSpacing));
  }
  onHeightChanged: updateRowsPerPage()

  Component.onCompleted: {
    updateRowsPerPage();
  }

  Connections {
    target: model

    function onPlayingRowChanged() {
      if (NSettings.value("ScrollToItem")) {
        var row = model.playingRow();
        if (row >= 0) {
          listView.positionViewAtIndex(row, ListView.Contain);
        }
      }
    }

    function onFocusedRowChanged() {
      var row = model.focusedRow();
      if (row >= 0) {
        listView.positionViewAtIndex(row, ListView.Contain);
      }
    }

    function onCleared() {
      resetContent();
    }
  }

  onDropIndicatorColorChanged: {
    dropMediaHereCanvas.requestPaint();
  }

  property int _hoveredRow: -1
  property int _dropRow: 0

  RowLayout {
    id: rowLayout
    anchors.fill: parent
    anchors.topMargin: parent.anchors.topMargin
    anchors.bottomMargin: parent.anchors.bottomMargin
    anchors.leftMargin: parent.anchors.leftMargin
    anchors.rightMargin: parent.anchors.rightMargin
    spacing: 0

    ListView {
      id: listView
      Layout.fillWidth: true
      Layout.fillHeight: true
      ScrollBar.vertical: scrollBar
      interactive: false
      model: NPlaylistController.model()

      delegate: Item {
        width: listView.width
        height: itemHeight
        Loader {
          anchors.fill: parent
          sourceComponent: itemDelegate
          property var itemData: Object.assign({}, model) // copy the model, QTBUG-82989 workaround
          onLoaded: {
            fadeOut.stealTextItem(item);
          }
        }

        NFadeOut {
          id: fadeOut
          anchors.fill: parent
        }
      }

      Rectangle {
        id: dragTarget
        width: parent.width
        visible: false
      }

      Rectangle {
        width: parent.width
        height: 1
        color: dropIndicatorColor
        visible: dropArea.containsDrag && listView.count > 0
        y: (itemHeight + itemSpacing) * _dropRow + 1 - listView.contentY
      }

      focus: true
      Keys.onPressed: event => {
        NPlaylistController.keyPress(event.key, event.modifiers);
      }
      Keys.onReleased: event => {
        NPlaylistController.keyRelease(event.key, event.modifiers);
      }

      onContentYChanged: {
        updatefirstVisibleRow();
        mouseArea.updateHoveredRow();
      }

      MouseArea {
        id: mouseArea
        anchors.fill: parent
        scrollGestureEnabled: false
        acceptedButtons: Qt.AllButtons
        propagateComposedEvents: true
        hoverEnabled: true

        onWheel: {
          listView.focus = true;
          const speed = 0.4;
          const newY = listView.contentY - wheel.angleDelta.y * speed;
          listView.contentY = Math.max(0, Math.min(newY, listView.contentHeight - listView.height));
        }

        onPressed: {
          updateHoveredRow();
          listView.focus = true;
          if (mouse.button == Qt.RightButton) {
            NPlaylistController.contextMenuRequested(mapToGlobal(mouse.x, mouse.y));
          } else {
            NPlaylistController.mousePress(_hoveredRow, mouse.modifiers);
          }
        }

        onReleased: {
          updateHoveredRow();
          NPlaylistController.mouseRelease(_hoveredRow, mouse.modifiers);
        }

        onDoubleClicked: {
          updateHoveredRow();
          NPlaylistController.mouseDoubleClick(_hoveredRow);
        }

        onPositionChanged: {
          updateHoveredRow();
        }

        onEntered: {
          updateHoveredRow();
        }

        onExited: {
          updateHoveredRow();
        }

        function updateHoveredRow() {
          NPlaylistController.mouseExit(_hoveredRow);
          let newHoveredRow = containsMouse ? listView.indexAt(mouseX, listView.contentY + mouseY) : -1;
          NPlaylistController.mouseEnter(newHoveredRow);
          _hoveredRow = newHoveredRow;
        }

        Drag.active: dragHandler.active
        Drag.dragType: Drag.Automatic
        Drag.supportedActions: Qt.CopyAction | Qt.MoveAction
        Drag.onDragFinished: dropAction => {
          if (dropAction == Qt.MoveAction) {
            NPlaylistController.removeFiles(NPlaylistController.selectedFiles());
          }
        }
        DragHandler {
          id: dragHandler
          dragThreshold: 2
          enabled: _hoveredRow >= 0
          onActiveChanged: {
            if (active) {
              mouseArea.Drag.mimeData = {
                "text/uri-list": NPlaylistController.selectedFiles().map(file => NUtils.pathToUri(file)).join("\n")
              };
            }
          }
        }

        DropArea {
          id: dropArea
          anchors.fill: parent
          keys: ["text/uri-list"]

          onDropped: {
            if (drop.source == mouseArea) {
              NPlaylistController.moveSelected(_dropRow);
            } else {
              NPlaylistController.dropUrls(_dropRow, drop.urls);
            }
            dragScrollSpeed = 0;
            resetContent();
          }

          function dropRowRefresh() {
            _dropRow = listView.indexAt(drag.x, listView.contentY + drag.y);
            if (_dropRow == -1) {
              _dropRow = listView.count;
              return;
            }
            let yOffset = drag.y + listView.contentY - listView.itemAtIndex(_dropRow).y - itemHeight / 2;
            if (yOffset > 0) {
              ++_dropRow;
            }
          }

          onPositionChanged: {
            if (drag.y < dragScrollMargin && listView.contentY > 0) {
              dragScrollSpeed = -Math.min((dragScrollMargin - drag.y) * dragScrollFactor + dragScrollSpeedMin, dragScrollSpeedMax);
            } else if (drag.y > listView.height - dragScrollMargin && listView.contentY < listView.contentHeight - listView.height) {
              dragScrollSpeed = Math.min((drag.y - (listView.height - dragScrollMargin)) * dragScrollFactor + dragScrollSpeedMin, dragScrollSpeedMax);
            } else {
              dragScrollSpeed = 0;
            }
            if (dragScrollSpeed !== 0 && !dragScrollAnimation.running) {
              dragScrollAnimation.start();
            }
            dropRowRefresh();
          }

          onExited: {
            dragScrollSpeed = 0;
          }

          NumberAnimation {
            id: dragScrollAnimation
            target: listView
            property: "contentY"
            duration: 40
            to: {
              let y = listView.contentY + dragScrollSpeed;
              if (dragScrollSpeed < 0) {
                return Math.max(0, y);
              }
              if (dragScrollSpeed > 0) {
                return Math.min(y, listView.contentHeight - listView.height);
              }
              return listView.contentY;
            }
            onStopped: {
              if (dragScrollSpeed !== 0) {
                start();
              }
            }
          }
        }
      }

      ColumnLayout {
        visible: listView.count == 0
        anchors.centerIn: parent

        Canvas {
          id: dropMediaHereCanvas
          Layout.preferredWidth: 120
          Layout.preferredHeight: 120

          onPaint: {
            let ctx = getContext("2d");
            ctx.reset();

            // border:
            {
              ctx.translate(0, 0);
              ctx.strokeStyle = dropIndicatorColor;
              ctx.lineWidth = 4.3;
              ctx.lineDashOffset = 6;
              ctx.setLineDash([3.7, 2.6]);
              const radius = 20;
              const x = ctx.lineWidth / 2;
              const y = ctx.lineWidth / 2;
              const width = this.width - ctx.lineWidth;
              const height = this.height - ctx.lineWidth;
              ctx.beginPath();
              ctx.moveTo(x + radius, y);
              ctx.lineTo(x + width - radius, y);
              ctx.quadraticCurveTo(x + width, y, x + width, y + radius);
              ctx.lineTo(x + width, y + height - radius);
              ctx.quadraticCurveTo(x + width, y + height, x + width - radius, y + height);
              ctx.lineTo(x + radius, y + height);
              ctx.quadraticCurveTo(x, y + height, x, y + height - radius);
              ctx.lineTo(x, y + radius);
              ctx.quadraticCurveTo(x, y, x + radius, y);
              ctx.closePath();
              ctx.stroke();
            }

            // arrow:
            {
              const points = [
                {
                  x: 33,
                  y: 78
                },
                {
                  x: 66,
                  y: 39
                },
                {
                  x: 51,
                  y: 39
                },
                {
                  x: 51,
                  y: 0
                },
                {
                  x: 15,
                  y: 0
                },
                {
                  x: 15,
                  y: 39
                },
                {
                  x: 0,
                  y: 39
                }
              ];
              ctx.beginPath();
              ctx.translate((this.width - Math.max(...points.map(p => p.x))) / 2, (this.height - Math.max(...points.map(p => p.y))) / 2);
              ctx.moveTo(points[0].x, points[0].y);
              for (let i = 1; i < points.length; ++i) {
                ctx.lineTo(points[i].x, points[i].y);
              }
              ctx.closePath();
              ctx.fillStyle = dropIndicatorColor;
              ctx.fill();
            }
          }
        }

        Text {
          Layout.alignment: Qt.AlignHCenter
          color: dropIndicatorColor
          text: qsTr("Drop media here")
        }
      }

      Loader {
        sourceComponent: dropAreaDelegate
        visible: dropArea.containsDrag && listView.count == 0
        anchors.fill: parent
      }
    }

    ScrollBar {
      id: scrollBar
      Layout.fillHeight: true
      Layout.preferredWidth: scrollbarWidth
      padding: scrollbarPadding > -1 ? scrollbarPadding : undefined
      leftPadding: scrollbarLeftPadding > -1 ? scrollbarLeftPadding : undefined
      topPadding: scrollbarTopPadding > -1 ? scrollbarTopPadding : undefined
      bottomPadding: scrollbarBottomPadding > -1 ? scrollbarBottomPadding : undefined
      rightPadding: scrollbarRightPadding > -1 ? scrollbarRightPadding : undefined
      active: hovered || pressed
      orientation: Qt.Vertical
      policy: ScrollBar.AlwaysOn
      visible: listView.contentHeight > listView.height
      size: listView.height / listView.contentHeight
      contentItem: Loader {
        sourceComponent: scrollbarContentItem
        property var scrollbar: scrollBar
      }
    }
  }
}
