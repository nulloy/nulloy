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

#include "waveformPeaks.h"

#include <QDebug>
#include <QtMath>

#define MAX_RES 2048
#define DEFAULT_SAMPLE_RATE 44100
#define LOW_MID_CROSSOVER 250.0   // Hz
#define MID_HIGH_CROSSOVER 3000.0 // Hz

NBiquad::NBiquad()
{
    // pass-through until set up
    m_b0 = 1;
    m_b1 = m_b2 = m_a1 = m_a2 = 0;
    clear();
}

void NBiquad::clear()
{
    m_x1 = m_x2 = m_y1 = m_y2 = 0;
}

void NBiquad::setup(Type type, double sampleRate, double frequency, double q)
{
    double w0 = 2 * M_PI * frequency / sampleRate;
    double cosW0 = qCos(w0);
    double alpha = qSin(w0) / (2 * q);
    double a0 = 1 + alpha;

    if (type == LowPass) {
        m_b0 = (1 - cosW0) / 2;
        m_b1 = 1 - cosW0;
        m_b2 = (1 - cosW0) / 2;
    } else {
        m_b0 = (1 + cosW0) / 2;
        m_b1 = -(1 + cosW0);
        m_b2 = (1 + cosW0) / 2;
    }
    m_a1 = -2 * cosW0;
    m_a2 = 1 - alpha;

    m_b0 /= a0;
    m_b1 /= a0;
    m_b2 /= a0;
    m_a1 /= a0;
    m_a2 /= a0;

    clear();
}

NWaveformPeaks::NWaveformPeaks()
{
    reset();
}

void NWaveformPeaks::reset()
{
    m_index = 0;
    m_factor = 1024;
    m_factor_k = 2;
    m_counter = 0;
    m_completed = false;

    m_vector = QVector<QPair<qreal, qreal>>(MAX_RES, qMakePair(0.0, 0.0));
    m_bands = QVector<NWaveformBands>(MAX_RES);

    m_sampleRate = 0;
    setSampleRate(DEFAULT_SAMPLE_RATE);
}

void NWaveformPeaks::setSampleRate(int rate)
{
    if (rate <= 0 || rate == m_sampleRate) {
        return;
    }
    m_sampleRate = rate;

    // keep crossovers below Nyquist for very low sample rates
    double nyquist = rate / 2.0;
    double lowMid = qMin(LOW_MID_CROSSOVER, nyquist * 0.45);
    double midHigh = qMin(MID_HIGH_CROSSOVER, nyquist * 0.9);

    m_lowFilter.setup(NBiquad::LowPass, rate, lowMid);
    m_midHighPass.setup(NBiquad::HighPass, rate, lowMid);
    m_midLowPass.setup(NBiquad::LowPass, rate, midHigh);
    m_highFilter.setup(NBiquad::HighPass, rate, midHigh);
}

int NWaveformPeaks::size() const
{
    if (m_completed) {
        return m_vector.size();
    } else {
        return m_index;
    }
}

void NWaveformPeaks::complete()
{
    m_completed = true;
    m_vector.resize(m_index + 1);
    m_bands.resize(m_index + 1);
}

qreal NWaveformPeaks::positive(int index) const
{
    return m_vector[index].first;
}

qreal NWaveformPeaks::negative(int index) const
{
    return m_vector[index].second;
}

void NWaveformPeaks::append(qreal value)
{
    if (m_completed) {
        qWarning() << "WaveformPeaks::append() : cannot append to completed.";
        return;
    }

    if (m_index == m_vector.size() - 1) {
        m_factor *= m_factor_k;
#if defined(QT_DEBUG) && !defined(QT_NO_DEBUG)
        qDebug() << "WaveformPeaks   ::"
                 << "resized  " << m_factor;
#endif

        qreal max;
        qreal min;
        int i;
        for (i = 0; i < m_vector.size() / m_factor_k; ++i) {
            max = 0;
            min = 0;
            NWaveformBands bands;
            for (int j = 0; j < m_factor_k; ++j) {
                int k = i * m_factor_k + j;
                max = qMax(m_vector[k].first, max);
                min = qMin(m_vector[k].second, min);
                bands.low = qMax(m_bands[k].low, bands.low);
                bands.mid = qMax(m_bands[k].mid, bands.mid);
                bands.high = qMax(m_bands[k].high, bands.high);
            }
            m_vector[i].first = max;
            m_vector[i].second = min;
            m_bands[i] = bands;
        }

        m_counter = 0;
        m_index = i;

        for (i = m_index; i < m_vector.size(); ++i) {
            m_vector[i].first = 0;
            m_vector[i].second = 0;
            m_bands[i] = NWaveformBands();
        }
    }

    if (m_counter < m_factor) {
        ++m_counter;
    } else {
        m_counter = 0;
        ++m_index;
    }

    m_vector[m_index].first = qMax(m_vector[m_index].first, value);
    m_vector[m_index].second = qMin(m_vector[m_index].second, value);

    float low = qAbs(m_lowFilter.process(value));
    float mid = qAbs(m_midLowPass.process(m_midHighPass.process(value)));
    float high = qAbs(m_highFilter.process(value));
    NWaveformBands &bands = m_bands[m_index];
    bands.low = qMax(bands.low, low);
    bands.mid = qMax(bands.mid, mid);
    bands.high = qMax(bands.high, high);
}
