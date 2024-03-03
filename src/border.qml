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

import QtQuick 2.15

Item {
  property alias size: rect.border.width
  property alias color: rect.border.color
  anchors.fill: parent
  Rectangle {
    id: rect
    anchors.fill: parent
    border {
      width: 10
      color: "red"
    }
    color: "transparent"
  }
}
