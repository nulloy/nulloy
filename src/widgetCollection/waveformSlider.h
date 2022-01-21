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

#ifndef N_WAVEFORM_SLIDER_H
#define N_WAVEFORM_SLIDER_H

#include <QAbstractSlider>
#include <QPainter>
#include <QVector>

class NPlaylistDataItem;
class NWaveformBuilderInterface;

class NWaveformSlider : public QAbstractSlider
{
    Q_OBJECT
    // clang-format off
    Q_PROPERTY(int radius READ radius WRITE setRadius)
    Q_PROPERTY(QBrush background READ background WRITE setBackground)
    Q_PROPERTY(QBrush wave_background READ waveBackground WRITE setWaveBackground)
    Q_PROPERTY(QColor wave_border_color READ waveBorderColor WRITE setWaveBorderColor)
    Q_PROPERTY(QBrush progress_playing_background READ progressPlayingBackground WRITE setProgressPlayingBackground)
    Q_PROPERTY(QBrush progress_paused_background READ progressPausedBackground WRITE setProgressPausedBackground)
    Q_PROPERTY(QBrush remaining_playing_background READ remainingPlayingBackground WRITE setRemainingPlayingBackground)
    Q_PROPERTY(QBrush remaining_paused_background READ remainingPausedBackground WRITE setRemainingPausedBackground)
    Q_PROPERTY(QString playing_composition READ playingComposition WRITE setPlayingComposition)
    Q_PROPERTY(QString paused_composition READ pausedComposition WRITE setPausedComposition)
    Q_PROPERTY(QColor groove_playing_background READ groovePlayingColor WRITE setGroovePlayingColor)
    Q_PROPERTY(QColor groove_paused_background READ groovePausedColor WRITE setGroovePausedColor)
    Q_PROPERTY(QColor file_drop_border_color READ fileDropBorderColor WRITE setFileDropBorderColor)
    Q_PROPERTY(QBrush file_drop_background READ fileDropBackground WRITE setFileDropBackground)
    // clang-format on

private:
    NWaveformBuilderInterface *m_waveBuilder;
    QImage m_backgroundImage;
    QImage m_progressPlayingImage;
    QImage m_progressPausedImage;
    QImage m_remainingPlayingImage;
    QImage m_remainingPausedImage;
    QTimer *m_timer;
    bool m_pausedState;
    QSize m_oldSize;
    int m_oldBuilderIndex;
    float m_oldBuilderPos;
    bool m_hasMedia;
    bool m_needsUpdate;

    void mousePressEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void changeEvent(QEvent *event);
    void init();

public:
    NWaveformSlider(QWidget *parent = 0);
    ~NWaveformSlider();
    QSize sizeHint() const;

public slots:
    void setMedia(const QString &file);
    void setPausedState(bool);
    void setValue(qreal value);

private slots:
    void checkForUpdate();
    void setValue(int){};

signals:
    void sliderMoved(qreal value);

    // DRAG & DROP >>
private:
    bool m_fileDrop;

protected:
    QStringList mimeTypes() const;
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
    virtual void dropEvent(QDropEvent *event);

signals:
    void filesDropped(const QList<NPlaylistDataItem> &dataItems);
    // << DRAG & DROP

    // STYLESHEET PROPERTIES >>
private:
    int m_radius;
    QBrush m_background;
    QBrush m_waveBackground;
    QColor m_waveBorderColor;
    QBrush m_progressPlayingBackground;
    QBrush m_progressPausedBackground;
    QBrush m_remainingPlayingBackground;
    QBrush m_remainingPausedBackground;
    QColor m_groovePlayingColor;
    QColor m_groovePausedColor;
    QPainter::CompositionMode m_playingComposition;
    QPainter::CompositionMode m_pausedComposition;
    QColor m_fileDropBorderColor;
    QBrush m_fileDropBackground;

public:
    int radius() const;
    void setRadius(int radius);

    QBrush background() const;
    void setBackground(QBrush brush);

    QBrush waveBackground() const;
    void setWaveBackground(QBrush brush);

    QColor waveBorderColor() const;
    void setWaveBorderColor(QColor color);

    QBrush progressPlayingBackground() const;
    void setProgressPlayingBackground(QBrush brush);

    QBrush progressPausedBackground() const;
    void setProgressPausedBackground(QBrush brush);

    QBrush remainingPlayingBackground() const;
    void setRemainingPlayingBackground(QBrush brush);

    QBrush remainingPausedBackground() const;
    void setRemainingPausedBackground(QBrush brush);

    QString playingComposition() const;
    void setPlayingComposition(const QString &mode);

    QString pausedComposition() const;
    void setPausedComposition(const QString &mode);

    QColor groovePlayingColor() const;
    void setGroovePlayingColor(QColor color);

    QColor groovePausedColor() const;
    void setGroovePausedColor(QColor color);

    QColor fileDropBorderColor() const;
    void setFileDropBorderColor(QColor color);

    QBrush fileDropBackground() const;
    void setFileDropBackground(QBrush brush);
    // << STYLESHEET PROPERTIES
};

#endif
