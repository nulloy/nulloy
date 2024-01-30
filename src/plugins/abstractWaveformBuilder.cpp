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

#include "abstractWaveformBuilder.h"

#include <QCryptographicHash>
#include <QObject>
#include <QtCore>

#include "common.h"

NAbstractWaveformBuilder::NAbstractWaveformBuilder()
{
    m_cacheLoaded = false;
    m_cacheFile = NCore::rcDir() + "/" + NCore::applicationBinaryName() + ".peaks";
}

NAbstractWaveformBuilder::~NAbstractWaveformBuilder() {}

void NAbstractWaveformBuilder::cacheLoad()
{
    QFile cache(m_cacheFile);

    if (m_cacheLoaded || !cache.exists()) {
        return;
    }

    QByteArray compressed;
    cache.open(QIODevice::ReadOnly);
    QDataStream inFile(&cache);
    inFile >> compressed;
    cache.close();

    QByteArray buffer = qUncompress(compressed);
    QDataStream inBuffer(&buffer, QIODevice::ReadOnly);

    QList<QByteArray> hashes;
    QList<NWaveformPeaks> peaks;
    inBuffer >> hashes >> peaks >> m_dateHash;

    Q_ASSERT(hashes.count() == peaks.count());
    m_peaksCache.clear();
    for (int i = 0; i < hashes.count(); ++i) {
        m_peaksCache.insert(hashes.at(i), new NWaveformPeaks(peaks.at(i)));
    }

    m_cacheLoaded = true;
}

void NAbstractWaveformBuilder::cacheSave()
{
    QByteArray buffer;
    QDataStream outBuffer(&buffer, QIODevice::WriteOnly);

    QList<QByteArray> hashes = m_peaksCache.keys();
    QList<NWaveformPeaks> peaks;
    foreach (QByteArray hash, hashes) {
        peaks << *m_peaksCache.object(hash);
    }

    outBuffer << hashes << peaks << m_dateHash;
    QByteArray compressed = qCompress(buffer);

    QFile cache(m_cacheFile);
    QDataStream outFile(&cache);
    cache.open(QIODevice::WriteOnly);
    outFile << compressed;
    cache.close();
}

bool NAbstractWaveformBuilder::peaksFindFromCache(const QString &file)
{
    cacheLoad();
    if (!m_cacheLoaded) {
        return false;
    }

    QDir dir(QFileInfo(m_cacheFile).absolutePath());
    QString path = dir.relativeFilePath(QFileInfo(file).absoluteFilePath());

    QByteArray pathHash = QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Sha1);
    QString modifDate = m_dateHash.value(pathHash);
    if (modifDate.isEmpty()) {
        return false;
    }

    if (modifDate == QFileInfo(file).lastModified().toString(Qt::ISODate)) {
        NWaveformPeaks *peaks = m_peaksCache.object(pathHash);
        if (peaks) {
            m_peaks = NWaveformPeaks(*peaks);
            return true;
        } else {
            m_dateHash.remove(pathHash);
            return false;
        }
    } else {
        m_peaksCache.remove(pathHash);
        m_dateHash.remove(pathHash);
        return false;
    }
}

void NAbstractWaveformBuilder::peaksAppendToCache(const QString &file)
{
    if (!m_peaks.isCompleted()) {
        return;
    }

    QDir dir(QFileInfo(m_cacheFile).absolutePath());
    QString path = dir.relativeFilePath(QFileInfo(file).absoluteFilePath());

    QByteArray pathHash = QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Sha1);
    m_peaksCache.insert(pathHash, new NWaveformPeaks(m_peaks));
    m_dateHash.insert(pathHash, QFileInfo(file).lastModified().toString(Qt::ISODate));

    cacheSave();
}

void NAbstractWaveformBuilder::reset()
{
    m_peaks.reset();
    m_oldIndex = 0;
    m_oldPos = 0.0;
}

void NAbstractWaveformBuilder::positionAndIndex(float &pos, int &index)
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
