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

#ifndef N_MAIN_WINDOW_H
#define N_MAIN_WINDOW_H

#include <QDialog>

class NMainWindow : public QDialog
{
	Q_OBJECT

private:
	bool m_dragActive;
	bool m_skinEnabled;
	QPoint m_dragPoint;
	QString m_styleSheet;

	bool eventFilter(QObject *obj, QEvent *event);
	void closeEvent(QCloseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void resizeEvent(QResizeEvent *event);

public:
	NMainWindow(QWidget *parent = 0);
	~NMainWindow();
	void init(const QString &uiFile);

public slots:
	void saveSettings();
	void loadSettings();
	void setTitle(QString title);
	void minimize();
	void toggleVisibility();

signals:
	void closed();
	void resized();
	void newTitle(const QString &title);
};

#endif

/* vim: set ts=4 sw=4: */
