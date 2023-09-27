#pragma once
#include <core/fwd.h>
#include <core/logger.h>

#include <vector>

struct Distribution2D {
    Distribution2D() = default;
    // take a 2D discrete distribution
    Distribution2D(const std::vector<Float> &data, const Vector2i &res) {
        m_res           = res;
        size_t nEntries = (m_res.x() + 1) * m_res.y();
        m_cdfCols.resize(nEntries);
        m_pdfRows.resize(m_res.y());
        m_cdfRows.resize(m_res.y() + 1);

        /** build a marginal & conditional cdf over luminances weighted by
         * sin(theta) */
        m_cdfRows[0] = 0.;
        Float rowSum = 0.0f;
        for (int y = 0; y < m_res.y(); y++) {
            m_cdfCols[y * (m_res.x() + 1)] = 0;
            Float colSum                   = 0;
            for (int x = 0; x < m_res.x(); x++) {
                Float val = data[y * m_res.x() + x];
                colSum += val;
                m_cdfCols[y * (m_res.x() + 1) + x + 1] = colSum;
            }

            Float normalization = 1. / colSum;
            for (int x = 0; x < m_res.x(); x++) {
                m_cdfCols[y * (m_res.x() + 1) + x + 1] *= normalization;
            }

            rowSum += colSum;
            m_pdfRows[y]     = colSum;
            m_cdfRows[y + 1] = rowSum;
        }
        Float normalization = 1. / rowSum;
        for (int y = 0; y < m_res.y(); y++) {
            m_cdfRows[y + 1] *= normalization;
        }
        m_cdfRows[m_res.y()] = 1.;

        if (rowSum == 0)
            PSDR_ERROR("Distribution2D: sum of all entries is zero!");
        if (!std::isfinite(rowSum))
            PSDR_ERROR("Distribution2D: sum of all entries is not finite!");
    }

    /** first sample the marginal row distribution, then sample the conditional
     * column distribution */
    inline std::pair<uint32_t, Float>
    sampleReuse(const Float *cdf, uint32_t size, Float &sample) const {
        // sample
        const Float *entry = std::lower_bound(cdf, cdf + size + 1, sample);
        // clamp
        uint32_t index = std::min(
            (uint32_t) std::max((ptrdiff_t) 0, entry - cdf - 1), size - 1);
        // reuse
        sample = (sample - cdf[index]) / (cdf[index + 1] - cdf[index]);
        return { index, cdf[index + 1] - cdf[index] };
    }

    // return the warped sample and associated pdf
    std::pair<Vector2, Float> sample(const Vector2 &_rnd) const {
        Vector2 rnd(_rnd);
        auto [row, pdf_row] = sampleReuse(m_cdfRows.data(), m_res.y(), rnd.y());
        auto [col, pdf_col] = sampleReuse(
            m_cdfCols.data() + row * (m_res.x() + 1), m_res.x(), rnd.x());
        Vector2 p = Vector2(col, row) + rnd;
        return { p.array() / m_res.cast<Float>().array(),
                 pdf_col * pdf_row * m_res.prod() };
    }

    std::vector<Float> pdfRows() const { return m_pdfRows; }
    std::vector<Float> cdfRows() const { return m_cdfRows; }
    std::vector<Float> cdfCols() const { return m_cdfCols; }

    std::vector<Float> m_pdfRows; // marginal pdf
    std::vector<Float> m_cdfRows; // marginal cdf
    std::vector<Float> m_cdfCols; // conditional cdf
    Vector2i           m_res;
};