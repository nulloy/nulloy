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

#ifndef N_WAVEFORM_BUILDER_GSTREAMER_H
#define N_WAVEFORM_BUILDER_GSTREAMER_H

#include <gst/gst.h>

#include "abstractWaveformBuilder.h"
#include "global.h"
#include "plugin.h"
#include "waveformBuilderInterface.h"

class QTimer;

class NWaveformBuilderGstreamer : public NWaveformBuilderInterface,
                                  public NPlugin,
                                  public NAbstractWaveformBuilder
{
    Q_OBJECT
    Q_INTERFACES(NWaveformBuilderInterface NPlugin)

private:
    GstElement *m_playbin;
    QString m_currentFile;
    QTimer *m_timer;
    qreal position() const;

private slots:
    void update();

public:
    NWaveformBuilderGstreamer(QObject *parent = NULL) : NWaveformBuilderInterface(parent) {}
    ~NWaveformBuilderGstreamer();

    void init();
    QString interfaceString() const { return NWaveformBuilderInterface::interfaceString(); }
    N::PluginType type() const { return N::WaveformBuilder; }

    void start(const QString &file);
    void stop();
    void positionAndIndex(float &pos, int &index)
    {
        NAbstractWaveformBuilder::positionAndIndex(pos, index);
    }
    NWaveformPeaks *peaks() { return NAbstractWaveformBuilder::peaks(); }

    void handleBuffer(gint16 *pcmBuffer, int nChannels, int nSamples);
};

#endif
