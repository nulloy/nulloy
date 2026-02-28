/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2026 Sergey Vlasov <sergey@vlasov.me>
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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

ColumnLayout {
  id: root
  anchors.margins: 10
  anchors.bottomMargin: 0
  spacing: 0

  SystemPalette {
    id: systemPalette
    colorGroup: SystemPalette.Active
  }

  default property list<NTab> tabs
  property int currentIndex: 0

  Component.onCompleted: {
    for (let i = 0; i < tabs.length; ++i) {
      const tab = tabs[i];
      const item = tab.contentItem;
      if (item) {
        item.parent = stackLayout;
        item.enabled = Qt.binding(function () {
          return tab.enabled;
        });
      }
    }
  }

  Component {
    id: fusionTabs
    RowLayout {
      spacing: -1
      Layout.fillWidth: true
      Repeater {
        model: root.tabs
        delegate: Item {
          id: tabButton
          required property var modelData
          required property int index
          readonly property bool checked: root.currentIndex === index
          readonly property bool hovered: mouseArea.containsMouse
          implicitWidth: label.implicitWidth + 20
          Layout.preferredHeight: checked ? 27 : 25
          Layout.alignment: Qt.AlignBottom
          visible: modelData.visible
          clip: true
          Rectangle {
            width: parent.width
            height: parent.height + radius
            radius: tabButton.checked ? 3 : 2
            border.color: Qt.darker(systemPalette.window, 1.3)
            border.width: 1
            gradient: Gradient {
              GradientStop {
                position: 0.0
                color: tabButton.checked ? Qt.lighter(systemPalette.window, 1.3) : systemPalette.button
              }
              GradientStop {
                position: 1.0
                color: tabButton.checked ? Qt.lighter(systemPalette.window, 1.1) : Qt.lighter(systemPalette.midlight, tabButton.hovered ? 1.1 : 1.0)
              }
            }
          }
          Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: Qt.darker(systemPalette.window, 1.3)
            visible: !tabButton.checked
          }
          Text {
            id: label
            anchors.centerIn: parent
            text: tabButton.modelData.title
            color: systemPalette.buttonText
          }
          MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: root.currentIndex = index
          }
        }
      }

      Item {
        Layout.fillWidth: true
        Layout.preferredHeight: 27
        Layout.alignment: Qt.AlignBottom
        Rectangle {
          anchors.bottom: parent.bottom
          width: parent.width
          height: 1
          color: Qt.darker(systemPalette.window, 1.3)
        }
      }
    }
  }

  Component {
    id: macosTabs

    RowLayout {
      id: segmentedTabs
      anchors.centerIn: parent
      height: 34
      spacing: 0

      property real overlap: 10.0

      Item {
        Layout.fillWidth: true
      }

      Repeater {
        model: root.tabs
        delegate: Item {
          id: tabContainer
          required property int index
          required property var modelData

          Layout.fillWidth: true
          Layout.fillHeight: true
          Layout.preferredWidth: 1
          Layout.minimumWidth: button.implicitWidth
          Layout.maximumWidth: button.implicitWidth + 30

          clip: true
          z: root.currentIndex === index ? 1 : 0

          Button {
            id: button
            anchors.verticalCenter: parent.verticalCenter

            x: {
              if (index === 0) {
                return 0;
              }
              if (index === root.tabs.length - 1) {
                return parent.width - width;
              }
              return (parent.width - width) / 2;
            }
            width: {
              let multiplier = 0;
              if (index === 0 || index === root.tabs.length - 1) {
                multiplier = 1;
              } else {
                multiplier = 2;
              }
              return parent.width + overlap * multiplier;
            }
            height: parent.height

            text: modelData.title
            checkable: true
            autoExclusive: true
            checked: root.currentIndex === index
            highlighted: checked

            palette.buttonText: {
              if (checked && Window.window.active && systemPalette.accent.hslLightness < 0.6) {
                return "white";
              } else {
                return systemPalette.buttonText;
              }
            }

            onClicked: root.currentIndex = index
          }

          Rectangle {
            width: 1
            height: parent.height / 2
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            color: systemPalette.mid
            visible: (!Window.window.active || (!button.checked && root.currentIndex !== index + 1)) && index < root.tabs.length - 1
            z: 2
          }
        }
      }
      Item {
        Layout.fillWidth: true
      }
    }
  }

  Loader {
    Layout.fillWidth: true
    sourceComponent: Qt.platform.os === "osx" ? macosTabs : fusionTabs
  }

  Item {
    Layout.fillWidth: true
    Layout.fillHeight: true
    clip: true

    Frame {
      anchors.fill: parent
      anchors.topMargin: Qt.platform.os === "osx" ? 0 : -1
      padding: root.tabs[root.currentIndex].padding

      background: Rectangle {
        color: Qt.lighter(systemPalette.window, 1.1)
        border.color: Qt.darker(systemPalette.window, 1.3)
        border.width: 1
        radius: Qt.platform.os === "osx" ? 5 : 0
      }

      StackLayout {
        id: stackLayout
        anchors.fill: parent
        currentIndex: root.currentIndex
      }
    }
  }
}
