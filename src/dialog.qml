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
import QtQuick.Window

ApplicationWindow {
  id: root
  visible: false
  modality: Qt.ApplicationModal

  property alias standardButtons: buttonBox.standardButtons
  property alias buttonBox: buttonBox
  property alias footerExtra: footerLeft.data

  function standardButton(button) {
    return buttonBox.standardButton(button);
  }

  signal closed
  signal rejected
  signal accepted

  //Shortcut {
  //  sequence: StandardKey.Cancel
  //  onActivated: root.close()
  //}

  onVisibleChanged: {
    if (!visible) {
      closed();
      //} else {
      //  // FIXME: flickers:
      //  x = NDialogHandler.parentCenterX - width / 2;
      //  y = NDialogHandler.parentCenterY - height / 2;
    }
  }

  footer: Pane {
    padding: 5
    contentItem: RowLayout {
      spacing: 10

      Row {
        id: footerLeft
        Layout.alignment: Qt.AlignVCenter
      }

      Item {
        Layout.fillWidth: true
      }

      DialogButtonBox {
        id: buttonBox
        objectName: "buttonBox"
        onAccepted: root.close()
        onRejected: root.close()
      }
    }
  }
}
