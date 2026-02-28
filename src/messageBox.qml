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
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

NDialog {
  id: dialog
  title: NMessageBoxHandler.title
  standardButtons: NMessageBoxHandler.standardButtons
  width: footer.implicitWidth
  height: contentHeight
  minimumHeight: contentHeight
  minimumWidth: footer.implicitWidth

  readonly property real contentHeight: Math.min(textArea.implicitHeight + footer.implicitHeight, 200)

  onAccepted: NMessageBoxHandler.closed(true)
  onRejected: NMessageBoxHandler.closed(false)

  TextArea {
    id: textArea
    anchors.fill: parent
    Layout.fillWidth: true
    Layout.fillHeight: true
    readOnly: true
    wrapMode: Text.WordWrap
    text: NMessageBoxHandler.text
    onLinkActivated: Qt.openUrlExternally(link)
  }

  footerExtra: CheckBox {
    visible: NMessageBoxHandler.checkBoxText !== ""
    text: NMessageBoxHandler.checkBoxText
    checked: NMessageBoxHandler.checkBoxChecked
    onCheckedChanged: NMessageBoxHandler.checkBoxChecked = checked
  }
}
