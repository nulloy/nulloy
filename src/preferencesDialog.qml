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

import QtQuick 2.2
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.4
import src 1.0

NDialog {
  id: dialog

  title: Qt.application.name + qsTr(" Preferences")
  width: 620
  standardButtons: StandardButton.Ok | StandardButton.Apply | StandardButton.Cancel

  property bool checkUpdate: false

  SystemPalette {
    id: systemPalette
  }

  function filterLineBreaks(event) {
    if (event.key == Qt.Key_Return || event.key == Qt.Key_Enter) {
      event.accepted = true;
    }
  }

  TabView {
    id: tabView
    anchors.fill: parent

    Timer {
      id: hidePluginsTabTimer
      interval: 0
      repeat: false
      onTriggered: {
        if (!NPluginsModel.some(item => item.plugins.length > 1)) {
          tabView.removeTab(3); // plugins tab
        }
      }
    }
    Component.onCompleted: {
      hidePluginsTabTimer.start();
    }

    Tab {
      title: qsTr("General")
      NScrollView {
        id: scrollView
        ColumnLayout {
          id: innerColumnLayout

          Component.onCompleted: {
            dialog.height = innerColumnLayout.implicitHeight + 150; // + window decoration + standard + tabs row
          }

          GridLayout {
            columns: 3
            Component.onCompleted: {
              if (typeof NSkinsModel === undefined) {
                // hide skins row:
                children[3].visible = false;
                children[4].visible = false;
                children[5].visible = false;
              }
            }

            Label {
              text: qsTr("Language:")
            }
            ComboBox {
              Layout.preferredWidth: 200
              textRole: "text"
              model: NLanguagesModel
              currentIndex: model.findIndex(item => item.value === NSettings.value("Language"))
              Component.onCompleted: visible = count > 1
              onActivated: {
                NSettings.setValue("Language", NLanguagesModel[currentIndex].value);
                languageRestartLabel.visible = true;
              }
              NScrollRedirect {
                target: scrollView
              }
            }
            RowLayout {
              Label {
                id: languageRestartLabel
                text: qsTr("Switching languages requires restart")
                color: "red"
                visible: false
              }
              Item {
                Layout.fillWidth: true
              }
            }

            Label {
              text: qsTr("Skin:")
            }
            ComboBox {
              Layout.preferredWidth: 200
              textRole: "text"
              model: NSkinsModel
              currentIndex: model.findIndex(item => item.value === NSettings.value("Skin"))
              Component.onCompleted: visible = count > 1
              onActivated: {
                NSettings.setValue("Skin", NSkinsModel[currentIndex].value);
                skinRestartLabel.visible = true;
              }
              NScrollRedirect {
                target: scrollView
              }
            }
            RowLayout {
              Label {
                id: skinRestartLabel
                text: qsTr("Switching skins requires restart")
                color: "red"
                visible: false
              }
              Item {
                Layout.fillWidth: true
              }
            }

            Label {
              text: qsTr("Qt style:")
            }
            ComboBox {
              Layout.preferredWidth: 200
              model: NStylesModel
              onActivated: {
                NSettings.setValue("Style", NStylesModel[currentIndex]);
                styleRestartLabel.visible = true;
              }
              Component.onCompleted: {
                visible = count > 1;
                currentIndex = find(NSettings.value("Style"));
              }
              NScrollRedirect {
                target: scrollView
              }
            }
            RowLayout {
              Label {
                id: styleRestartLabel
                text: qsTr("Switching styles requires restart")
                color: "red"
                visible: false
              }
              Item {
                Layout.fillWidth: true
              }
            }
          }

          CheckBox {
            text: qsTr("Always show icon in system tray")
            checked: NSettings.value("TrayIcon")
            onCheckedChanged: NSettings.setValue("TrayIcon", checked)
          }

          CheckBox {
            visible: Qt.platform.os != "osx"
            text: qsTr("Hide to system tray when closed")
            checked: NSettings.value("MinimizeToTray")
            onCheckedChanged: NSettings.setValue("MinimizeToTray", checked)
          }

          CheckBox {
            visible: Qt.platform.os == "osx"
            text: qsTr("Quit when closed")
            checked: NSettings.value("QuitOnClose")
            onCheckedChanged: NSettings.setValue("QuitOnClose", checked)
          }

          CheckBox {
            text: qsTr("Quit when playback finished")
            checked: NSettings.value("QuitWhenFinished")
            onCheckedChanged: NSettings.setValue("QuitWhenFinished", checked)
          }

          CheckBox {
            text: qsTr("Restore playlist after restart")
            checked: NSettings.value("RestorePlaylist")
            onCheckedChanged: NSettings.setValue("RestorePlaylist", checked)
          }

          CheckBox {
            text: qsTr("Start in paused state")
            checked: NSettings.value("StartPaused")
            onCheckedChanged: NSettings.setValue("StartPaused", checked)
          }

          CheckBox {
            text: qsTr("Allow only one instance")
            checked: NSettings.value("SingleInstance")
            onCheckedChanged: NSettings.setValue("SingleInstance", checked)
          }

          CheckBox {
            text: qsTr("Enqueue files when in one instance")
            checked: NSettings.value("EnqueueFiles")
            onCheckedChanged: NSettings.setValue("EnqueueFiles", checked)
          }

          CheckBox {
            text: qsTr("Play enqueued files immediately")
            checked: NSettings.value("PlayEnqueued")
            onCheckedChanged: NSettings.setValue("PlayEnqueued", checked)
          }

          CheckBox {
            text: qsTr("Show volume in decibels (using Stevens' law)")
            checked: NSettings.value("ShowDecibelsVolume")
            onCheckedChanged: NSettings.setValue("ShowDecibelsVolume", checked)
          }

          CheckBox {
            visible: Qt.platform.os == "windows"
            text: qsTr("Show progress on taskbar")
            checked: NSettings.value("TaskbarProgress")
            onCheckedChanged: NSettings.setValue("TaskbarProgress", checked)
          }

          CheckBox {
            text: qsTr("Display log dialog in case of errors")
            checked: NSettings.value("DisplayLogDialog")
            onCheckedChanged: NSettings.setValue("DisplayLogDialog", checked)
          }

          RowLayout {
            visible: checkUpdate
            CheckBox {
              text: qsTr("Automatically check for updates")
              checked: NSettings.value("AutoCheckUpdates")
              onCheckedChanged: NSettings.setValue("AutoCheckUpdates", checked)
            }

            Button {
              text: qsTr("Check now")
              onClicked: {
                versionLabel.text = qsTr("Checking...");
                NUpdateChecker.checkOnline();
              }
            }

            Label {
              id: versionLabel
              function setText(version) {
                if (version != "") {
                  text = qsTr("Latest: ") + version;
                }
              }
              Component.onCompleted: {
                versionLabel.setText(NUpdateChecker.version);
              }
              Connections {
                target: NUpdateChecker
                function onVersionChanged() {
                  versionLabel.setText(NUpdateChecker.version);
                }
              }
            }
          }

          RowLayout {
            visible: Qt.platform.os != "osx" && Qt.platform.os != "windows"
            CheckBox {
              id: customFileManagerCheckBox
              text: qsTr("Custom File Manager:")
              checked: NSettings.value("CustomFileManager")
              onCheckedChanged: NSettings.setValue("CustomFileManager", checked)
            }

            TextField {
              Layout.fillWidth: true
              text: NSettings.value("CustomFileManagerCommand")
              onTextChanged: NSettings.setValue("CustomFileManagerCommand", text)
              enabled: customFileManagerCheckBox.checked
            }

            Button {
              text: qsTr("Help")
              onClicked: {
                customFileManagerHelpDialog.open();
              }

              Dialog {
                id: customFileManagerHelpDialog
                modality: Qt.NonModal
                title: qsTr("File Manager Configuration")
                RowLayout {
                  NTextArea {
                    Layout.preferredWidth: 640
                    wrapMode: Text.WordWrap
                    textFormat: Text.MarkdownText
                    Component.onCompleted: {
                      let txt = "";
                      txt += qsTr("Supported parameters:");
                      txt += "\n\n";
                      txt += "* **%F** - " + qsTr("File name");
                      txt += "\n";
                      txt += "* **%p** - " + qsTr("File name including absolute path");
                      txt += "\n";
                      txt += "* **%P** - " + qsTr("Directory path without file name");
                      txt += "\n\n";
                      txt += qsTr("Examples:");
                      txt += "\n\n";
                      txt += "* `open -a '/Applications/Path Finder.app' '%p'`";
                      txt += "\n";
                      txt += "* `pcmanfm -n '%P' & sleep 1.5 && xdotool type '%F' && xdotool key Escape`";
                      text = txt;
                    }
                  }
                }
              }
            }
          }

          RowLayout {
            visible: Qt.platform.os != "osx" && Qt.platform.os != "windows"
            CheckBox {
              id: customTrashCheckBox
              text: qsTr("Custom Trash Command:")
              checked: NSettings.value("CustomTrash")
              onCheckedChanged: NSettings.setValue("CustomTrash", checked)
            }

            TextField {
              Layout.fillWidth: true
              text: NSettings.value("CustomTrashCommand")
              onTextChanged: NSettings.setValue("CustomTrashCommand", text)
              enabled: customTrashCheckBox.checked
            }

            Button {
              text: qsTr("Help")
              onClicked: {
                trashCommandHelpDialog.open();
              }

              Dialog {
                id: trashCommandHelpDialog
                modality: Qt.NonModal
                title: qsTr("Trash Command Configuration")
                RowLayout {
                  NTextArea {
                    Layout.preferredWidth: 640
                    wrapMode: Text.WordWrap
                    textFormat: Text.MarkdownText
                    Component.onCompleted: {
                      let txt = "";
                      txt += qsTr("Supported parameters:");
                      txt += "\n\n";
                      txt += "* **%F** - " + qsTr("File name");
                      txt += "\n";
                      txt += "* **%p** - " + qsTr("File name including absolute path");
                      txt += "\n";
                      txt += "* **%P** - " + qsTr("Directory path without file name");
                      txt += "\n\n";
                      txt += qsTr("Examples:");
                      txt += "\n\n";
                      txt += "* `trash-put '%p'`";
                      txt += "\n";
                      txt += "* `mkdir -p \"$HOME/.Trash\" && mv '%p' \"$HOME/.Trash/\"`";
                      text = txt;
                    }
                  }
                }
              }
            }
          }

          RowLayout {
            Label {
              text: qsTr("File filters:")
              Layout.alignment: Qt.AlignTop
            }

            TextArea {
              Layout.fillWidth: true
              Layout.fillHeight: true
              Layout.preferredHeight: 100

              text: NSettings.value("FileFilters")
              onTextChanged: NSettings.setValue("FileFilters", text)
              Keys.onPressed: filterLineBreaks(event)

              NScrollRedirect {
                target: scrollView
              }
            }
          }
        }
      }
    }

    Tab {
      title: qsTr("Track Information")
      NScrollView {
        id: scrollView
        ColumnLayout {
          id: innerColumnLayout

          RowLayout {
            GridLayout {
              columns: 2

              Label {
                text: qsTr("Window title:")
              }
              TextField {
                Layout.fillWidth: true
                text: NSettings.value("WindowTitleTrackInfo")
                onTextChanged: NSettings.setValue("WindowTitleTrackInfo", text)
              }

              Label {
                text: qsTr("Playlist item:")
              }
              TextField {
                Layout.fillWidth: true
                text: NSettings.value("PlaylistTrackInfo")
                onTextChanged: NSettings.setValue("PlaylistTrackInfo", text)
              }

              Label {
                text: qsTr("Encoding:")
              }
              ComboBox {
                Layout.fillWidth: true
                model: NEncodingsModel
                onActivated: {
                  NSettings.setValue("EncodingTrackInfo", NEncodingsModel[currentIndex]);
                }
                Component.onCompleted: {
                  enabled = count > 1;
                  currentIndex = find(NSettings.value("EncodingTrackInfo"));
                }
                NScrollRedirect {
                  target: scrollView
                }
              }

              Label {
                text: qsTr("Tooltip:")
              }
              TextField {
                Layout.fillWidth: true
                text: NSettings.value("TooltipTrackInfo")
                onTextChanged: NSettings.setValue("TooltipTrackInfo", text)
              }
            }

            Button {
              text: qsTr("Help")
              onClicked: {
                customFileManagerHelpDialog.open();
              }

              Dialog {
                id: customFileManagerHelpDialog
                modality: Qt.NonModal
                title: qsTr("Title Formats")
                RowLayout {
                  NTextArea {
                    Layout.preferredWidth: 640
                    wrapMode: Text.WordWrap
                    textFormat: Text.MarkdownText
                    Component.onCompleted: {
                      let txt = "";
                      txt = qsTr("Supported parameters:");
                      txt += "\n\n";
                      txt += "* **%a** - " + qsTr("Artist") + "\n";
                      txt += "* **%t** - " + qsTr("Title") + "\n";
                      txt += "* **%A** - " + qsTr("Album") + "\n";
                      txt += "* **%c** - " + qsTr("Comment") + "\n";
                      txt += "* **%g** - " + qsTr("Genre") + "\n";
                      txt += "* **%y** - " + qsTr("Year") + "\n";
                      txt += "* **%n** - " + qsTr("Track number") + "\n";
                      txt += "* **%i** - " + qsTr("Track index in playlist (Playlist only)") + "\n";
                      txt += "* **%T** - " + qsTr("Elapsed playback time (Waveform only)") + "\n";
                      txt += "* **%r** - " + qsTr("Remaining playback time (Waveform only)") + "\n";
                      txt += "* **%C** - " + qsTr("Time position under cursor (Tooltip only)") + "\n";
                      txt += "* **%o** - " + qsTr("Time offset under cursor (Tooltip only)") + "\n";
                      txt += "* **%d** - " + qsTr("Duration in format hh:mm:ss") + "\n";
                      txt += "* **%D** - " + qsTr("Duration in seconds") + "\n";
                      txt += "* **%L** - " + qsTr("Playlist duration in format hh:mm:ss") + "\n";
                      txt += "* **%b** - " + qsTr("Bit depth") + "\n";
                      txt += "* **%B** - " + qsTr("Bitrate in Kbps") + "\n";
                      txt += "* **%s** - " + qsTr("Sample rate in kHz") + "\n";
                      txt += "* **%H** - " + qsTr("Number of channels") + "\n";
                      txt += "* **%M** - " + qsTr("BPM (beats per minute)") + "\n";
                      txt += "* **%f** - " + qsTr("File name without extension") + "\n";
                      txt += "* **%F** - " + qsTr("File name") + "\n";
                      txt += "* **%p** - " + qsTr("File name including absolute path") + "\n";
                      txt += "* **%P** - " + qsTr("Directory path without file name") + "\n";
                      txt += "* **%N** - " + qsTr("Directory name") + "\n";
                      txt += "* **%e** - " + qsTr("File name extension") + "\n";
                      txt += "* **%E** - " + qsTr("File name extension in uppercase") + "\n";
                      txt += "* **%v** - " + qsTr("Program version number") + "\n";
                      txt += "* **{** - " + qsTr("Start of a condition block. Use \"\\\\{\" to print \"{\" character.") + "\n";
                      txt += "* **}** - " + qsTr("End of a condition block. Use \"\\\\}\" to print \"}\" character.") + "\n";
                      txt += "* **|** - " + qsTr("Alternative separator, to be used inside a condition block. Use \"\\\\|\" to print \"|\" character.") + "\n";
                      txt += "* **\\\\%** - " + qsTr("Print \"%\" character") + "\n";
                      txt += "\n\n";
                      txt += qsTr("Examples:");
                      txt += "\n\n";
                      txt += "* **%g** - " + qsTr("Print \"\\<genre\\>\". If not available, print nothing.") + "\n";
                      txt += "* **{Comment: %c}** - " + qsTr("Print \"Comment: \\<comment text\\>\". If not available, print nothing.") + "\n";
                      txt += "* **{%a - %t|%F}** - " + qsTr("Print \"\\<artist\\> - \\<title\\>\". If either of the tags is not available, print file name instead.") + "\n";
                      txt += "* **{%B/%s|{%B}{%s}}** - " + qsTr("Print \"\\<bitrate\\>/\\<sample rate\\>\". If either of the tags is not available, first try to print bitrate, then try to print sample rate.") + "\n";
                      text = txt;
                    }
                  }
                }
              }
            }
          }
          RowLayout {
            id: tooltipOffsetLayout
            Label {
              text: qsTr("Tooltip offset relative to mouse:")
            }
            Label {
              text: "X:"
            }
            SpinBox {
              id: tooltipOffsetXSpinBox
              Layout.preferredWidth: 70
              value: NSettings.value("TooltipOffset")[0]
              onValueChanged: NSettings.setValue("TooltipOffset", [tooltipOffsetXSpinBox.value, tooltipOffsetYSpinBox.value])
              NScrollRedirect {
                target: scrollView
              }
            }
            Label {
              text: "Y:"
            }
            SpinBox {
              id: tooltipOffsetYSpinBox
              Layout.preferredWidth: 70
              value: NSettings.value("TooltipOffset")[1]
              onValueChanged: NSettings.setValue("TooltipOffset", [tooltipOffsetXSpinBox.value, tooltipOffsetYSpinBox.value])
              NScrollRedirect {
                target: scrollView
              }
            }
          }
          Item {
            height: 5
          }
          Label {
            text: qsTr("Waveform sections:")
          }
          RowLayout {
            Layout.fillHeight: false
            Component {
              id: trackInfoCell
              TextArea {
                horizontalAlignment: TextInput.AlignHCenter
                verticalAlignment: TextEdit.AlignVCenter
                text: NSettings.value(settingsName)
                onTextChanged: NSettings.setValue(settingsName, text)
                Keys.onPressed: filterLineBreaks(event)

                NScrollRedirect {
                  target: scrollView
                }
              }
            }
            ColumnLayout {
              id: firstColumn
              Label {
                text: ""
                Layout.fillHeight: true
              }
              Label {
                text: qsTr("Top")
                Layout.fillHeight: true
                horizontalAlignment: Text.AlignRight
                Layout.minimumWidth: firstColumn.width
              }
              Label {
                text: qsTr("Middle")
                Layout.fillHeight: true
                horizontalAlignment: Text.AlignRight
                Layout.minimumWidth: firstColumn.width
              }
              Label {
                text: qsTr("Bottom")
                Layout.fillHeight: true
                horizontalAlignment: Text.AlignRight
                Layout.minimumWidth: firstColumn.width
              }
            }
            ColumnLayout {
              Layout.fillWidth: true
              Label {
                text: qsTr("Left")
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
              }
              Loader {
                property string settingsName: "TrackInfo/TopLeft"
                property ScrollView scrollRedirectTarget: scrollView
                sourceComponent: trackInfoCell
                Layout.fillWidth: true
                Layout.preferredHeight: 50
              }
              Loader {
                property string settingsName: "TrackInfo/MiddleLeft"
                property ScrollView scrollRedirectTarget: scrollView
                sourceComponent: trackInfoCell
                Layout.fillWidth: true
                Layout.preferredHeight: 50
              }
              Loader {
                property string settingsName: "TrackInfo/BottomLeft"
                property ScrollView scrollRedirectTarget: scrollView
                sourceComponent: trackInfoCell
                Layout.fillWidth: true
                Layout.preferredHeight: 50
              }
            }
            ColumnLayout {
              Layout.fillWidth: true
              Label {
                text: qsTr("Center")
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
              }
              Loader {
                property string settingsName: "TrackInfo/TopCenter"
                property ScrollView scrollRedirectTarget: scrollView
                sourceComponent: trackInfoCell
                Layout.fillWidth: true
                Layout.preferredHeight: 50
              }
              Loader {
                property string settingsName: "TrackInfo/MiddleCenter"
                property ScrollView scrollRedirectTarget: scrollView
                sourceComponent: trackInfoCell
                Layout.fillWidth: true
                Layout.preferredHeight: 50
              }
              Loader {
                property string settingsName: "TrackInfo/BottomCenter"
                property ScrollView scrollRedirectTarget: scrollView
                sourceComponent: trackInfoCell
                Layout.fillWidth: true
                Layout.preferredHeight: 50
              }
            }
            ColumnLayout {
              Layout.fillWidth: true
              Label {
                text: qsTr("Right")
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
              }
              Loader {
                property string settingsName: "TrackInfo/TopRight"
                property ScrollView scrollRedirectTarget: scrollView
                sourceComponent: trackInfoCell
                Layout.fillWidth: true
                Layout.preferredHeight: 50
              }
              Loader {
                property string settingsName: "TrackInfo/MiddleRight"
                property ScrollView scrollRedirectTarget: scrollView
                sourceComponent: trackInfoCell
                Layout.fillWidth: true
                Layout.preferredHeight: 50
              }
              Loader {
                property string settingsName: "TrackInfo/BottomRight"
                property ScrollView scrollRedirectTarget: scrollView
                sourceComponent: trackInfoCell
                Layout.fillWidth: true
                Layout.preferredHeight: 50
              }
            }
          }
          Item {
            Layout.fillHeight: true
          }
        }
      }
    }

    Tab {
      title: qsTr("Keyboard")
      NScrollView {
        id: scrollView
        ColumnLayout {
          id: innerColumnLayout

          TableView {
            id: shortcutEditorTable
            model: NShortcutEditorModel
            alternatingRowColors: false
            Layout.fillWidth: true
            Layout.fillHeight: true
            property int cellWidth: (shortcutEditorTable.width - shortcutEditorTable.__verticalScrollBar.width) / 4 - 1
            property int cellHeight: 50
            horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff

            rowDelegate: Rectangle {
              height: shortcutEditorTable.cellHeight
              color: "transparent"
            }

            Component {
              id: tableCell
              TextArea {
                id: delegate
                readOnly: !editable
                text: cellModel[cellRole]
                frameVisible: false
                backgroundVisible: false
                horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                verticalAlignment: TextEdit.AlignVCenter

                Rectangle {
                  width: delegate.width
                  height: 1
                  color: Qt.platform.os == "osx" ? systemPalette.mid : systemPalette.shadow
                  anchors {
                    bottom: delegate.bottom
                    left: delegate.left
                  }
                }
                Rectangle {
                  width: 1
                  height: delegate.height
                  color: Qt.platform.os == "osx" ? systemPalette.mid : systemPalette.shadow
                  anchors {
                    top: delegate.top
                    right: delegate.right
                  }
                }
                Rectangle {
                  width: delegate.width
                  height: delegate.height
                  color: (editable && delegate.activeFocus) ? systemPalette.highlight : "transparent"
                  z: -1
                }
                Canvas {
                  width: 10
                  height: 10
                  anchors {
                    top: parent.top
                    right: parent.right
                    topMargin: 3
                    rightMargin: 4
                  }
                  onPaint: {
                    if (!editable) {
                      return;
                    }
                    let ctx = getContext("2d");
                    ctx.strokeStyle = "red";
                    ctx.lineWidth = 2;
                    ctx.beginPath();
                    ctx.moveTo(1, 1);
                    ctx.lineTo(width - 1, height - 1);
                    ctx.moveTo(width - 1, 1);
                    ctx.lineTo(1, height - 1);
                    ctx.stroke();
                  }
                  MouseArea {
                    anchors.fill: parent
                    onPressed: {
                      onClicked: cellModel[cellRole] = "";
                    }
                  }
                }
                Keys.onPressed: event => {
                  event.accepted = true;
                  if (!editable) {
                    return;
                  }
                  cellModel[cellRole] = NShortcutEditorModel.appendShortcut(text, event.key, event.modifiers);
                }
                onCursorPositionChanged: {
                  delegate.cursorPosition = 0;
                }
                NScrollRedirect {
                  target: scrollRedirectTarget
                }
              }
            }

            TableViewColumn {
              title: qsTr("Action")
              width: shortcutEditorTable.cellWidth
              delegate: Loader {
                property bool editable: false
                property var cellModel: model
                property string cellRole: "name"
                property ScrollView scrollRedirectTarget: shortcutEditorTable
                sourceComponent: tableCell
              }
            }
            TableViewColumn {
              title: qsTr("Description")
              width: shortcutEditorTable.cellWidth
              delegate: Loader {
                property bool editable: false
                property var cellModel: model
                property string cellRole: "description"
                property ScrollView scrollRedirectTarget: shortcutEditorTable
                sourceComponent: tableCell
              }
            }
            TableViewColumn {
              title: qsTr("Shortcut")
              width: shortcutEditorTable.cellWidth
              delegate: Loader {
                property bool editable: true
                property var cellModel: model
                property string cellRole: "shortcut"
                property ScrollView scrollRedirectTarget: shortcutEditorTable
                sourceComponent: tableCell
              }
            }
            TableViewColumn {
              title: qsTr("Global Shortcut")
              width: shortcutEditorTable.cellWidth
              delegate: Loader {
                property bool editable: true
                property var cellModel: model
                property string cellRole: "globalShortcut"
                property ScrollView scrollRedirectTarget: shortcutEditorTable
                sourceComponent: tableCell
              }
            }
          }

          Item {
            Layout.preferredHeight: 5
          }

          RowLayout {
            spacing: 20
            ColumnLayout {
              Layout.alignment: Qt.AlignTop
              Label {
                text: qsTr("Jumps (in seconds):")
              }
              RowLayout {
                Item {
                  Layout.preferredWidth: 10
                }
                ColumnLayout {
                  RowLayout {
                    Label {
                      text: qsTr("Jump #1")
                    }
                    SpinBox {
                      decimals: 1
                      stepSize: 0.1
                      maximumValue: 1000.0
                      Layout.preferredWidth: 70
                      value: NSettings.value("Jump1")
                      onValueChanged: NSettings.setValue("Jump1", value)
                      NScrollRedirect {
                        target: scrollView
                      }
                    }
                  }
                  RowLayout {
                    Label {
                      text: qsTr("Jump #2")
                    }
                    SpinBox {
                      decimals: 1
                      stepSize: 0.1
                      maximumValue: 1000.0
                      Layout.preferredWidth: 70
                      value: NSettings.value("Jump2")
                      onValueChanged: NSettings.setValue("Jump2", value)
                      NScrollRedirect {
                        target: scrollView
                      }
                    }
                  }
                  RowLayout {
                    Label {
                      text: qsTr("Jump #3")
                    }
                    SpinBox {
                      decimals: 1
                      stepSize: 0.1
                      maximumValue: 1000.0
                      Layout.preferredWidth: 70
                      value: NSettings.value("Jump3")
                      onValueChanged: NSettings.setValue("Jump3", value)
                      NScrollRedirect {
                        target: scrollView
                      }
                    }
                  }
                }
              }
            }
            ColumnLayout {
              Layout.alignment: Qt.AlignTop
              Label {
                text: qsTr("Speed:")
              }
              RowLayout {
                Item {
                  Layout.preferredWidth: 10
                }
                ColumnLayout {
                  RowLayout {
                    Label {
                      text: qsTr("Increment step")
                    }
                    SpinBox {
                      decimals: 2
                      stepSize: 0.01
                      maximumValue: 0.1
                      Layout.preferredWidth: 70
                      value: NSettings.value("SpeedStep")
                      onValueChanged: NSettings.setValue("SpeedStep", value)
                      NScrollRedirect {
                        target: scrollView
                      }
                    }
                  }
                }
              }
            }
            ColumnLayout {
              visible: false
              Layout.alignment: Qt.AlignTop
              Label {
                text: qsTr("Pitch:")
              }
              RowLayout {
                Item {
                  Layout.preferredWidth: 10
                }
                ColumnLayout {
                  RowLayout {
                    Label {
                      text: qsTr("Increment step")
                    }
                    SpinBox {
                      decimals: 2
                      stepSize: 0.01
                      maximumValue: 0.1
                      Layout.preferredWidth: 70
                      //value: NSettings.value("PitchStep")
                      //onValueChanged: NSettings.setValue("PitchStep", value)
                      NScrollRedirect {
                        target: scrollView
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    Tab {
      title: qsTr("Plugins")
      NScrollView {
        id: scrollView
        ColumnLayout {
          id: innerColumnLayout

          ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            model: NPluginsModel
            spacing: 30

            delegate: Item {
              RowLayout {
                Label {
                  text: modelData.pluginType + ":"
                  Layout.preferredWidth: 120
                }

                ComboBox {
                  model: modelData.plugins
                  Component.onCompleted: {
                    currentIndex = find(NSettings.value("Plugins/" + modelData.pluginType));
                    enabled = count > 1;
                  }
                  onActivated: {
                    NSettings.setValue("Plugins/" + modelData.pluginType, currentText);
                    pluginsRestartLabel.visible = true;
                  }
                }
              }
            }
          }

          Label {
            id: pluginsRestartLabel
            text: qsTr("Switching plugins requires restart")
            color: "red"
            visible: false
          }
        }
      }
    }
  }
}
