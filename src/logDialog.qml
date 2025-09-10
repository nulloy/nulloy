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

import QtQuick 2.2
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.4
import QtQuick.Dialogs 1.2

NDialog {
  title: Qt.application.name + " " + qsTr("Log")
  standardButtons: Dialog.Close
  width: 450

  modality: Qt.NonModal

  ColumnLayout {
    anchors.fill: parent
    spacing: 20

    NScrollView {
      Layout.fillWidth: true
      Layout.fillHeight: true
      padding: 0

      TextEdit {
        Layout.fillWidth: true
        Layout.fillHeight: true
        readOnly: true
        selectByMouse: true
        textFormat: Text.RichText
        wrapMode: Text.WordWrap
        text: NLogDialogHandler.text
      }
    }

    CheckBox {
      Layout.fillWidth: true
      text: qsTr("Don't show this dialog anymore")
      checked: !NSettings.value("DisplayLogDialog")
      onCheckedChanged: NSettings.setValue("DisplayLogDialog", !checked)
    }
  }
}
