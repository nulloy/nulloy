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

#ifndef N_VOLUME_SLIDER_H
#define N_VOLUME_SLIDER_H

#include "slider.h"

class NVolumeSlider : public NSlider
{
    Q_OBJECT

public:
    NVolumeSlider(QWidget *parent);

private:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *e);
    QString toolTipText(int value) const;

private slots:
    void on_valueChanged(int value);
    void showToolTip(int x, int y);
};

#endif
