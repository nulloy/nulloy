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

import "utils.js" as NUtilsJS

ApplicationWindow {
  id: mainWindow
  visible: true
  flags: Qt.FramelessWindowHint
  title: _oldMainWindow.windowTitle

  property int resizerWidth: 5

  // FIXME: tmp workaround for compositor:
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

  MouseArea {
    anchors.fill: parent
    acceptedButtons: Qt.RightButton | Qt.LeftButton
    onPressed: {
      if (mouse.button == Qt.RightButton) {
        NPlayer.showContextMenu(Qt.point(mouseX + mainWindow.x - _oldMainWindow.x, mouseY + mainWindow.y - _oldMainWindow.y));
      } else {
        mainWindow.startSystemMove();
      }
    }
  }

  Repeater {
    model: _actionsList.length
    delegate: Item {
      Shortcut {
        enabled: _actionsList[index].enabled
        sequences: _actionsList[index].shortcuts()
        onActivated: _actionsList[index].trigger()
      }
    }
  }

  NResizeHandler {
    id: topRightResizer
    anchors.top: parent.top
    anchors.right: parent.right
    width: resizerWidth
    height: resizerWidth
    cursorShape: Qt.SizeBDiagCursor
    resizeFlags: Qt.TopEdge | Qt.RightEdge
  }

  NResizeHandler {
    id: topLeftResizer
    anchors.top: parent.top
    anchors.left: parent.left
    width: resizerWidth
    height: resizerWidth
    cursorShape: Qt.SizeFDiagCursor
    resizeFlags: Qt.TopEdge | Qt.LeftEdge
  }

  NResizeHandler {
    id: bottomRightResizer
    anchors.bottom: parent.bottom
    anchors.right: parent.right
    width: resizerWidth
    height: resizerWidth
    cursorShape: Qt.SizeFDiagCursor
    resizeFlags: Qt.BottomEdge | Qt.RightEdge
  }

  NResizeHandler {
    id: bottomLeftResizer
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    width: resizerWidth
    height: resizerWidth
    cursorShape: Qt.SizeBDiagCursor
    resizeFlags: Qt.BottomEdge | Qt.LeftEdge
  }

  NResizeHandler {
    anchors.top: parent.top
    anchors.left: topLeftResizer.right
    anchors.right: topRightResizer.left
    height: resizerWidth
    cursorShape: Qt.SizeVerCursor
    resizeFlags: Qt.TopEdge
  }

  NResizeHandler {
    anchors.bottom: parent.bottom
    anchors.left: bottomLeftResizer.right
    anchors.right: bottomRightResizer.left
    height: resizerWidth
    cursorShape: Qt.SizeVerCursor
    resizeFlags: Qt.BottomEdge
  }

  NResizeHandler {
    anchors.top: topRightResizer.bottom
    anchors.bottom: bottomRightResizer.top
    anchors.left: parent.left
    width: resizerWidth
    cursorShape: Qt.SizeHorCursor
    resizeFlags: Qt.LeftEdge
  }

  NResizeHandler {
    anchors.top: topLeftResizer.bottom
    anchors.bottom: bottomLeftResizer.top
    anchors.right: parent.right
    width: resizerWidth
    cursorShape: Qt.SizeHorCursor
    resizeFlags: Qt.RightEdge
  }

  FocusScope {
    id: focusScope
    anchors.fill: parent
    focus: true
    Component.onCompleted: {
      var component = Qt.createQmlObject(NSkinFileSystem.readFile("ui.qml"), focusScope);
      component.anchors.fill = component.parent;
      minimumWidth = NUtilsJS.calculateMinimumWidth(component);
      minimumHeight = NUtilsJS.calculateMinimumHeight(component);
    }
  }
}
