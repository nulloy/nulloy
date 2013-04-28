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

#ifndef N_WAVEFORM_BUILDER_PHONON_H
#define N_WAVEFORM_BUILDER_PHONON_H

#include "pluginElementInterface.h"
#include "waveformBuilderInterface.h"
#include "abstractWaveformBuilder.h"

#include <phonon/audiooutput.h>
#include <phonon/audiodataoutput.h>
#include <phonon/mediaobject.h>
#include <QTimer>

class NWaveformBuilderPhonon :	public NWaveformBuilderInterface,
								public NPluginElementInterface,
								public NAbstractWaveformBuilder
{
	Q_OBJECT
	Q_INTERFACES(NWaveformBuilderInterface NPluginElementInterface)

private:
	Phonon::MediaObject *m_mediaObject;
	Phonon::AudioOutput *m_audioOutput;
	Phonon::AudioDataOutput *m_audioDataOutput;

	QString m_currentFile;
	QTimer *m_timer;
	qreal position();

public:
	NWaveformBuilderPhonon(QObject *parent = NULL) : NWaveformBuilderInterface(parent) {}
	~NWaveformBuilderPhonon();
	void init();
	QString interface() { return NWaveformBuilderInterface::interface(); }
	PluginType type() { return WaveformBuilder; }

	void start(const QString &file);
	void stop();
	void positionAndIndex(float &pos, int &index) { NAbstractWaveformBuilder::positionAndIndex(pos, index); }
	NWaveformPeaks* peaks() { return NAbstractWaveformBuilder::peaks(); }

private slots:
	void update();
	void handleData(const QMap< Phonon::AudioDataOutput::Channel, QVector<qint16> > &data);
};

#endif

/* vim: set ts=4 sw=4: */
