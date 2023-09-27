#include <core/cube_distrb.h>
#include <core/logger.h>
#include <core/utils.h>

/**
 * \brief Set the resolution of the cube distribution, need to call set_mass after this
 * initialize m_num_cells, m_res, m_unit, m_cells
 */
void CubeDistribution::set_resolution(const Array2i &res) {
    Eigen::Array<int64_t, 2, 1> prod_res;
    prod_res[1] = res[1];
    prod_res[0] = res[0] * prod_res[1];
    PSDR_ASSERT(prod_res[0] < std::numeric_limits<int>::max());

    m_num_cells = static_cast<int>(prod_res[0]);
    m_res       = res;
    // reciprocal
    m_unit = res.cast<Float>().inverse().matrix();
    m_cells.resize(m_num_cells, 2);
    auto cur = m_cells.col(1); // reference
    // cur = arange(m_num_cells)
    cur = Eigen::ArrayXi::LinSpaced(m_num_cells, 0, m_num_cells - 1);

    for (int i = 0; i < 1; ++i) {
        int            denorm = static_cast<int>(prod_res[i + 1]);
        m_cells.col(i)        = cur / denorm;
        cur -= m_cells.col(i) * denorm;
    }
}

void CubeDistribution::set_mass(const Bitmap &radiance) {
    int width  = m_res[0];
    int height = m_res[1];

    int            n      = width * height;
    Eigen::ArrayXi idx    = Eigen::ArrayXi::LinSpaced(n, 0, n - 1);
    Eigen::ArrayXi mod    = idx - (idx / height) * height;
    Eigen::ArrayXd thetas = (mod.cast<Float>() + 0.5) * (M_PI / static_cast<Float>(height));

    ArrayXd pmf(n);
    for (int i = 0; i < m_cells.rows(); i++) {
        Vector2i cell  = m_cells.row(i).matrix();
        Float    theta = thetas[i];
        Vector2  uv    = (cell.cast<Float>() + Vector2(0.5, 0.5)).cwiseProduct(m_unit);
        Spectrum val   = radiance.eval(uv, false);
        pmf[i]         = rgb2luminance(val) * sin(theta); // NOTE reference
    }

    m_distrb = DiscreteDistribution(pmf);

    m_ready = true;
}

Float CubeDistribution::sample_reuse(Vector2 &samples) const {
    PSDR_ASSERT(m_ready);
    Float pdf;
    int   idx = m_distrb.sampleReuse(samples[1], pdf);
    samples += m_cells.row(idx).cast<Float>().matrix();
    samples = samples.cwiseProduct(m_unit);
    return pdf * static_cast<Float>(m_num_cells);
}

Float CubeDistribution::pdf(const Vector2 &p) const {
    PSDR_ASSERT(m_ready);
    Eigen::Array2i ip = (p.array() * m_res.cast<Float>()).floor().cast<int>();
    if (not (ip[0] >= 0 && ip[0] < m_res[0] && ip[1] >= 0 && ip[1] < m_res[1])) {
        printf("warning cube distribution ip: %d %d, res: %d %d", ip[0], ip[1], m_res[0], m_res[1]);
        return 0;
    }
    // PSDR_ASSERT(ip[0] >= 0 && ip[0] < m_res[0] && ip[1] >= 0 && ip[1] < m_res[1]);
    int idx = ip[0];
    for (int i = 1; i < 2; ++i) {
        idx = idx * m_res[i] + ip[i];
    }
    return m_distrb[idx] * static_cast<Float>(m_num_cells);
}