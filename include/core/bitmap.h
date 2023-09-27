#pragma once
#include <core/fwd.h>
struct Bitmap
{
    /// Supported image file formats
    enum class FileFormat
    {
        PNG,
        OpenEXR,
        JPEG,
        Unknown,
        Auto
    };

    Bitmap() {}
    Bitmap(const Spectrum &value)
    {
        fill(value);
    }
    Bitmap(const char *filename)
    {
        load(filename);
    }
    Bitmap(const ArrayXd &data, const Vector2i &res);

    Bitmap operator+(const Bitmap &rhs) const
    {
        Bitmap ret = *this;
        for (int i = 0; i < m_data.size(); ++i)
            ret.m_data[i] += rhs.m_data[i];
        return ret;
    }

    Bitmap &operator+=(const Bitmap &rhs)
    {
        for (int i = 0; i < m_data.size(); ++i)
            m_data[i] += rhs.m_data[i];
        return *this;
    }
    void setZero();
    std::string toString() const;
    void load(const char *filename);
    void save(const char *filename) const;
    void fill(const Spectrum &value);
    Spectrum eval(const Vector2 &_uv, bool flip_v = true) const;
    ArrayXd getData() const;
    void setData(const ArrayXd &data);
    int width() const { return m_res.x(); }
    int height() const { return m_res.y(); }

    Vector2i m_res;
    std::vector<Spectrum> m_data;
};
