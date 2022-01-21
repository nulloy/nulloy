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

#ifndef N_ABSTRACT_WAVEFORM_BUILDER_H
#define N_ABSTRACT_WAVEFORM_BUILDER_H

#include <QHash>

#include "cache.h"
#include "waveformPeaks.h"

class QString;

class NAbstractWaveformBuilder
{
private:
    int m_oldIndex;
    float m_oldPos;
    bool m_cacheLoaded;
    QString m_cacheFile;

    void cacheLoad();
    void cacheSave();

protected:
    NWaveformPeaks m_peaks;
    NCache<QByteArray, NWaveformPeaks> m_peaksCache;
    QHash<QByteArray, QString> m_dateHash;

    virtual void reset();
    virtual qreal position() const = 0;
    bool peaksFindFromCache(const QString &file);
    void peaksAppendToCache(const QString &file);

public:
    NAbstractWaveformBuilder();
    ~NAbstractWaveformBuilder();

    NWaveformPeaks *peaks() { return &m_peaks; }
    void positionAndIndex(float &pos, int &index);
};

#endif
