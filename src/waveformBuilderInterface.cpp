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

#include "waveformBuilderInterface.h"
#include "core.h"
#include "settings.h"
#include <QCryptographicHash>

NWaveformBuilderInterface::NWaveformBuilderInterface(QObject *parent) : QThread(parent)
{
	m_cacheLoaded = FALSE;
	m_cacheFile = NCore::rcDir() + "/" + NCore::applicationBinaryName() + ".peaks";
}

NWaveformBuilderInterface::~NWaveformBuilderInterface() {}

void NWaveformBuilderInterface::cacheLoad()
{
	QFile cache(m_cacheFile);
	if (!m_cacheLoaded && cache.exists()) {
		QByteArray compressed;
		cache.open(QIODevice::ReadOnly);
		QDataStream inFile(&cache);
		inFile >> compressed;
		cache.close();

		QByteArray buffer = qUncompress(compressed);
		QDataStream inBuffer(&buffer, QIODevice::ReadOnly);
		inBuffer >> m_peaksCache >> m_dateHash;

		m_cacheLoaded = TRUE;
	}
}

void NWaveformBuilderInterface::cacheSave()
{
	QByteArray buffer;
	QDataStream outBuffer(&buffer, QIODevice::WriteOnly);
	outBuffer << m_peaksCache << m_dateHash;
	QByteArray compressed = qCompress(buffer);

	QFile cache(m_cacheFile);
	QDataStream outFile(&cache);
	cache.open(QIODevice::WriteOnly);
	outFile << compressed;
	cache.close();
}

bool NWaveformBuilderInterface::peaksFindFromCache(const QString &file)
{
	cacheLoad();
	if (!m_cacheLoaded)
		return FALSE;

	QFileInfo info = QFileInfo(file);

	QString path = info.absoluteFilePath();
	QByteArray pathHash = QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Sha1);
	QString modifDate = m_dateHash.value(pathHash);
	if (modifDate.isEmpty())
		return FALSE;

	if (modifDate == info.lastModified().toString(Qt::ISODate)) {
		m_peaks = *m_peaksCache.object(pathHash);
		return TRUE;
	} else {
		m_peaksCache.remove(pathHash);
		m_dateHash.remove(pathHash);
		return FALSE;
	}
}

void NWaveformBuilderInterface::peaksAppendToCache(const QString &file)
{
	if (!m_peaks.isCompleted())
		return;

	QFileInfo info = QFileInfo(file);
	QString path = info.absoluteFilePath();
	QByteArray pathHash = QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Sha1);
	m_peaksCache.insert(pathHash, &m_peaks);
	m_dateHash.insert(pathHash, info.lastModified().toString(Qt::ISODate));

	cacheSave();
}

void NWaveformBuilderInterface::reset()
{
	m_peaks.reset();
	m_oldIndex = 0;
	m_oldPos = 0.0;
}

void NWaveformBuilderInterface::positionAndIndex(float &pos, int &index)
{
	if (m_peaks.isCompleted()) {
		pos = 1.0;
		index = m_peaks.size();
		return;
	}

	float newPos = position();
	if (newPos != m_oldPos) {
		m_oldIndex = m_peaks.size();
		m_oldPos = newPos;
	}

	pos = m_oldPos;
	index = m_oldIndex;
}

/* vim: set ts=4 sw=4: */
