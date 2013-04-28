/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2013 Sergey Vlasov <sergey@vlasov.me>
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

#ifndef N_W7_TASK_BAR_H
#define N_W7_TASK_BAR_H

#ifdef WIN32

#include <QWidget>

class NW7TaskBar : public QObject
{
	Q_OBJECT

private:
	static NW7TaskBar *m_instance;

	NW7TaskBar(QObject *parent = 0);
	~NW7TaskBar();

public:
	enum State {
		NoProgress,
		Indeterminate,
		Normal,
		Error,
		Paused
	};

	static NW7TaskBar* instance();
	static void init(QObject *parent);
	static void setWindow(QWidget *window);
	static bool winEvent(MSG *message, long *result);
	static void setOverlayIcon(const QIcon &icon, const QString &text);

public slots:
	void setProgress(qreal val);
	void setState(State state);
};

#endif

#endif

/* vim: set ts=4 sw=4: */
