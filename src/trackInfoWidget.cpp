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

#include "trackInfoWidget.h"

#include <QGraphicsOpacityEffect>
#include <QLayout>
#include <QMouseEvent>
#include <QPropertyAnimation>

#include "label.h"
#include "pluginLoader.h"
#include "settings.h"
#include "trackInfoReader.h"

NTrackInfoWidget::~NTrackInfoWidget() {}

NTrackInfoWidget::NTrackInfoWidget(QFrame *parent) : QFrame(parent)
{
    m_trackInfoReader = NULL;

    QStringList vNames = QStringList() << "Top"
                                       << "Middle"
                                       << "Bottom";
    QStringList hNames = QStringList() << "Left"
                                       << "Center"
                                       << "Right";
    QVBoxLayout *vLayout = new QVBoxLayout;
    for (int i = 0; i < vNames.count(); ++i) {
        QWidget *hContainer = new QWidget;
        hContainer->setAttribute(Qt::WA_TransparentForMouseEvents);
        QHBoxLayout *hLayout = new QHBoxLayout;
        hLayout->setContentsMargins(0, 0, 0, 0);
        hLayout->setSpacing(0);
        hContainer->setLayout(hLayout);
        if (i > 0) {
            QSpacerItem *vSpacer = new QSpacerItem(20, 0, QSizePolicy::Minimum,
                                                   QSizePolicy::Expanding);
            vLayout->addItem(vSpacer);
        }
        for (int j = 0; j < hNames.count(); ++j) {
            if (j > 0) {
                QSpacerItem *hSpacer = new QSpacerItem(40, 14, QSizePolicy::Expanding,
                                                       QSizePolicy::Minimum);
                hLayout->addItem(hSpacer);
            }
            NLabel *label = new NLabel;
            label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
            label->setObjectName(vNames.at(i) + hNames.at(j));
            label->setAlignment(Qt::AlignHCenter);
            hLayout->addWidget(label);
        }
        vLayout->addWidget(hContainer);
    }
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    m_container = new QWidget;
    m_container->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_container->setLayout(vLayout);
    layout->addWidget(m_container);

    m_effect = new QGraphicsOpacityEffect(this);
    m_effect->setOpacity(1.0);
    setGraphicsEffect(m_effect);

    m_animation = new QPropertyAnimation(m_effect, "opacity", this);
    m_animation->setDuration(150);
    m_animation->setEasingCurve(QEasingCurve::OutQuad);
    m_animation->setStartValue(1.0);
    m_animation->setEndValue(0.0);

    setMouseTracking(true);
    QList<NLabel *> labels = findChildren<NLabel *>();
    foreach (NLabel *label, labels) {
        label->setAttribute(Qt::WA_TransparentForMouseEvents);
        label->hide();
    }
    hide();

    m_msec = 0;
    m_heightThreshold = minimumSizeHint().height();

    loadSettings();
}

void NTrackInfoWidget::setTrackInfoReader(NTrackInfoReader *reader)
{
    m_trackInfoReader = reader;
}

void NTrackInfoWidget::enterEvent(QEvent *)
{
#ifndef Q_OS_MAC // QTBUG-15367
    m_animation->setDirection(QAbstractAnimation::Forward);
    if (m_animation->state() == QAbstractAnimation::Stopped) {
        m_animation->start();
    }
#else
    m_container->hide();
#endif
}

void NTrackInfoWidget::leaveEvent(QEvent *)
{
    m_container->show();
    m_animation->setDirection(QAbstractAnimation::Backward);
    if (m_animation->state() == QAbstractAnimation::Stopped) {
        m_animation->start();
    }
}

void NTrackInfoWidget::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    bool toShrink = (m_heightThreshold > height());
    QVBoxLayout *vLayout = dynamic_cast<QVBoxLayout *>(m_container->layout());
    Q_ASSERT(vLayout);

    vLayout->itemAt(0)->widget()->setHidden(toShrink); // top
    vLayout->itemAt(4)->widget()->setHidden(toShrink); // bottom

    QFrame::resizeEvent(event);
}

bool NTrackInfoWidget::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        emit showToolTip("");
    }

    return QFrame::event(event);
}

void NTrackInfoWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!rect().contains(event->pos()) || m_tooltipFormat.isEmpty()) {
        emit showToolTip("");
        return;
    }

    QString text = m_tooltipFormat;
    if (m_tooltipFormat.isEmpty()) {
        return;
    }

    QString seconds = m_trackInfoReader->getInfo('D');
    if (seconds.isEmpty()) {
        return;
    }

    float mouse_pos = (float)event->x() / width();
    int seconds_at_mouse_pos = seconds.toInt() * mouse_pos;
    // time position under mouse pointer:
    text.replace("%C", NTrackInfoReader::formatTime(seconds_at_mouse_pos));

    int seconds_elapsed = m_msec / 1000;
    int seconds_delta = seconds_at_mouse_pos - seconds_elapsed;
    QString delta_formatted = NTrackInfoReader::formatTime(qAbs(seconds_delta));
    // time offset under mouse pointer:
    text.replace("%o", QString("%1%2").arg(seconds_delta < 0 ? "-" : "+").arg(delta_formatted));

    if (text.isEmpty()) {
        return;
    }

    emit showToolTip(text);
}

void NTrackInfoWidget::updateFileLabels(const QString &file)
{
    if (!m_trackInfoReader) {
        return;
    }
    m_trackInfoReader->setSource(file);

    if (!QFileInfo(file).exists()) {
        hide();
        return;
    } else {
        show();
    }

    QString encoding = NSettings::instance()->value("EncodingTrackInfo").toString();
    foreach (NLabel *label, m_fileLabelsMap.keys()) {
        QString format = m_fileLabelsMap[label];
        QString text = m_trackInfoReader->toString(format);

        label->setText(text);
        label->setVisible(!text.isEmpty());
    }
}

void NTrackInfoWidget::updatePlaylistLabels()
{
    if (!m_trackInfoReader) {
        return;
    }

    foreach (NLabel *label, m_playlistLabelsMap.keys()) {
        QString format = m_playlistLabelsMap[label];
        QString text = m_trackInfoReader->toString(format);

        label->setText(text);
        label->setVisible(!text.isEmpty());
    }
}

void NTrackInfoWidget::loadSettings()
{
    m_fileLabelsMap.clear();
    m_playbackLabelsMap.clear();
    m_playlistLabelsMap.clear();

    QList<NLabel *> labels = findChildren<NLabel *>();
    for (int i = 0; i < labels.size(); ++i) {
        NLabel *label = labels.at(i);
        QString format = NSettings::instance()->value("TrackInfo/" + label->objectName()).toString();

        if (format.contains("%T") || format.contains("%r")) { // elapsed or remaining playback time
            m_playbackLabelsMap[label] = format;
        }

        if (format.contains("%L")) { // playlist duration
            m_playlistLabelsMap[label] = format;
        }

        m_fileLabelsMap[label] = format;
    }

    m_tooltipFormat = NSettings::instance()->value("TooltipTrackInfo").toString();
}

void NTrackInfoWidget::updatePlaybackLabels(qint64 msec)
{
    m_msec = msec;

    m_trackInfoReader->updatePlaybackPosition(msec / 1000);
    foreach (NLabel *label, m_playbackLabelsMap.keys()) {
        QString format = m_playbackLabelsMap[label];
        QString text = m_trackInfoReader->toString(format);
        label->setText(text);
        label->setVisible(!text.isEmpty());
    }
}
