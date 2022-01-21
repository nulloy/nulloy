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

#ifndef N_COVER_WIDGET_POPUP_H
#define N_COVER_WIDGET_POPUP_H

#include <QLabel>

class QPropertyAnimation;
class QGraphicsOpacityEffect;

#define MARGIN 50

class NCoverWidgetPopup : public QWidget
{
    Q_OBJECT

private:
    QGraphicsOpacityEffect *m_effect;
    QPropertyAnimation *m_animation;
    QPixmap m_pixmap;
    QLabel *m_pixmapLabel;
    void mousePressEvent(QMouseEvent *);
    bool eventFilter(QObject *obj, QEvent *event);

public:
    void setPixmap(const QPixmap &pixmap);
    NCoverWidgetPopup(QWidget *parent = 0);
    void resize(const QSize &size);

private slots:
    void on_animation_finished();

public slots:
    void show();
};

#endif
