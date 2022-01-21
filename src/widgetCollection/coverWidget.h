/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2022 Sergey Vlasov <sergey@vlasov.me>
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

#ifndef N_COVER_WIDGET_H
#define N_COVER_WIDGET_H

#include <QLabel>

class NCoverReaderInterface;
class NCoverWidgetPopup;

class NCoverWidget : public QLabel
{
    Q_OBJECT

private:
    NCoverReaderInterface *m_coverReader;
    QPixmap m_pixmap;
    NCoverWidgetPopup *m_popup;

    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void changeEvent(QEvent *event);
    void fitToHeight(int height);
    void init();

public:
    NCoverWidget(QWidget *parent = 0);
    ~NCoverWidget();

public slots:
    void setSource(const QString &file);
};

#endif
