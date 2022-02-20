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

#include "trackInfoWidget.h"

#include <QGraphicsOpacityEffect>
#include <QLayout>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QTime>
#include <QToolTip>

#include "label.h"
#include "pluginLoader.h"
#include "settings.h"
#include "tagReaderInterface.h"

NTrackInfoWidget::~NTrackInfoWidget() {}

NTrackInfoWidget::NTrackInfoWidget(QFrame *parent) : QFrame(parent)
{
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
    foreach (NLabel *label, labels)
        label->setAttribute(Qt::WA_TransparentForMouseEvents);

    m_msec = 0;
    m_heightThreshold = minimumSizeHint().height();

    loadSettings();
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
        QToolTip::hideText();
    }

    return QFrame::event(event);
}

void NTrackInfoWidget::mouseMoveEvent(QMouseEvent *event)
{
    showToolTip(event->x(), event->y());
}

void NTrackInfoWidget::updateStaticTags(const QString &file)
{
    NTagReaderInterface *tagReader = dynamic_cast<NTagReaderInterface *>(
        NPluginLoader::getPlugin(N::TagReader));
    if (!tagReader) {
        return;
    }

    QString encoding = NSettings::instance()->value("EncodingTrackInfo").toString();
    foreach (NLabel *label, m_staticFormatsMap.keys()) {
        QString format = m_staticFormatsMap[label];
        QString text = tagReader->toString(file, format, encoding);
        if (text.isEmpty()) { // reading tags failed
            hide();
            return;
        }

        label->setText(text);
    }

    m_trackDurationSec = tagReader->toString(file, "%D", encoding).toInt();

    show();
}

void NTrackInfoWidget::loadSettings()
{
    m_staticFormatsMap.clear();
    m_dynamicFormatsMap.clear();

    QList<NLabel *> labels = findChildren<NLabel *>();
    for (int i = 0; i < labels.size(); ++i) {
        NLabel *label = labels.at(i);
        QString format = NSettings::instance()->value("TrackInfo/" + label->objectName()).toString();
        if (format.contains("%T") || format.contains("%r")) {
            m_dynamicFormatsMap[label] = format;
        } else if (format.contains("%")) {
            m_staticFormatsMap[label] = format;
        }
        label->setVisible(!format.isEmpty());
    }

    m_tooltipFormat = NSettings::instance()->value("TooltipTrackInfo").toString();
}

void NTrackInfoWidget::tick(qint64 msec)
{
    m_msec = msec;

    QString encoding = NSettings::instance()->value("EncodingTrackInfo").toString();
    QTime total = QTime(0, 0).addSecs(m_trackDurationSec);
    QTime current = QTime(0, 0).addMSecs(msec);
    QTime remaining = total.addMSecs(-msec);
    int hours = m_trackDurationSec / 60 / 60;
    foreach (NLabel *label, m_dynamicFormatsMap.keys()) {
        QString text = m_dynamicFormatsMap[label];
        if (hours > 0) {
            text.replace("%d", total.toString("h:mm:ss"));
            text.replace("%T", current.toString("h:mm:ss"));
            text.replace("%r", remaining.toString("h:mm:ss"));
        } else {
            text.replace("%d", total.toString("m:ss"));
            text.replace("%T", current.toString("m:ss"));
            text.replace("%r", remaining.toString("m:ss"));
        }
        text.replace("%D", QString(m_trackDurationSec));
        label->setText(text);
        label->setVisible(!text.isEmpty());
    }
}

void NTrackInfoWidget::showToolTip(int x, int y)
{
    if (!rect().contains(QPoint(x, y)) || m_tooltipFormat.isEmpty()) {
        QToolTip::hideText();
        return;
    }

    QString text = m_tooltipFormat;
    if (m_tooltipFormat.isEmpty()) {
        return;
    }

    float posAtX = (float)x / width();
    int secAtX = m_trackDurationSec * posAtX;
    QTime timeAtX = QTime(0, 0).addSecs(secAtX);
    QString strAtPos;
    if (secAtX > 60 * 60) { // has hours
        strAtPos = timeAtX.toString("h:mm:ss");
    } else {
        strAtPos = timeAtX.toString("m:ss");
    }
    text.replace("%C", strAtPos);

    int secCur = m_msec / 1000;
    int secDiff = secAtX - secCur;
    QTime timeDiff = QTime(0, 0).addSecs(qAbs(secDiff));
    QString diffStr;
    if (qAbs(secDiff) > 60 * 60) { // has hours
        diffStr = timeDiff.toString("h:mm:ss");
    } else {
        diffStr = timeDiff.toString("m:ss");
    }
    text.replace("%o", QString("%1%2").arg(secDiff < 0 ? "-" : "+").arg(diffStr));

    QStringList offsetList = NSettings::instance()->value("TooltipOffset").toStringList();
    QToolTip::showText(mapToGlobal(QPoint(x, y) +
                                   QPoint(offsetList.at(0).toInt(), offsetList.at(1).toInt())),
                       text);
}
