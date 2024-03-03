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
import QtQuick.Controls 2.15
import NSvgImage 1.0

Button {
  id: button
  property alias source: image.source
  property string elementId: ""
  property string elementIdHovered: elementId
  property string elementIdPressed: elementId
  property alias image: image

  background: Rectangle {
    color: "transparent"
  }

  NSvgImage {
    id: image
    anchors.fill: parent
    elementId: button.pressed ? button.elementIdPressed : (button.hovered ? button.elementIdHovered : button.elementId)
  }

  onDoubleClicked: button.clicked()
}
