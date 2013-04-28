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

#ifndef N_WAVEFORM_BUILDER_VLC_H
#define N_WAVEFORM_BUILDER_VLC_H

#include "pluginElementInterface.h"
#include "waveformBuilderInterface.h"
#include "abstractWaveformBuilder.h"

#include <vlc/vlc.h>
#include <QTimer>

class NWaveformBuilderVlc :	public NWaveformBuilderInterface,
							public NPluginElementInterface,
							public NAbstractWaveformBuilder
{
	Q_OBJECT
	Q_INTERFACES(NWaveformBuilderInterface NPluginElementInterface)

private:
	libvlc_instance_t *m_vlcInstance;
	libvlc_media_player_t *m_mediaPlayer;
	QString m_currentFile;

	QByteArray m_pcmBuffer;
	QTimer *m_timer;
	qreal position();

public:
	NWaveformBuilderVlc(QObject *parent = NULL) : NWaveformBuilderInterface(parent) {}
	~NWaveformBuilderVlc();
	void init();
	QString interface() { return NWaveformBuilderInterface::interface(); }
	PluginType type() { return WaveformBuilder; }

	void start(const QString &file);
	void stop();
	void positionAndIndex(float &pos, int &index) { NAbstractWaveformBuilder::positionAndIndex(pos, index); }
	NWaveformPeaks* peaks() { return NAbstractWaveformBuilder::peaks(); }

	void prepareBuffer(uint8_t **pcmBuffer, unsigned int size);
	void handleBuffer(uint8_t *pcmBuffer, unsigned int nChannels, unsigned int nSamples);

private slots:
	void update();
};

#endif

/* vim: set ts=4 sw=4: */
