/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2014 Sergey Vlasov <sergey@vlasov.me>
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
#ifdef _N_NO_SKINS_
#include "ui_form.h"
#endif

class NMainWindow : public QDialog
{
	Q_OBJECT

private:
#ifdef _N_NO_SKINS_
	Ui::Dialog ui;
#endif
	bool m_dragActive;
	QPoint m_dragPoint;
	QPoint m_unmaximizedPos;
	QSize m_unmaximizedSize;
	bool m_isFullScreen;

	void changeEvent(QEvent *event);
	bool eventFilter(QObject *obj, QEvent *event);
	void closeEvent(QCloseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void resizeEvent(QResizeEvent *event);

#ifdef Q_WS_WIN
	bool m_framelessShadow;
	void updateFramelessShadow();
	bool winEvent(MSG *message, long *result);
#endif

public:
	NMainWindow(QWidget *parent = 0);
	~NMainWindow();
	void init(const QString &uiFile);
#ifdef Q_WS_WIN
	Q_INVOKABLE void setFramelessShadow(bool enabled);
#endif

public slots:
	void loadSettings();
	void saveSettings();
	void show();
	void toggleMaximize();
	void toggleFullScreen();
	void setTitle(QString title);
	void setOnTop(bool onTop);
	void reject() {};

signals:
	void closed();
	void resized();
	void newTitle(const QString &title);
	void fullScreenEnabled(bool enabled);
	void maximizeEnabled(bool enabled);
};

#endif

