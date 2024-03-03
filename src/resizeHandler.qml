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
import QtQuick.Window 2.15

MouseArea {
  property int border: 10
  property Window target: null
  hoverEnabled: true
  anchors.fill: parent
  cursorShape: {
    const pos = Qt.point(mouseX, mouseY);
    if (pos.x < border && pos.y < border) {
      return Qt.SizeFDiagCursor;
    }
    if (pos.x >= width - border && pos.y >= height - border) {
      return Qt.SizeFDiagCursor;
    }
    if (pos.x >= width - border && pos.y < border) {
      return Qt.SizeBDiagCursor;
    }
    if (pos.x < border && pos.y >= height - border) {
      return Qt.SizeBDiagCursor;
    }
    if (pos.x < border || pos.x >= width - border) {
      return Qt.SizeHorCursor;
    }
    if (pos.y < border || pos.y >= height - border) {
      return Qt.SizeVerCursor;
    }
  }
  onPressed: {
    const pos = Qt.point(mouseX, mouseY);
    let flags;
    if (pos.x < border) {
      flags |= Qt.LeftEdge;
    }
    if (pos.x >= width - border) {
      flags |= Qt.RightEdge;
    }
    if (pos.y < border) {
      flags |= Qt.TopEdge;
    }
    if (pos.y >= height - border) {
      flags |= Qt.BottomEdge;
    }
    if (flags !== undefined) {
      target.startSystemResize(flags);
    } else {
      mouse.accepted = false;
    }
  }
}
