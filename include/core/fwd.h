#pragma once

#include <Eigen/Dense>
#include <vector>

#include "config.h"
#include "macro.h"
#include "spectrum.h"
#include <variant>

// Specify calculation precision
#define DOUBLE_PRECISION

#ifdef DOUBLE_PRECISION
typedef double Float;
#define Epsilon 1e-9
#define FilterEpsilon 1e-12
#define M_PI 3.14159265358979323846
#define INV_PI 0.31830988618379067154
#define INV_TWOPI 0.15915494309189534561
#define INV_FOURPI 0.07957747154594766788
#else
typedef float Float;
#define Epsilon 1e-4f
#define M_PI 3.14159265358979323846f
#define INV_PI 0.31830988618379067154f
#define INV_TWOPI 0.15915494309189534561f
#define INV_FOURPI 0.07957747154594766788f
#endif

#define AD

typedef Eigen::Matrix<Float, 2, 1> Vector2;
typedef Eigen::Matrix<float, 2, 1> Vector2f;
typedef Eigen::Matrix<double, 2, 1> Vector2d;
typedef Eigen::Matrix<int, 2, 1> Vector2i;

typedef Eigen::Array<Float, 2, 1> Array2;
typedef Eigen::Array<float, 2, 1> Array2f;
typedef Eigen::Array<double, 2, 1> Array2d;
typedef Eigen::Array<int, 2, 1> Array2i;

typedef Eigen::Matrix<Float, 3, 1> Vector;
typedef Eigen::Matrix<Float, 3, 1> Vector3;
typedef Eigen::Matrix<float, 3, 1> Vector3f;
typedef Eigen::Matrix<double, 3, 1> Vector3d;
typedef Eigen::Matrix<int, 3, 1> Vector3i;

typedef Eigen::Array<Float, 3, 1> Array;
typedef Eigen::Array<Float, 3, 1> Array3;
typedef Eigen::Array<float, 3, 1> Array3f;
typedef Eigen::Array<double, 3, 1> Array3d;
typedef Eigen::Array<int, 3, 1> Array3i;

typedef Eigen::Matrix<Float, 4, 1> Vector4;
typedef Eigen::Matrix<float, 4, 1> Vector4f;
typedef Eigen::Matrix<double, 4, 1> Vector4d;
typedef Eigen::Matrix<int, 4, 1> Vector4i;

typedef Eigen::Array<Float, 4, 1> Array4;
typedef Eigen::Array<float, 4, 1> Array4f;
typedef Eigen::Array<double, 4, 1> Array4d;
typedef Eigen::Array<int, 4, 1> Array4i;

typedef Eigen::Matrix<Float, 3, 3> Matrix3x3;
typedef Eigen::Matrix<float, 3, 3> Matrix3x3f;
typedef Eigen::Matrix<double, 3, 3> Matrix3x3d;
typedef Eigen::Matrix<Float, 4, 4> Matrix4x4;
typedef Eigen::Matrix<float, 4, 4> Matrix4x4f;
typedef Eigen::Matrix<double, 4, 4> Matrix4x4d;
typedef Eigen::Matrix<Float, 2, 4> Matrix2x4;

typedef Eigen::Array<Float, 3, 1> Spectrum;
typedef Eigen::Array<float, 3, 1> Spectrum3f;
typedef Eigen::Array<double, 3, 1> Spectrum3d;

// typedef RGBSpectrum<Float, 1, 1> Spectrum;
// typedef RGBSpectrum<float, 1, 1> Spectrum3f;
// typedef RGBSpectrum<double, 1, 1> Spectrum3d;

typedef Eigen::ArrayXd ArrayXd;
typedef Eigen::Array<Float, -1, 3, Eigen::RowMajor> ArrayX3d;

template <typename T>
using ArrayT = Eigen::Array<T, -1, 1>;

#include <memory>
template <typename T>
using ref = std::shared_ptr<T>;

using ParamType = std::variant<const Float *,
                               const Vector *,
                               const Spectrum *,
                               const Matrix3x3 *,
                               const Matrix4x4 *,
                               const std::vector<Vector> *,
                               const std::vector<Spectrum> *>;

template <typename T>
auto flattened(const T &t)
{
    return Eigen::Map<const ArrayXd>(t.data(), t.size());
}

template <typename T>
auto flattened(T &t)
{
    return Eigen::Map<ArrayXd>(t.data(), t.size());
}

template <typename T1, typename T2>
auto reshaped(T1 t, int rows, int cols)
{
    assert(rows >= 0 || cols >= 0);
    if (rows >= 0 && cols >= 0)
    {
        assert(t.size() == rows * cols);
        return Eigen::Map<T2>(t.data(), rows, cols);
    }
    if (rows >= 0)
    {
        assert(t.size() % rows == 0);
        return Eigen::Map<T2>(t.data(), rows, t.size() / rows);
    }
    if (cols >= 0)
    {
        assert(t.size() % cols == 0);
        return Eigen::Map<T2>(t.data(), t.size() / cols, cols);
    }
    assert(false);
    return Eigen::Map<T2>(t.data(), rows, cols);
}
// const version
template <typename T>
auto reshaped(const ArrayXd &t, int rows, int cols)
{
    return reshaped<const ArrayXd &, const T>(t, rows, cols);
}
// non-const version
template <typename T>
auto reshaped(ArrayXd &t, int rows, int cols)
{
    return reshaped<ArrayXd &, T>(t, rows, cols);
}

template <typename T>
auto reshaped(const ArrayXd &t);

template <>
inline auto reshaped<ArrayX3d>(const ArrayXd &t)
{
    return reshaped<const ArrayX3d>(t, -1, 3);
}

template <typename T>
auto reshaped(ArrayXd &t);

template <>
inline auto reshaped<ArrayX3d>(ArrayXd &t)
{
    return reshaped<ArrayX3d>(t, -1, 3);
}

#if defined(ENZYME)
template <typename T>
T __enzyme_virtualreverse(T) __attribute__((weak));

template <typename... Args>
void __enzyme_autodiff(void *, Args...) __attribute__((weak));
template <typename... Args>
void *__enzyme_augmentfwd(void *, Args...) __attribute__((weak));
template <typename... Args>
void __enzyme_reverse(void *, Args...) __attribute__((weak));

template <typename... Args>
void __enzyme_fwddiff(void *, Args...) __attribute__((weak));
template <typename... Args>
double __enzyme_fwddiff_double(void *, Args...) __attribute__((weak));

extern int enzyme_dup __attribute__((weak)),
    enzyme_const __attribute__((weak)),
    enzyme_out __attribute__((weak)),
    enzyme_dupnoneed __attribute__((weak));
#endif

#define DEBUG(x)                                   \
    do                                             \
    {                                              \
        std::cerr << #x << ": " << x << std::endl; \
    } while (0)
