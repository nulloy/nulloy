/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2025 Sergey Vlasov <sergey@vlasov.me>
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
import NImage 1.0

NDialog {
  id: dialog

  title: Qt.application.name + " " + qsTr("Tag Editor")
  width: 750
  height: 450
  minimumWidth: 500
  minimumHeight: 300

  standardButtons: Dialog.Close | Dialog.Apply | Dialog.Reset

  SystemPalette {
    id: systemPalette
    colorGroup: SystemPalette.Active
  }

  component TagTextField: TextField {
    id: textField
    required property string tagName
    property bool hasSingleValue: NTags[tagName] === undefined ? true : NTags[tagName].length == 1
    property bool hasNewLines: /\n/.test(NTags[tagName] === undefined ? '' : NTags[tagName].join(''))
    property bool editable: hasSingleValue && !hasNewLines
    Layout.fillWidth: true

    text: NTags[tagName].join(' / ').replace(/\n/g, ' / ')
    readOnly: !editable
    enabled: editable

    ToolTip.visible: hovered && !editable
    ToolTip.delay: 500
    ToolTip.text: qsTr("This tag contains multiple values or lines and can be only edited in Raw Tags tab.")

    onTextEdited: {
      if (!editable) {
        return;
      }
      let oldValue = NTags[tagName];
      let newValue = [text];
      if (JSON.stringify(newValue) === JSON.stringify(oldValue)) {
        return;
      }
      NTags[tagName] = newValue;
    }
  }

  Component.onCompleted: {
    let enabled = () => !NTagEditorDialogHandler.readOnly && NTagEditorDialogHandler.modifiedAndUnsaved;

    let resetButton = dialog.standardButton(Dialog.Reset);
    resetButton.text = qsTr("Revert");
    resetButton.enabled = Qt.binding(enabled);

    let applyButton = dialog.standardButton(Dialog.Apply);
    applyButton.text = qsTr("Save");
    applyButton.enabled = Qt.binding(enabled);
  }

  RowLayout {
    anchors.right: parent.right
    anchors.top: parent.top
    anchors.margins: 10

    Label {
      visible: NTagEditorDialogHandler.readOnly
      text: qsTr("Read-only mode")
      color: 'red'
    }

    Button {
      visible: NTagEditorDialogHandler.readOnly
      text: qsTr("Edit as UTF-8")
      onClicked: NTagEditorDialogHandler.editAsUtf8()
    }

    Button {
      visible: NTagEditorDialogHandler.readOnly
      text: qsTr("Switch to UTF-8")
      onClicked: NTagEditorDialogHandler.switchEncodingToUtf8()
    }

    Label {
      text: qsTr("Encoding:")
    }

    ComboBox {
      id: encodingComboBox
      Layout.preferredWidth: 180
      model: NTagEditorDialogHandler.encodings
      currentIndex: NTagEditorDialogHandler.encodingCurrentIndex
      onActivated: {
        NTagEditorDialogHandler.encodingCurrentIndex = currentIndex;
      }

      // FIXME: Qt bug? why is this needed? property binding does not work
      Connections {
        target: NTagEditorDialogHandler
        function onEncodingCurrentIndexChanged() {
          encodingComboBox.currentIndex = NTagEditorDialogHandler.encodingCurrentIndex;
        }
      }

      MouseArea {
        anchors.fill: parent
        propagateComposedEvents: true
        onPressed: mouse => mouse.accepted = false
        onWheel: wheel => {
          let oldIndex = encodingComboBox.currentIndex;
          if (wheel.angleDelta.y > 0) {
            if (encodingComboBox.currentIndex > 0) {
              --encodingComboBox.currentIndex;
            }
          } else {
            if (encodingComboBox.currentIndex < encodingComboBox.count - 1) {
              ++encodingComboBox.currentIndex;
            }
          }

          if (oldIndex !== encodingComboBox.currentIndex) {
            NTagEditorDialogHandler.encodingCurrentIndex = encodingComboBox.currentIndex;
          }
        }
      }
    }
  }

  NTabView {
    id: tabView
    anchors.fill: parent

    NTab {
      title: qsTr("General")
      padding: 10

      SplitView {
        clip: true

        handle: Rectangle {
          implicitWidth: 7
          color: "transparent"
        }

        GridLayout {
          enabled: !NTagEditorDialogHandler.readOnly
          SplitView.preferredWidth: parent.width * 0.55
          SplitView.fillHeight: true
          columns: 2

          Label {
            text: qsTr("Track #:")
          }
          RowLayout {
            TagTextField {
              tagName: "TRACKNUMBER"
            }

            Label {
              text: qsTr("BPM:")
            }
            TagTextField {
              tagName: "BPM"
            }
          }

          Label {
            text: qsTr("Title:")
          }
          TagTextField {
            tagName: "TITLE"
          }

          Label {
            text: qsTr("Artist:")
          }
          TagTextField {
            tagName: "ARTIST"
          }

          Label {
            text: qsTr("Album:")
          }
          TagTextField {
            tagName: "ALBUM"
          }

          Label {
            text: qsTr("Date:")
          }
          RowLayout {
            TagTextField {
              Layout.preferredWidth: 50
              tagName: "DATE"
            }

            Label {
              text: qsTr("Genre:")
            }
            TagTextField {
              tagName: "GENRE"
            }
          }

          Label {
            text: qsTr("Comment:")
            Layout.alignment: Qt.AlignTop
          }

          NScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            background: Rectangle {
              border.color: commentTextArea.activeFocus ? systemPalette.highlight : systemPalette.mid
              color: systemPalette.base
            }

            TextArea {
              id: commentTextArea
              Layout.fillWidth: true
              Layout.fillHeight: true
              leftPadding: 5
              rightPadding: 5
              topPadding: 5
              bottomPadding: 5

              property bool hasSingleValue: NTags.COMMENT === undefined ? true : NTags.COMMENT.length == 1
              property bool editable: hasSingleValue
              enabled: editable

              ToolTip.visible: hovered && !editable
              ToolTip.delay: 500
              ToolTip.text: qsTr("This tag contains multiple values and can be only edited in Raw Tags tab.")

              text: NTags.COMMENT.join("\n")
              // TODO: TextArea has textEdited signal since Qt 6.9.
              // onEditingFinished is triggered only when focus goes away, not triggered when e.g. Save is clicked.
              onEditingFinished: {
                let oldValue = NTags.COMMENT;
                let newValue = [text];
                if (JSON.stringify(newValue) === JSON.stringify(oldValue)) {
                  return;
                }

                NTags.COMMENT = newValue;
              }
            }
          }
        }

        GridLayout {
          enabled: !NTagEditorDialogHandler.readOnly
          SplitView.preferredWidth: parent.width * 0.45
          SplitView.fillHeight: true
          columns: 2

          Label {
            text: qsTr("Publisher:")
          }
          TagTextField {
            tagName: "PUBLISHER"
          }

          Label {
            text: qsTr("Composer:")
          }
          TagTextField {
            tagName: "COMPOSER"
          }

          Label {
            text: qsTr("Copyright:")
          }
          TagTextField {
            tagName: "COPYRIGHT"
          }

          Label {
            text: qsTr("URL:")
          }
          TagTextField {
            tagName: "URL"
          }

          Label {
            text: qsTr("Encoded by:")
          }
          TagTextField {
            tagName: "ENCODEDBY"
          }

          Label {
            text: qsTr("Artwork:")
          }

          Rectangle {
            id: artworkWrapper
            color: systemPalette.base

            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.fillHeight: true

            property int tileWidth: 100
            property int tileHeight: 100
            property int spacing: 5

            NScrollView {
              anchors.fill: parent

              Grid {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: artworkWrapper.spacing
                columns: Math.floor((width - artworkWrapper.spacing) / (artworkWrapper.tileWidth + artworkWrapper.spacing))
                padding: 5

                Repeater {
                  model: NTagEditorDialogHandler.coverImages()
                  NImage {
                    image: modelData
                    height: artworkWrapper.tileHeight
                    width: artworkWrapper.tileWidth
                  }
                }
              }
            }

            Rectangle {
              anchors.fill: parent
              border.color: systemPalette.mid
              color: "transparent"
            }
          }
        }
      }
    }

    NTab {
      title: qsTr("Raw Tags")

      NScrollView {
        GridLayout {
          id: grid
          Layout.margins: 10
          Layout.bottomMargin: 5
          Layout.fillWidth: true
          enabled: !NTagEditorDialogHandler.readOnly
          columns: 2

          property var valueSplittedTags: {
            let arr = [];
            for (let tagName of NTags.keys().sort()) {
              for (let valueIndex = 0; valueIndex < NTags[tagName].length; ++valueIndex) {
                arr.push({
                  tagName: tagName,
                  valueIndex: valueIndex,
                  value: NTags[tagName][valueIndex]
                });
              }
            }
            return arr;
          }

          Repeater {
            model: grid.valueSplittedTags

            Label {
              Layout.row: index
              Layout.column: 0
              text: modelData.tagName + (NTags[modelData.tagName].length > 1 ? (" " + (index)) : "") + ":"
            }
          }

          Repeater {
            model: grid.valueSplittedTags

            TextArea {
              Layout.row: index
              Layout.column: 1
              Layout.fillWidth: true
              Layout.preferredHeight: contentHeight + topPadding + bottomPadding

              leftPadding: 5
              rightPadding: 5

              text: modelData.value

              // TODO: TextArea has textEdited signal since Qt 6.9.
              // onEditingFinished is triggered only when focus goes away, not triggered when e.g. Save is clicked.
              onEditingFinished: {
                let oldValue = NTags[modelData.tagName][modelData.valueIndex];
                let newValue = text;
                if (JSON.stringify(newValue) === JSON.stringify(oldValue)) {
                  return;
                }
                NTags[modelData.tagName][modelData.valueIndex] = newValue;
              }

              background: Rectangle {
                border.color: parent.activeFocus ? systemPalette.highlight : systemPalette.mid
                color: systemPalette.base
              }
            }
          }

          Item {
            Layout.fillHeight: true
          }
        }
      }
    }
  }
}
