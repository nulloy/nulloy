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

#include "waveformPeaks.h"

#include <QDebug>

#define MAX_RES 2048

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
            for (int j = 0; j < m_factor_k; ++j) {
                max = qMax(m_vector[i * m_factor_k + j].first, max);
                min = qMin(m_vector[i * m_factor_k + j].second, min);
            }
            m_vector[i].first = max;
            m_vector[i].second = min;
        }

        m_counter = 0;
        m_index = i;

        for (i = m_index; i < m_vector.size(); ++i) {
            m_vector[i].first = 0;
            m_vector[i].second = 0;
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
}
