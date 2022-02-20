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

#ifndef N_TRACK_INFO_WIDGET_H
#define N_TRACK_INFO_WIDGET_H

#include <QFrame>
#include <QMap>

class QPropertyAnimation;
class QGraphicsOpacityEffect;
class NLabel;

class NTrackInfoWidget : public QFrame
{
    Q_OBJECT

private:
    qint64 m_msec;
    int m_heightThreshold;
    QString m_tooltipFormat;
    QMap<NLabel *, QString> m_staticFormatsMap;
    QMap<NLabel *, QString> m_dynamicFormatsMap;
    QGraphicsOpacityEffect *m_effect;
    QPropertyAnimation *m_animation;
    QWidget *m_container;
    int m_trackDurationSec;

    bool event(QEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

public:
    NTrackInfoWidget(QFrame *parent = 0);
    ~NTrackInfoWidget();

public slots:
    void updateStaticTags(const QString &file);
    void loadSettings();
    void tick(qint64 msec);

private slots:
    void showToolTip(int x, int y);
};

#endif
