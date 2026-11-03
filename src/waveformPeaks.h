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

#ifndef N_WAVEFORM_PEAKS_H
#define N_WAVEFORM_PEAKS_H

#include <QDataStream>
#include <QPair>
#include <QVector>

// Per-bucket peak amplitude of the low / mid / high frequency bands.
struct NWaveformBands
{
    float low;
    float mid;
    float high;

    NWaveformBands() : low(0), mid(0), high(0) {}

    friend inline QDataStream &operator<<(QDataStream &out, const NWaveformBands &b)
    {
        out << b.low << b.mid << b.high;
        return out;
    }

    friend inline QDataStream &operator>>(QDataStream &in, NWaveformBands &b)
    {
        in >> b.low >> b.mid >> b.high;
        return in;
    }
};

// Direct form I biquad filter (RBJ Audio EQ Cookbook).
class NBiquad
{
private:
    double m_b0, m_b1, m_b2, m_a1, m_a2;
    double m_x1, m_x2, m_y1, m_y2;

public:
    enum Type { LowPass, HighPass };

    NBiquad();
    void setup(Type type, double sampleRate, double frequency, double q = 0.70710678);
    void clear();
    inline double process(double x)
    {
        double y = m_b0 * x + m_b1 * m_x1 + m_b2 * m_x2 - m_a1 * m_y1 - m_a2 * m_y2;
        m_x2 = m_x1;
        m_x1 = x;
        m_y2 = m_y1;
        m_y1 = y;
        return y;
    }
};

class NWaveformPeaks
{
private:
    QVector<QPair<qreal, qreal>> m_vector;
    QVector<NWaveformBands> m_bands;
    bool m_completed;
    int m_index;
    int m_factor;
    int m_factor_k;
    int m_counter;

    int m_sampleRate;
    NBiquad m_lowFilter;
    NBiquad m_midHighPass;
    NBiquad m_midLowPass;
    NBiquad m_highFilter;

public:
    NWaveformPeaks();
    void reset();
    void setSampleRate(int rate);
    void append(qreal value);
    void complete();
    bool isCompleted() const { return m_completed; }
    int size() const;
    qreal positive(int index) const;
    qreal negative(int index) const;
    qreal low(int index) const { return m_bands[index].low; }
    qreal mid(int index) const { return m_bands[index].mid; }
    qreal high(int index) const { return m_bands[index].high; }

    friend inline QDataStream &operator<<(QDataStream &out, const NWaveformPeaks &p)
    {
        out << p.m_vector << p.m_bands << p.m_index << p.m_completed;
        return out;
    }

    friend inline QDataStream &operator>>(QDataStream &in, NWaveformPeaks &p)
    {
        p.m_vector.clear();
        p.m_bands.clear();
        in >> p.m_vector >> p.m_bands >> p.m_index >> p.m_completed;
        return in;
    }
};

#endif
