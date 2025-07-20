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

#include "shortcutEditorWidget.h"

#include <QAction>
#include <QDebug>
#include <QHeaderView>
#include <QKeyEvent>

#include "action.h"

NShortcutEditorWidget::NShortcutEditorWidget(QWidget *parent) : QTableWidget(parent)
{
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setColumnCount(4);
    setHorizontalHeaderLabels(QStringList() << tr("Action") << tr("Description") << tr("Shortcut")
                                            << tr("Global Shortcut"));

    verticalHeader()->setVisible(false);

    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionMode(QAbstractItemView::SingleSelection);

    m_init = false;
}

NShortcutEditorWidget::~NShortcutEditorWidget() {}

void NShortcutEditorWidget::init(const QList<NAction *> &actionList)
{
    if (m_init) {
        return;
    }
    m_init = true;

    m_actionList = actionList;
    setRowCount(m_actionList.size());
    for (int i = 0; i < m_actionList.size(); ++i) {
        NAction *action = m_actionList.at(i);

        // also removes "..." at the end of shortcut text, if present:
        QTableWidgetItem *nameItem = new QTableWidgetItem(action->text().replace("...", ""));
        nameItem->setFlags(nameItem->flags() ^ Qt::ItemIsEditable ^ Qt::ItemIsSelectable);
        nameItem->setData(Qt::UserRole, action->objectName());
        setItem(i, Name, nameItem);

        QTableWidgetItem *descriptionItem = new QTableWidgetItem(action->statusTip());
        descriptionItem->setFlags(descriptionItem->flags() ^ Qt::ItemIsEditable ^
                                  Qt::ItemIsSelectable);
        setItem(i, Description, descriptionItem);

        setItem(i, Shortcut, new QTableWidgetItem(action->shortcuts().join(", ")));
        setItem(i, GlobalShortcut, new QTableWidgetItem(action->globalShortcuts().join(", ")));
    }

    resizeColumnToContents(Name);
    resizeColumnToContents(Description);
    horizontalHeader()->setSectionResizeMode(Name, QHeaderView::Fixed);
    horizontalHeader()->setSectionResizeMode(Description, QHeaderView::Fixed);
    horizontalHeader()->setSectionResizeMode(Shortcut, QHeaderView::Stretch);
    horizontalHeader()->setSectionResizeMode(GlobalShortcut, QHeaderView::Stretch);
    horizontalHeader()->setStretchLastSection(true);
}

void NShortcutEditorWidget::applyShortcuts()
{
    for (int i = 0; i < rowCount(); ++i) {
        QString objectName = item(i, Name)->data(Qt::UserRole).toString();
        foreach (NAction *action, m_actionList) {
            if (objectName != action->objectName()) {
                continue;
            }

            action->setShortcuts(item(i, Shortcut)->text().split(", "));
            action->setGlobalShortcuts(item(i, GlobalShortcut)->text().split(", "));
        }
    }
}

QString NShortcutEditorWidget::keyEventToString(QKeyEvent *e)
{
    int keyInt = e->key();
    QString seqStr = QKeySequence(e->key()).toString();

    // clang-format off
    if (seqStr.isEmpty() ||
        keyInt == Qt::Key_Control ||
        keyInt == Qt::Key_Alt || keyInt == Qt::Key_AltGr ||
        keyInt == Qt::Key_Meta ||
        keyInt == Qt::Key_Shift)
    {
        return "";
    }
    // clang-format on

    QStringList strSequence;
    if (e->modifiers() & Qt::ControlModifier) {
        strSequence << "Ctrl";
    }
    if (e->modifiers() & Qt::AltModifier) {
        strSequence << "Alt";
    }
    if (e->modifiers() & Qt::ShiftModifier) {
        strSequence << "Shift";
    }
    if (e->modifiers() & Qt::MetaModifier) {
        strSequence << "Meta";
    }

    return strSequence.join("+") + (strSequence.isEmpty() ? "" : "+") + seqStr;
}

void NShortcutEditorWidget::keyPressEvent(QKeyEvent *e)
{
    QTableWidgetItem *currentItem = item(currentRow(), currentColumn());
    if (!(currentItem->flags() & Qt::ItemIsEditable)) {
        return;
    }
    QString text = currentItem->text();

    int keyInt = e->key();
    bool modifiers = e->modifiers() &
                     (Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier | Qt::MetaModifier);

    if (!modifiers && (keyInt == Qt::Key_Delete || keyInt == Qt::Key_Backspace)) {
        currentItem->setText("");
        return;
    }

    QString shortcut = keyEventToString(e);
    if (shortcut == "") {
        QTableWidget::keyPressEvent(e);
        return;
    }

    if (text.split(", ").size() >= 3) {
        text = "";
    }

    if (!text.isEmpty()) {
        text += ", ";
    }

    currentItem->setText(text + shortcut);
}
