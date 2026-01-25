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

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2

NDialog {
  title: NMessageBoxHandler.title
  standardButtons: NMessageBoxHandler.standardButtons
  width: 400

  onAccepted: NMessageBoxHandler.closed(true)
  onRejected: NMessageBoxHandler.closed(false)

  ColumnLayout {
    anchors.fill: parent
    spacing: 20

    TextEdit {
      Layout.fillWidth: true
      readOnly: true
      selectByMouse: true
      wrapMode: Text.WordWrap
      textFormat: Text.MarkdownText
      text: NMessageBoxHandler.text
      onLinkActivated: Qt.openUrlExternally(link)
    }

    CheckBox {
      Layout.fillWidth: true
      visible: NMessageBoxHandler.checkBoxText !== ""
      text: NMessageBoxHandler.checkBoxText
      checked: NMessageBoxHandler.checkBoxChecked
      onCheckedChanged: NMessageBoxHandler.checkBoxChecked = checked
    }
  }
}
