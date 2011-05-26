/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2011 Sergey Vlasov <sergey@vlasov.me>
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

NShortcutEditorWidget::NShortcutEditorWidget(QWidget *parent) : QTableWidget(parent)
{
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	setColumnCount(3);
	setHorizontalHeaderLabels(QStringList() << "Name" << "Description" << "Shortcut");

	verticalHeader()->setVisible(FALSE);

	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setSelectionMode(QAbstractItemView::SingleSelection);

	setStyleSheet("QTableView::item:disabled { color: black; }");
}

NShortcutEditorWidget::~NShortcutEditorWidget() {}

void NShortcutEditorWidget::init(const QList<NAction *> &actionList)
{
	m_actionList = actionList;
	setRowCount(m_actionList.size());
	for (int i = 0; i < m_actionList.size(); ++i) {
		NAction *action = m_actionList.at(i);
		QTableWidgetItem *nameItem = new QTableWidgetItem(action->icon(), action->text());
		nameItem->setFlags(Qt::NoItemFlags);
		nameItem->setData(Qt::UserRole, action->objectName());
		setItem(i, 0, nameItem);

		QTableWidgetItem *descriptionItem = new QTableWidgetItem(action->statusTip());
		descriptionItem->setFlags(Qt::NoItemFlags);
		setItem(i, 1, descriptionItem);

		QList<QKeySequence> shortcut = action->shortcuts();
		QStringList str;
		foreach (QKeySequence seq, shortcut)
			str << seq.toString();
		QTableWidgetItem *shortcutItem = new QTableWidgetItem(str.join(", "));
		setItem(i, 2, shortcutItem);
	}

	resizeColumnsToContents();
	horizontalHeader()->setStretchLastSection(TRUE);
}

void NShortcutEditorWidget::applyShortcuts()
{
	for (int i = 0; i < rowCount(); ++i) {
		QString objectName = item(i, 0)->data(Qt::UserRole).toString();
		QString shortcut = item(i, 2)->text();

		for (int j = 0; j < m_actionList.size(); j++) {
			if (objectName == m_actionList.at(j)->objectName()) {
				if (shortcut.isEmpty()) {
					m_actionList.at(j)->setEnabled(FALSE);
				} else {
					m_actionList.at(j)->setEnabled(TRUE);
					m_actionList.at(j)->setShortcut(QKeySequence(shortcut));
				}
			}
		}
	}
}

QString NShortcutEditorWidget::keyEventToString(QKeyEvent *e)
{
	int keyInt = e->key();
	QString seqStr = QKeySequence(e->key()).toString();
	bool modifiers = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier);

	if (!modifiers || seqStr.isEmpty() ||
		keyInt == Qt::Key_Control ||
		keyInt == Qt::Key_Alt || keyInt == Qt::Key_AltGr ||
		keyInt == Qt::Key_Shift)
	{
		return "";
	}

	QStringList strSequence;
	if (e->modifiers() & Qt::ControlModifier)
		strSequence << "Ctrl";
	if (e->modifiers() & Qt::AltModifier)
		strSequence << "Alt";
	if (e->modifiers() & Qt::ShiftModifier)
		strSequence << "Shift";

	return strSequence.join("+") + (strSequence.isEmpty() ? "" : "+") + seqStr;
}

void NShortcutEditorWidget::keyPressEvent(QKeyEvent *e)
{
	int keyInt = e->key();
	bool modifiers = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier);

	int row = selectionModel()->currentIndex().row();

	if (!modifiers && keyInt == Qt::Key_Delete)
		setItem(row, 2, new QTableWidgetItem(""));

	QString str = keyEventToString(e);
	if (str == "") {
		QTableWidget::keyPressEvent(e);
		return;
	}

	setItem(row, 2, new QTableWidgetItem(str));
}

/* vim: set ts=4 sw=4: */
