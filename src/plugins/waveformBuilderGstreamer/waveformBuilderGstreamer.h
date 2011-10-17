/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2011 Sergey Vlasov <sergey@vlasov.me>
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

#include "pluginInterface.h"
#include "waveformBuilderInterface.h"
#include "abstractWaveformBuilder.h"

#include <gst/gst.h>

#if defined Q_WS_WIN || defined Q_WS_MAC
#include <QTimer>
#endif

class NWaveformBuilderGstreamer :	public NWaveformBuilderInterface,
									public NPluginInterface,
									public NAbstractWaveformBuilder
{
	Q_OBJECT
	Q_INTERFACES(NWaveformBuilderInterface NPluginInterface)

private:
	GstElement *m_playbin;
	QString m_currentFile;
	qreal position();

#if defined Q_WS_WIN || defined Q_WS_MAC
private:
	QTimer *m_timer;
private slots:
	void update();
#endif

public:
	NWaveformBuilderGstreamer(QObject *parent = NULL) : NWaveformBuilderInterface(parent) {}
	~NWaveformBuilderGstreamer();

	void init();
	QString identifier() { return "Nulloy/Waveform/GStreamer/0.3.2"; }
	QString interface() { return NWaveformBuilderInterface::interface(); }

	void start(const QString &file);
	void stop();
	void positionAndIndex(float &pos, int &index) { NAbstractWaveformBuilder::positionAndIndex(pos, index); }
	NWaveformPeaks* peaks() { return NAbstractWaveformBuilder::peaks(); }

	void handleBuffer(gint16 *pcmBuffer, int nChannels, int nSamples);
};

#endif

/* vim: set ts=4 sw=4: */
