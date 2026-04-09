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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

NDialog {
  id: dialog

  title: Qt.application.name + qsTr(" Preferences")
  width: 620
  height: 650
  minimumWidth: 450
  minimumHeight: 300

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
    popupType: Popup.Window
    standardButtons: Dialog.Close

    property alias text: textArea.text

    background: Rectangle {
      color: systemPalette.window
    }

    NScrollView {
      anchors.fill: parent
      TextArea {
        id: textArea
        readOnly: true
        Layout.fillWidth: true
        Layout.fillHeight: true
        wrapMode: Text.WordWrap
        textFormat: Text.MarkdownText
        background: null
      }
      Item {
        Layout.fillHeight: true
      }
    }

    footer: DialogButtonBox {
      padding: 10
      topPadding: 0
    }
  }

  component SettingsCheckBox: CheckBox {
    required property string settingsName

    checked: NSettings.value(settingsName)
    onCheckedChanged: NSettings.setValue(settingsName, checked)
    implicitHeight: 20
    Layout.leftMargin: 5
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

    background: Rectangle {
      border.color: parent.activeFocus ? systemPalette.highlight : systemPalette.mid
      color: systemPalette.base
    }
  }

  component ShortcutsTableHeader: Label {
    horizontalAlignment: TextInput.AlignHCenter
    verticalAlignment: TextEdit.AlignVCenter

    Layout.fillWidth: true
    Layout.preferredWidth: 1
    Layout.preferredHeight: 20

    Rectangle {
      anchors.fill: parent
      border.width: 1
      border.color: Qt.platform.os == "osx" ? systemPalette.mid : systemPalette.midlight
      z: -1

      gradient: Gradient {
        GradientStop {
          position: 0.0
          color: systemPalette.base
        }
        GradientStop {
          position: 1.0
          color: systemPalette.alternateBase
        }
      }
    }
  }

  component ShortcutsTableTextArea: TextArea {
    id: delegate

    property bool editable: true
    required property var cellModel
    required property string cellRole

    text: cellModel[cellRole]
    padding: 0
    readOnly: !editable

    verticalAlignment: TextEdit.AlignVCenter
    wrapMode: Text.WordWrap

    onTextChanged: cursorPosition = text.length
    onActiveFocusChanged: cursorPosition = text.length
    onCursorPositionChanged: cursorPosition = text.length

    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.preferredWidth: 999

    Rectangle {
      anchors.fill: parent
      color: systemPalette.base
      border.width: 1
      border.color: Qt.platform.os == "osx" ? systemPalette.mid : systemPalette.midlight
      z: -1
    }

    Rectangle {
      anchors.fill: parent
      anchors.margins: 1
      visible: delegate.editable && delegate.activeFocus
      radius: 3
      border.width: 1
      border.color: systemPalette.highlight
      color: "transparent"
    }

    Button {
      anchors.right: parent.right
      anchors.top: parent.top
      anchors.margins: 2
      visible: delegate.editable

      opacity: 0.6
      width: 20
      height: 20
      text: "×"

      background: Rectangle {
        color: parent.hovered ? systemPalette.midlight : "transparent"
        radius: 12
      }

      MouseArea {
        anchors.fill: parent
        onClicked: {
          cellModel[cellRole] = "";
        }
        cursorShape: Qt.PointingHandCursor
      }
    }

    MouseArea {
      anchors.fill: parent
      visible: !delegate.editable
      cursorShape: Qt.ArrowCursor
    }

    Keys.onPressed: event => {
      event.accepted = true;
      cellModel[cellRole] = NShortcutEditorModel.appendShortcut(text, event.key, event.modifiers);
    }
  }

  NTabView {
    anchors.fill: parent

    NTab {
      title: qsTr("General")

      NScrollView {
        GridLayout {
          Layout.margins: 10
          Layout.bottomMargin: 0
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
            model: NSkinModel
            currentIndex: model.findIndex(item => item.value === NSettings.value("Skin"))
            Component.onCompleted: visible = count > 1
            onActivated: {
              NSettings.setValue("Skin", NSkinModel[currentIndex].value);
              skinRestartLabel.visible = true;
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

        //SettingsCheckBox {
        //  visible: Qt.platform.os == "windows"
        //  text: qsTr("Show progress on taskbar")
        //  settingsName: "TaskbarProgress"
        //}

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
          Layout.rightMargin: 10
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
              contentWidth: 600

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
          Layout.rightMargin: 10
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
              contentWidth: 500

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
          Layout.topMargin: 0
          Layout.margins: 10
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

            background: Rectangle {
              border.color: parent.activeFocus ? systemPalette.highlight : systemPalette.mid
              color: systemPalette.base
            }
          }
        }
      }
    }

    NTab {
      title: qsTr("Track Information")

      NScrollView {
        RowLayout {
          Layout.margins: 10
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

              contentWidth: 500
              contentHeight: 400

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

        RowLayout {
          Layout.margins: 10

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
          }

          Label {
            text: "Y:"
          }

          SpinBox {
            id: tooltipOffsetYSpinBox
            Layout.preferredWidth: 70
            value: NSettings.value("TooltipOffset")[1]
            onValueChanged: NSettings.setValue("TooltipOffset", [tooltipOffsetXSpinBox.value, tooltipOffsetYSpinBox.value])
          }
        }

        Label {
          text: qsTr("Waveform sections:")
          Layout.leftMargin: 10
          Layout.rightMargin: 10
        }

        RowLayout {
          Layout.leftMargin: 10
          Layout.rightMargin: 10
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
            }
            TrackInfoTextArea {
              settingsName: "TrackInfo/MiddleLeft"
            }
            TrackInfoTextArea {
              settingsName: "TrackInfo/BottomLeft"
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
            }
            TrackInfoTextArea {
              settingsName: "TrackInfo/MiddleCenter"
            }
            TrackInfoTextArea {
              settingsName: "TrackInfo/BottomCenter"
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
            }
            TrackInfoTextArea {
              settingsName: "TrackInfo/MiddleRight"
            }
            TrackInfoTextArea {
              settingsName: "TrackInfo/BottomRight"
            }
          }
        }

        Item {
          Layout.fillHeight: true
        }
      }
    }

    NTab {
      title: qsTr("Keyboard")

      Pane {
        clip: true
        padding: 10
        rightPadding: 0

        background: Rectangle {
          color: "transparent"
        }

        ColumnLayout {
          anchors.fill: parent
          spacing: -1

          RowLayout {
            Layout.fillWidth: true
            Layout.rightMargin: 10
            spacing: -1
            ShortcutsTableHeader {
              text: qsTr("Action")
            }
            ShortcutsTableHeader {
              text: qsTr("Description")
            }
            ShortcutsTableHeader {
              text: qsTr("Shortcut")
            }
            ShortcutsTableHeader {
              text: qsTr("Global Shortcut")
            }
          }

          ListView {
            id: shortcutsTableListView
            model: NShortcutEditorModel
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 50
            clip: true
            spacing: -1

            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar {}

            Rectangle {
              anchors.fill: parent
              color: "transparent"
              border.width: 1
              border.color: Qt.platform.os == "osx" ? systemPalette.mid : systemPalette.midlight
              anchors.rightMargin: 10
            }

            Rectangle {
              anchors.fill: parent
              color: systemPalette.base
              anchors.rightMargin: 10
              z: -1
            }

            delegate: RowLayout {
              width: shortcutsTableListView.width
              height: 40
              spacing: -1

              ShortcutsTableTextArea {
                editable: false
                cellModel: model
                cellRole: "name"
              }
              ShortcutsTableTextArea {
                editable: false
                cellModel: model
                cellRole: "description"
              }
              ShortcutsTableTextArea {
                cellModel: model
                cellRole: "shortcut"
              }
              ShortcutsTableTextArea {
                Layout.rightMargin: 10
                cellModel: model
                cellRole: "globalShortcut"
              }
            }
          }

          Item {
            Layout.preferredHeight: 20
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
                      Layout.preferredWidth: 70
                      from: 0
                      to: 10000
                      stepSize: 1
                      value: NSettings.value("Jump1") * 10
                      onValueModified: NSettings.setValue("Jump1", value / 10.0)
                      textFromValue: function (value, locale) {
                        return Number(value / 10.0).toLocaleString(locale, 'f', 1);
                      }
                      valueFromText: function (text, locale) {
                        return Number.fromLocaleString(locale, text) * 10;
                      }
                    }
                  }

                  RowLayout {
                    Label {
                      text: qsTr("Jump #2")
                    }
                    SpinBox {
                      Layout.preferredWidth: 70
                      from: 0
                      to: 10000
                      stepSize: 1
                      value: NSettings.value("Jump2") * 10
                      onValueModified: NSettings.setValue("Jump2", value / 10.0)
                      textFromValue: function (value, locale) {
                        return Number(value / 10.0).toLocaleString(locale, 'f', 1);
                      }
                      valueFromText: function (text, locale) {
                        return Number.fromLocaleString(locale, text) * 10;
                      }
                    }
                  }

                  RowLayout {
                    Label {
                      text: qsTr("Jump #3")
                    }
                    SpinBox {
                      Layout.preferredWidth: 70
                      from: 0
                      to: 10000
                      stepSize: 1
                      value: NSettings.value("Jump3") * 10
                      onValueModified: NSettings.setValue("Jump3", value / 10.0)
                      textFromValue: function (value, locale) {
                        return Number(value / 10.0).toLocaleString(locale, 'f', 1);
                      }
                      valueFromText: function (text, locale) {
                        return Number.fromLocaleString(locale, text) * 10;
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
                      Layout.preferredWidth: 70
                      from: 0
                      to: 10
                      stepSize: 1
                      value: NSettings.value("SpeedStep") * 100
                      onValueModified: NSettings.setValue("SpeedStep", value / 100.0)
                      textFromValue: function (value, locale) {
                        return Number(value / 100.0).toLocaleString(locale, 'f', 2);
                      }
                      valueFromText: function (text, locale) {
                        return Number.fromLocaleString(locale, text) * 100;
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
                      Layout.preferredWidth: 70
                      from: 0
                      to: 10
                      stepSize: 1
                      value: NSettings.value("PitchStep") * 100
                      onValueModified: NSettings.setValue("PitchStep", value / 100.0)
                      textFromValue: function (value, locale) {
                        return Number(value / 100.0).toLocaleString(locale, 'f', 2);
                      }
                      valueFromText: function (text, locale) {
                        return Number.fromLocaleString(locale, text) * 100;
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

    NTab {
      title: qsTr("Plugins")
      visible: NPluginModel.some(item => item.plugins.length > 1)

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
