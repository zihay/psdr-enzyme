// Copyright @PSDR_AUTHORS 2022

#pragma once
#ifndef SPECTRUM_H__
#define SPECTRUM_H__

#include <Eigen/Dense>

#include <core/fwd.h>

template <typename T, int N, int M>
Eigen::Matrix<T, N, M> diagonal(T r)
{
  Eigen::Matrix<T, N, M> m = Eigen::Matrix<T, N, M>::Zero();
  for (int i = 0; i < std::min(N, M); ++i)
  {
    m(i, i) = r;
  }

  return m;
}

template <typename T, int N, int M>
class RGBSpectrum
{
public:
  RGBSpectrum()
  {
    c[0] = diagonal<T, N, M>(static_cast<T>(0.0f));
    c[1] = diagonal<T, N, M>(static_cast<T>(0.0f));
    c[2] = diagonal<T, N, M>(static_cast<T>(0.0f));
  }

  RGBSpectrum(T r)
  {
    c[0] = diagonal<T, N, M>(r);
    c[1] = diagonal<T, N, M>(r);
    c[2] = diagonal<T, N, M>(r);
  }

  RGBSpectrum(T r1, T r2, T r3)
  {
    c[0] = diagonal<T, N, M>(r1);
    c[1] = diagonal<T, N, M>(r2);
    c[2] = diagonal<T, N, M>(r3);
  }

  static RGBSpectrum Zero()
  {
    RGBSpectrum ret(static_cast<T>(0.0f));
    return ret;
  }

  static RGBSpectrum Ones()
  {
    RGBSpectrum ret(static_cast<T>(1.0f));
    return ret;
  }

  Eigen::Matrix<T, N, M> &operator[](int i)
  {
    return c[i];
  }

  Eigen::Matrix<T, N, M> operator[](int i) const
  {
    return c[i];
  }

  RGBSpectrum operator+(const RGBSpectrum &rhs) const
  {
    RGBSpectrum<T, N, M> ret = *this;
    for (int i = 0; i < 3; ++i)
    {
      ret[i] = ret[i] + rhs[i];
    }

    return ret;
  }

  RGBSpectrum &operator+=(const RGBSpectrum &rhs)
  {
    for (int i = 0; i < 3; ++i)
    {
      c[i] += rhs[i];
    }

    return *this;
  }

  RGBSpectrum operator-(const RGBSpectrum &rhs) const
  {
    RGBSpectrum<T, N, M> ret = *this;
    for (int i = 0; i < 3; ++i)
    {
      ret[i] = ret[i] - rhs[i];
    }

    return ret;
  }

  RGBSpectrum &operator-=(const RGBSpectrum &rhs)
  {
    for (int i = 0; i < 3; ++i)
    {
      c[i] -= rhs[i];
    }

    return *this;
  }

  RGBSpectrum operator*(const RGBSpectrum &rhs) const
  {
    RGBSpectrum<T, N, M> ret = *this;
    for (int i = 0; i < 3; ++i)
    {
      ret.c[i] = c[i] * rhs[i];
    }
    return ret;
  }

  RGBSpectrum operator*(T r) const
  {
    RGBSpectrum<T, N, M> ret = *this;
    for (int i = 0; i < 3; ++i)
    {
      ret[i] = r * ret[i];
    }

    return ret;
  }

  friend inline RGBSpectrum operator*(float lhs, const RGBSpectrum &rhs)
  {
    return rhs * lhs;
  }

  RGBSpectrum &operator*=(const RGBSpectrum &rhs)
  {
    for (int i = 0; i < 3; ++i)
    {
      c[i] = c[i] * rhs[i];
    }

    return *this;
  }

  RGBSpectrum operator/(const RGBSpectrum &rhs) const
  {
    RGBSpectrum ret = *this;
    for (int i = 0; i < 3; ++i)
    {
      ret[i] = (ret[i].array() / rhs[i].array()).matrix();
    }

    return ret;
  }

  RGBSpectrum operator/(T r) const
  {
    RGBSpectrum ret = *this;
    for (int i = 0; i < 3; ++i)
    {
      ret[i] /= r;
    }

    return ret;
  }

  RGBSpectrum &operator/=(const RGBSpectrum &rhs)
  {
    for (int i = 0; i < 3; ++i)
    {
      c[i] = (c[i].array() / rhs[i].array()).matrix();
    }

    return *this;
  }

  RGBSpectrum &operator/=(const T rhs)
  {
    for (int i = 0; i < 3; ++i)
    {
      c[i] = c[i] / rhs;
    }

    return *this;
  }

  void setZero()
  {
    for (int i = 0; i < 3; ++i)
    {
      c[i].setZero();
    }
  }

  T maxCoeff() const
  {
    T res = c[0].maxCoeff();
    for (int i = 0; i < 3; ++i)
    {
      res = std::max(res, c[i].maxCoeff());
    }

    return res;
  }

  RGBSpectrum cwiseProduct(const RGBSpectrum &rhs) const
  {
    RGBSpectrum ret;
    for (int i = 0; i < 3; ++i)
    {
      ret[i] = c[i].cwiseProduct(rhs[i]);
    }

    return ret;
  }

  RGBSpectrum<T, M, N> transpose() const
  {
    RGBSpectrum<T, M, N> ret;
    for (int i = 0; i < 3; ++i)
    {
      ret.c[i] = c[i].transpose();
    }

    return ret;
  }

  RGBSpectrum abs() const
  {
    RGBSpectrum ret = *this;
    for (int i = 0; i < 3; ++i)
    {
      ret[i] = ret[i].array().abs().matrix();
    }

    return ret;
  }

  RGBSpectrum sqrt() const
  {
    RGBSpectrum ret = *this;
    for (int i = 0; i < 3; ++i)
    {
      ret[i] = ret[i].array().sqrt().matrix();
    }

    return ret;
  }

  RGBSpectrum square() const
  {
    RGBSpectrum ret = *this;
    for (int i = 0; i < 3; ++i)
    {
      ret[i] = ret[i].array().square().matrix();
    }

    return ret;
  }

  bool isZero() const
  {
    bool res = true;
    for (int i = 0; i < 3; ++i)
    {
      res &= (this->c[i].isZero());
    }

    return res;
  }

  bool isZero(T eps) const
  {
    bool res = true;
    for (int i = 0; i < 3; ++i)
    {
      res &= (this->c[i].isZero(eps));
    }

    return res;
  }

  bool allFinite() const
  {
    bool res = true;
    for (int i = 0; i < 3; ++i)
    {
      res &= (this->c[i].allFinite());
    }

    return res;
  }
  bool isApprox(const RGBSpectrum &rhs, T eps = 1e-7) const
  {
    bool res = true;
    for (int i = 0; i < 3; ++i)
    {
      res &= (this->c[i].isApprox(rhs.c[i], eps));
    }

    return res;
  }

  T sum() const
  {
    T res = 0;
    for (int i = 0; i < 3; ++i)
    {
      res += this->c[i].sum();
    }
    return res;
  }

  Eigen::Array3d array() const
  {
    return Eigen::Array3d(c[0](0), c[1](0), c[2](0));
  }

  std::string toString() const
  {
    std::ostringstream oss;
    oss << "[ " << c[0] << ", " << c[1] << ", " << c[2] << " ]";
    return oss.str();
  }

  T *data() { return (T *)c; }
  size_t rows() const { return c->rows() * N * 3; }
  size_t cols() const { return c->cols() * M; }

protected:
  Eigen::Matrix<T, N, M> c[3];
};

#endif // SPECTRUM_H__
