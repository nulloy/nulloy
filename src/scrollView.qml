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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

ScrollView {
  id: scrollView
  clip: true
  ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

  default property alias childItem: columnLayout.children

  Component.onCompleted: {
    contentItem.boundsBehavior = Flickable.StopAtBounds;
    contentItem.boundsMovement = Flickable.StopAtBounds;
  }

  ColumnLayout {
    id: columnLayout
    width: scrollView.availableWidth
    height: Math.max(columnLayout.implicitHeight, scrollView.height)
  }
}
