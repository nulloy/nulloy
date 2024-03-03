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

function calculateMinimumHeight(item) {
  if (typeof ColumnLayout === 'undefined') {
    console.error('utils.js: QtQuick.Layouts definition is missing');
  }
  return _calculateMinimumHeight(item);
}

function _calculateMinimumHeight(item) {
  if (item.Layout.minimumHeight > 0) {
    return item.Layout.minimumHeight;
  }
  if (item instanceof ColumnLayout) {
    return Object.keys(item.children).reduce((acc, key) => {
      return acc + item.children[key].Layout.minimumHeight;
    }, 0) + item.spacing * (item.children.length - 1) + item.anchors.topMargin + item.anchors.bottomMargin;
  }
  return Object.keys(item.children).reduce((acc, key) => {
    return acc + _calculateMinimumHeight(item.children[key]);
  }, 0);
}

function calculateMinimumWidth(item) {
  if (typeof RowLayout === 'undefined') {
    console.error('utils.js: QtQuick.Layouts definition is missing');
  }
  return _calculateMinimumWidth(item);
}

function _calculateMinimumWidth(item) {
  if (item.Layout.minimumWidth > 0) {
    return item.Layout.minimumWidth;
  }
  if (item instanceof RowLayout) {
    return Object.keys(item.children).reduce((acc, key) => {
      return acc + item.children[key].Layout.minimumWidth;
    }, 0) + item.spacing * (item.children.length - 1) + item.anchors.leftMargin + item.anchors.rightMargin;
  }
  return Object.keys(item.children).reduce((acc, key) => {
    return acc + _calculateMinimumWidth(item.children[key]);
  }, 0);
}

function bound(min, val, max) {
  return Math.min(Math.max(min, val), max);
}

function findChild(parent, type) {
  if (parent instanceof Text) {
    return parent;
  }
  for (let i = 0; i < parent.children.length; ++i) {
    let found = findChild(parent.children[i], type)
    if (found) {
      return found;
    }
  }
  return null;
}
