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

#include "logDialog.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QDebug>

NLogDialog::NLogDialog(QWidget *parent) : QDialog(parent)
{
	QVBoxLayout *layout = new QVBoxLayout(this);

	m_textBrowser = new QTextBrowser;
	m_textBrowser->setStyleSheet("QTextBrowser { background: transparent; }");
	m_textBrowser->setFrameShape(QFrame::NoFrame);
	layout->addWidget(m_textBrowser);

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));
	layout->addWidget(buttonBox);

	setLayout(layout);
	setMinimumWidth(500);
}

NLogDialog::~NLogDialog() {}

void NLogDialog::showMessage(QMessageBox::Icon icon, const QString &title, const QString &msg)
{
	if (!m_text.isEmpty())
		m_text.append("<br>");

	if (m_oldTitle != title) {
		if (!m_text.isEmpty())
			m_text.append("<br>");
		m_text.append("<b>" + title + "</b><br>");
		m_oldTitle = title;
	}

	switch (icon) {
	case QMessageBox::Critical:
		m_text.append("<span style=\"background-color: #ff0000\">Error</span>: ");
		break;
	case QMessageBox::Warning:
		m_text.append("<span style=\"background-color: #ffaa00\">Warning</span>: ");
		break;
	default:
		break;
	}

	m_text.append(msg);
	m_textBrowser->setHtml(m_text);

	QTextCursor cur = m_textBrowser->textCursor();
	cur.movePosition(QTextCursor::End);
	m_textBrowser->setTextCursor(cur);

	showNormal();
	activateWindow();
}

void NLogDialog::closeEvent(QCloseEvent *event)
{
	Q_UNUSED(event);

	m_textBrowser->clear();
	m_text.clear();
	m_oldTitle.clear();
}

/* vim: set ts=4 sw=4: */
