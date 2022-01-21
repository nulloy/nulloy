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

#ifndef N_WAVEFORM_PEAKS_H
#define N_WAVEFORM_PEAKS_H

#include <QDataStream>
#include <QPair>
#include <QVector>

class NWaveformPeaks
{
private:
    QVector<QPair<qreal, qreal>> m_vector;
    bool m_completed;
    int m_index;
    int m_factor;
    int m_factor_k;
    int m_counter;

public:
    NWaveformPeaks();
    void reset();
    void append(qreal value);
    void complete();
    bool isCompleted() const { return m_completed; }
    int size() const;
    qreal positive(int index) const;
    qreal negative(int index) const;

    friend inline QDataStream &operator<<(QDataStream &out, const NWaveformPeaks &p)
    {
        out << p.m_vector << p.m_index << p.m_completed;
        return out;
    }

    friend inline QDataStream &operator>>(QDataStream &in, NWaveformPeaks &p)
    {
        p.m_vector.clear();
        in >> p.m_vector >> p.m_index >> p.m_completed;
        return in;
    }
};

#endif
