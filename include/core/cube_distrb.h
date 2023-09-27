#pragma once
#include <core/bitmap.h>
#include <core/fwd.h>
#include <core/pmf.h>

struct CubeDistribution {
    void set_resolution(const Array2i &res);
    void set_mass(const Bitmap &pmf);

    Float sample_reuse(Vector2 &samples) const;
    Float pdf(const Vector2 &p) const;

    DiscreteDistribution m_distrb;

    Array2i         m_res;
    int             m_num_cells;
    Eigen::ArrayX2i m_cells;
    Vector2         m_unit;
    bool            m_ready = false;
};