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

NDialog {
  id: dialog

  title: Qt.application.name + qsTr(" Preferences")
  width: 620
  height: 650
  standardButtons: Dialog.Ok | Dialog.Apply | Dialog.Cancel

  property bool checkUpdate: false

  SystemPalette {
    id: systemPalette
  }

  function filterLineBreaks(event) {
    if (event.key == Qt.Key_Return || event.key == Qt.Key_Enter) {
      event.accepted = true;
    }
  }

  component HelpDialog: Dialog {
    standardButtons: Dialog.Close

    property alias text: textArea.text

    ScrollView {
      id: scrollView
      anchors.fill: parent
      horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
      verticalScrollBarPolicy: Qt.ScrollBarAsNeeded

      TextEdit {
        id: textArea
        readOnly: true
        width: scrollView.width - scrollView.__verticalScrollBar.width
        wrapMode: TextEdit.WordWrap
        selectByMouse: true
        textFormat: Text.MarkdownText
      }
    }
  }

  component SettingsCheckBox: CheckBox {
    required property string settingsName

    checked: NSettings.value(settingsName)
    onCheckedChanged: NSettings.setValue(settingsName, checked)
    implicitHeight: 20
    Layout.leftMargin: 0
  }

  component TrackInfoTextArea: TextArea {
    required property string settingsName

    Layout.fillWidth: true
    Layout.preferredHeight: 50
    horizontalAlignment: TextInput.AlignHCenter
    verticalAlignment: TextEdit.AlignVCenter
    wrapMode: Text.WordWrap

    implicitWidth: parent.width

    text: NSettings.value(settingsName)
    onTextChanged: NSettings.setValue(settingsName, text)

    Keys.onPressed: filterLineBreaks(event)

    Rectangle {
      anchors.fill: parent
      border.color: parent.activeFocus ? systemPalette.highlight : systemPalette.mid
      color: "transparent"
    }
  }

  component ShortcutsTableTextArea: TextArea {
    id: delegate

    property bool editable: true
    required property var cellModel
    required property string cellRole

    readOnly: !editable
    text: cellModel[cellRole]
    frameVisible: false
    horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
    verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff

    verticalAlignment: TextEdit.AlignVCenter
    wrapMode: Text.WordWrap

    onTextChanged: cursorPosition = text.length
    onActiveFocusChanged: cursorPosition = text.length
    onCursorPositionChanged: cursorPosition = text.length

    Layout.fillWidth: true
    Layout.fillHeight: true

    Rectangle {
      anchors.fill: parent
      anchors.margins: -1
      color: "transparent"
      border.width: 1
      border.color: Qt.platform.os == "osx" ? systemPalette.mid : systemPalette.midlight
      z: -1
    }

    Rectangle {
      anchors.fill: parent
      anchors.rightMargin: 1
      visible: delegate.editable && delegate.activeFocus
      radius: 3
      border.width: 1
      border.color: systemPalette.highlight
      color: "transparent"
    }

    Label {
      anchors.right: parent.right
      anchors.top: parent.top
      anchors.margins: 2
      visible: delegate.editable
      horizontalAlignment: Text.AlignHCenter
      verticalAlignment: Text.AlignVCenter

      opacity: 0.6
      width: 20
      height: 20
      text: "×"

      Rectangle {
        anchors.fill: parent
        color: mouseArea.containsMouse ? systemPalette.midlight : "transparent"
        radius: 12
        z: -1
      }

      MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: {
          cellModel[cellRole] = "";
        }
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
      }
    }

    Keys.onPressed: event => {
      event.accepted = true;
      cellModel[cellRole] = NShortcutEditorModel.appendShortcut(text, event.key, event.modifiers);
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
        if (!NPluginModel.some(item => item.plugins.length > 1)) {
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
        GridLayout {
          columns: 3
          Component.onCompleted: {
            if (typeof NSkinModel === undefined) {
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
            model: NLanguageModel
            currentIndex: model.findIndex(item => item.value === NSettings.value("Language"))
            Component.onCompleted: visible = count > 1
            onActivated: {
              NSettings.setValue("Language", NLanguageModel[currentIndex].value);
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
            NScrollRedirect {
              target: scrollView
            }
          }

          Label {
            text: qsTr("Skin:")
          }
          ComboBox {
            Layout.preferredWidth: 200
            textRole: "text"
            model: NSkinModel
            currentIndex: model.findIndex(item => item.value === NSettings.value("Skin"))
            Component.onCompleted: visible = count > 1
            onActivated: {
              NSettings.setValue("Skin", NSkinModel[currentIndex].value);
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
        }

        SettingsCheckBox {
          text: qsTr("Always show icon in system tray")
          settingsName: "TrayIcon"
        }

        SettingsCheckBox {
          visible: Qt.platform.os != "osx"
          text: qsTr("Hide to system tray when closed")
          settingsName: "MinimizeToTray"
        }

        SettingsCheckBox {
          visible: Qt.platform.os == "osx"
          text: qsTr("Quit when closed")
          settingsName: "QuitOnClose"
        }

        SettingsCheckBox {
          text: qsTr("Quit when playback finished")
          settingsName: "QuitWhenFinished"
        }

        SettingsCheckBox {
          text: qsTr("Restore playlist after restart")
          settingsName: "RestorePlaylist"
        }

        SettingsCheckBox {
          text: qsTr("Start in paused state")
          settingsName: "StartPaused"
        }

        SettingsCheckBox {
          text: qsTr("Allow only one instance")
          settingsName: "SingleInstance"
        }

        SettingsCheckBox {
          text: qsTr("Enqueue files when in one instance")
          settingsName: "EnqueueFiles"
        }

        SettingsCheckBox {
          text: qsTr("Play enqueued files immediately")
          settingsName: "PlayEnqueued"
        }

        SettingsCheckBox {
          text: qsTr("Show volume in decibels (using Stevens' law)")
          settingsName: "ShowDecibelsVolume"
        }

        SettingsCheckBox {
          visible: Qt.platform.os == "windows"
          text: qsTr("Show progress on taskbar")
          settingsName: "TaskbarProgress"
        }

        SettingsCheckBox {
          text: qsTr("Display log dialog in case of errors")
          settingsName: "DisplayLogDialog"
        }

        RowLayout {
          visible: checkUpdate
          SettingsCheckBox {
            text: qsTr("Automatically check for updates")
            settingsName: "AutoCheckUpdates"
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
          SettingsCheckBox {
            id: customFileManagerCheckBox
            text: qsTr("Custom File Manager:")
            settingsName: "CustomFileManager"
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

            HelpDialog {
              id: customFileManagerHelpDialog
              title: qsTr("File Manager Configuration")
              width: 600

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

        RowLayout {
          visible: Qt.platform.os != "osx" && Qt.platform.os != "windows"
          SettingsCheckBox {
            id: customTrashCheckBox
            text: qsTr("Custom Trash Command:")
            settingsName: "CustomTrash"
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

            HelpDialog {
              id: trashCommandHelpDialog
              title: qsTr("Trash Command Configuration")
              width: 500

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
            wrapMode: Text.WordWrap
            onTextChanged: NSettings.setValue("FileFilters", text)
            Keys.onPressed: filterLineBreaks(event)

            Rectangle {
              anchors.fill: parent
              border.color: parent.activeFocus ? systemPalette.highlight : systemPalette.mid
              color: "transparent"
            }

            NScrollRedirect {
              target: scrollView
            }
          }
        }
      }
    }

    Tab {
      title: qsTr("Track Information")

      NScrollView {
        id: scrollView
        RowLayout {
          Layout.bottomMargin: 0

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
              model: NEncodingModel
              onActivated: {
                NSettings.setValue("EncodingTrackInfo", NEncodingModel[currentIndex]);
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
              titleFormatsHelpDialog.open();
            }

            HelpDialog {
              id: titleFormatsHelpDialog
              title: qsTr("Title Formats")

              width: 500
              height: 400

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

        Item {
          Layout.preferredHeight: 5
        }

        RowLayout {
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
          Layout.preferredHeight: 5
        }

        Label {
          text: qsTr("Waveform sections:")
        }

        RowLayout {
          Layout.fillHeight: false

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
            Layout.preferredWidth: parent.width

            Label {
              text: qsTr("Left")
              horizontalAlignment: Text.AlignHCenter
              Layout.fillWidth: true
            }

            TrackInfoTextArea {
              settingsName: "TrackInfo/TopLeft"
              NScrollRedirect {
                target: scrollView
              }
            }
            TrackInfoTextArea {
              settingsName: "TrackInfo/MiddleLeft"
              NScrollRedirect {
                target: scrollView
              }
            }
            TrackInfoTextArea {
              settingsName: "TrackInfo/BottomLeft"
              NScrollRedirect {
                target: scrollView
              }
            }
          }

          ColumnLayout {
            Layout.preferredWidth: parent.width

            Label {
              text: qsTr("Center")
              horizontalAlignment: Text.AlignHCenter
              Layout.fillWidth: true
            }

            TrackInfoTextArea {
              settingsName: "TrackInfo/TopCenter"
              NScrollRedirect {
                target: scrollView
              }
            }
            TrackInfoTextArea {
              settingsName: "TrackInfo/MiddleCenter"
              NScrollRedirect {
                target: scrollView
              }
            }
            TrackInfoTextArea {
              settingsName: "TrackInfo/BottomCenter"
              NScrollRedirect {
                target: scrollView
              }
            }
          }

          ColumnLayout {
            Layout.preferredWidth: parent.width

            Label {
              text: qsTr("Right")
              horizontalAlignment: Text.AlignHCenter
              Layout.fillWidth: true
            }

            TrackInfoTextArea {
              settingsName: "TrackInfo/TopRight"
              NScrollRedirect {
                target: scrollView
              }
            }
            TrackInfoTextArea {
              settingsName: "TrackInfo/MiddleRight"
              NScrollRedirect {
                target: scrollView
              }
            }
            TrackInfoTextArea {
              settingsName: "TrackInfo/BottomRight"
              NScrollRedirect {
                target: scrollView
              }
            }
          }
        }

        Item {
          Layout.fillHeight: true
        }
      }
    }

    Tab {
      title: qsTr("Keyboard")

      Rectangle {
        clip: true
        color: "transparent"

        ColumnLayout {
          anchors.fill: parent
          anchors.margins: 10

          TableView {
            id: shortcutEditorTable
            model: NShortcutEditorModel
            alternatingRowColors: false
            Layout.fillWidth: true
            Layout.fillHeight: true
            property int cellWidth: (shortcutEditorTable.width - shortcutEditorTable.__verticalScrollBar.width) / 4
            property int cellHeight: 40
            horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff

            rowDelegate: Rectangle {
              height: shortcutEditorTable.cellHeight
              color: "transparent"
            }

            TableViewColumn {
              title: qsTr("Action")
              width: shortcutEditorTable.cellWidth
              delegate: ShortcutsTableTextArea {
                editable: false
                cellModel: model
                cellRole: "name"
                NScrollRedirect {
                  target: shortcutEditorTable
                }
              }
            }

            TableViewColumn {
              title: qsTr("Description")
              width: shortcutEditorTable.cellWidth
              delegate: ShortcutsTableTextArea {
                editable: false
                cellModel: model
                cellRole: "description"
                NScrollRedirect {
                  target: shortcutEditorTable
                }
              }
            }

            TableViewColumn {
              title: qsTr("Shortcut")
              width: shortcutEditorTable.cellWidth
              delegate: ShortcutsTableTextArea {
                cellModel: model
                cellRole: "shortcut"
                NScrollRedirect {
                  target: shortcutEditorTable
                }
              }
            }

            TableViewColumn {
              title: qsTr("Global Shortcut")
              width: shortcutEditorTable.cellWidth
              delegate: ShortcutsTableTextArea {
                cellModel: model
                cellRole: "globalShortcut"
                NScrollRedirect {
                  target: shortcutEditorTable
                }
              }
            }
          }

          Item {
            Layout.preferredHeight: 10
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
                      NScrollRedirect {}
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
                      NScrollRedirect {}
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
                      NScrollRedirect {}
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
                      NScrollRedirect {}
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
                      value: NSettings.value("PitchStep")
                      onValueChanged: NSettings.setValue("PitchStep", value)
                      NScrollRedirect {}
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
        ListView {
          Layout.margins: 10
          Layout.fillWidth: true
          Layout.fillHeight: true

          model: NPluginModel
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
