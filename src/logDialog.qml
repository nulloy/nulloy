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
import QtQuick.Dialogs

NDialog {
  title: Qt.application.name + " " + qsTr("Log")
  standardButtons: Dialog.Close
  width: 450
  height: 200
  minimumHeight: 100
  minimumWidth: footer.implicitWidth

  modality: Qt.NonModal

  NScrollView {
    anchors.fill: parent
    TextArea {
      Layout.fillWidth: true
      Layout.fillHeight: true
      readOnly: true
      textFormat: Text.RichText
      wrapMode: Text.WordWrap
      text: NLogDialogHandler.text
    }
  }

  footerExtra: CheckBox {
    text: qsTr("Don't show this dialog anymore")
    checked: !NSettings.value("DisplayLogDialog")
    onCheckedChanged: NSettings.setValue("DisplayLogDialog", !checked)
  }
}
