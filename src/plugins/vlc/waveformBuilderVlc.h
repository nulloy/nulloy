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

#ifndef N_WAVEFORM_BUILDER_VLC_H
#define N_WAVEFORM_BUILDER_VLC_H

#include <vlc/vlc.h>

#include <QTimer>

#include "abstractWaveformBuilder.h"
#include "plugin.h"
#include "waveformBuilderInterface.h"

class NWaveformBuilderVlc : public NWaveformBuilderInterface,
                            public NPlugin,
                            public NAbstractWaveformBuilder
{
    Q_OBJECT
    Q_INTERFACES(NWaveformBuilderInterface NPlugin)

private:
    libvlc_instance_t *m_vlcInstance;
    libvlc_media_player_t *m_mediaPlayer;
    QString m_currentFile;

    QByteArray m_pcmBuffer;
    QTimer *m_timer;
    qreal position() const;

public:
    NWaveformBuilderVlc(QObject *parent = NULL) : NWaveformBuilderInterface(parent) {}
    ~NWaveformBuilderVlc();
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

    void prepareBuffer(uint8_t **pcmBuffer, unsigned int size);
    void handleBuffer(uint8_t *pcmBuffer, unsigned int nChannels, unsigned int nSamples);

private slots:
    void update();
};

#endif
