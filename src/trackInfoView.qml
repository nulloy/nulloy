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
import QtQuick.Layouts 1.15

Item {
  id: root
  anchors.fill: parent

  signal tooltipRequested(string text)

  property Component itemDelegate: Text {
    text: itemText
  }

  component DelegateLoader: Loader {
    id: loader
    sourceComponent: itemDelegate
    property int rowIndex
    property int columnIndex
    property string itemText: NTrackInfoModel[NTrackInfoModel.keys()[rowIndex * 3 + columnIndex]]

    visible: itemText != ""
    onItemTextChanged: {
      item.text = itemText;
    }

    onLoaded: {
      if (!(item instanceof Text)) {
        throw new Error("itemDelegate does not derive from Text");
      }
      fadeOut.stealTextItem(item);
    }
    NFadeOut {
      id: fadeOut
      anchors.fill: loader
    }
  }

  component RowComponent: Row {
    property int index

    property var widths: {
      var width0 = item0.visible ? item0.implicitWidth : 0;
      var width1 = item1.visible ? item1.implicitWidth : 0;
      var width2 = item2.visible ? item2.implicitWidth : 0;
      var total = width0 + width1 + width2;
      if (width >= total) {
        return [width0, width1, width2];
      }
      var scale = width / total;
      return [width0 * scale, width1 * scale, width2 * scale];
    }
    property int slack: width - (widths[0] + widths[1] + widths[2])
    property int leftSpacer: slack > 0 ? Math.max(0, (width - widths[1]) / 2 - widths[0]) : 0
    property int rightSpacer: slack > 0 ? Math.max(0, width - widths[2] - (leftSpacer + widths[0] + widths[1])) : 0

    DelegateLoader {
      id: item0
      rowIndex: index
      columnIndex: 0
      width: widths[columnIndex]
    }
    Item {
      width: leftSpacer
      height: 1
    }
    DelegateLoader {
      id: item1
      rowIndex: index
      columnIndex: 1
      width: widths[columnIndex]
    }
    Item {
      width: rightSpacer
      height: 1
    }
    DelegateLoader {
      id: item2
      rowIndex: index
      columnIndex: 2
      width: widths[columnIndex]
    }
  }

  Item {
    anchors.fill: parent

    RowComponent {
      anchors.top: parent.top
      anchors.left: parent.left
      anchors.right: parent.right
      width: parent.width
      index: 0
    }
    RowComponent {
      anchors.horizontalCenter: parent.horizontalCenter
      anchors.verticalCenter: parent.verticalCenter
      width: parent.width
      index: 1
    }
    RowComponent {
      anchors.bottom: parent.bottom
      anchors.left: parent.left
      anchors.right: parent.right
      width: parent.width
      index: 2
    }

    onHeightChanged: {
      var heightThreshold = 40;
      children[0].visible = height > heightThreshold;
      children[2].visible = height > heightThreshold;
    }
  }

  Behavior on opacity {
    NumberAnimation {
      duration: 150
      easing.type: Easing.OutQuad
    }
  }
  MouseArea {
    id: mouseArea
    anchors.fill: parent
    hoverEnabled: true
    acceptedButtons: Qt.NoButton
    onEntered: {
      root.opacity = 0;
    }
    onExited: {
      root.opacity = 1;
      tooltipRequested("");
    }
    onPositionChanged: {
      tooltipRequested(NTrackInfoModel.formatTooltip(mouseArea.mouseX / mouseArea.width));
    }
  }
}
