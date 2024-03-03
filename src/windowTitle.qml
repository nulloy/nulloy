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

NFadeOut {
  property Component itemDelegate: Text {}

  Loader {
    id: loader
    anchors.centerIn: parent.width >= loader.width ? parent : undefined
    anchors.left: parent.width >= loader.width ? undefined : parent.left
    anchors.verticalCenter: parent.verticalCenter
    sourceComponent: itemDelegate
  }

  function findTextItem(item) {
    for (let i = 0; i < item.children.length; ++i) {
      let child = item.children[i];
      if (child instanceof Text) {
        return child;
      } else if (child.children.length > 0) {
        return findTextItem(child);
      }
    }
  }

  Component.onCompleted: {
    let textItem = findTextItem(loader);
    if (textItem) {
      textItem.text = Qt.binding(() => {
        return mainWindow.title;
      });
    }
  }
}
