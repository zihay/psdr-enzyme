#pragma once
#ifndef UTILITY_H__
#define UTILITY_H__

#include <core/fwd.h>
#include <core/ptr.h>
#include <core/ray.h>

inline std::string toString(const Spectrum &value)
{
    std::ostringstream oss;
    oss << "[ " << value[0] << ", " << value[1] << ", " << value[2] << " ]";
    return oss.str();
}

template <typename T>
inline T clamp(const T &v, const T &lo, const T &hi)
{
    if (v < lo)
        return lo;
    else if (v > hi)
        return hi;
    else
        return v;
}

template <typename T>
inline T square(const T &x) { return x * x; }
template <typename T>
inline T cubic(const T &x) { return x * x * x; }

Vector3 xfm_point(const Matrix4x4 &T, const Vector3 &pos);
Vector3 xfm_vector(const Matrix4x4 &T, const Vector3 &vec);

Vector2 squareToUniformDiskConcentric(const Vector2 &sample);
Vector squareToCosineHemisphere(const Vector2 &sample);
Vector squareToCosineSphere(const Vector2 &sample);
Vector squareToUniformSphere(const Vector2 &sample);

Float squareToCosineHemispherePdf(const Vector &d);
Float squareToCosineSpherePdf(const Vector &d);

void coordinateSystem(const Vector &n, Vector &s, Vector &t);

Float luminance(const Vector &v);
// bool isPixelValueValid(const Spectrum3f &val);
void progressIndicator(Float progress, int barWidth = 70);

Float fresnelDielectricExt(Float cosThetaI, Float &cosThetaT, Float eta);

inline Float fresnelDielectricExt(Float cosThetaI, Float eta)
{
    Float cosThetaT;
    return fresnelDielectricExt(cosThetaI, cosThetaT, eta);
}

inline Vector reflect(const Vector &wi, const Vector &n) { return 2.0f * wi.dot(n) * n - wi; }

Vector refract(const Vector &wi, const Vector &n, Float eta, Float cosThetaT);
Vector refract(const Vector &wi, const Vector &n, Float eta);
Vector refract(const Vector &wi, const Vector &n, Float eta, Float &cosThetaT, Float &F);

/* spatial form */
Array rayIntersectTriangle(const Vector &v0, const Vector &v1, const Vector &v2, const Ray &ray);

[[nodiscard]] bool rayIntersectTriangle(const Vector &v0, const Vector &v1, const Vector &v2, const Ray &ray, Float &u, Float &v, Float &t);

// /* material form. relative speed. */
Vector rayIntersectTriangleAD(const Vector &v0, const Vector &v1, const Vector &v2, const Ray &ray);

Float computeIntersectionInTri(const Vector3 &a, const Vector3 &b0, const Vector3 &c0, const Vector3 &b1, const Vector3 &c1, Float t0);

// For path-space edge sampling
Vector squareToEdgeRayDirection(const Vector2 &sample, const Vector &n0, const Vector &n1, Float &pdf);

inline int ravel_multi_index(Array2i idx, Array2i dim)
{
    return idx[0] + idx[1] * dim[0];
}

inline Array2i unravel_index(int idx, Array2i dim)
{
    return {idx % dim[0], idx / dim[0]};
}

inline void print(std::string name, const Spectrum &s)
{
    printf("%s: %s\n", name.c_str(), ::toString(s).c_str());
}

template <typename T>
__attribute__((optnone)) T detach(const T &x)
{
    return x;
}

template <typename T>
T misWeight(T pdfA, T pdfB)
{
    return square(pdfA) / (square(pdfA) + square(pdfB));
}

namespace util
{
    inline void print(const Vector &v)
    {
        printf("%f, %f, %f\n", v[0], v[1], v[2]);
    }

    inline void print(const Vector3i &v)
    {
        printf("%d, %d, %d\n", v[0], v[1], v[2]);
    }

    inline void print(const std::vector<Vector> &v)
    {
        for (auto &e : v)
            print(e);
    }

    inline void print(const std::vector<Vector3i> &v)
    {
        for (auto &e : v)
            print(e);
    }
}

// IO Utils
ArrayXd from_spectrum_list_to_tensor(const std::vector<Spectrum> &spec_list, int n_pixels);

// template <class T>
// void from_spectrum_list_to_ptr(const std::vector<T>& spec_list, int n_pixels,
//     ptr<float> p_image);

std::vector<Spectrum> from_tensor_to_spectrum_list(const ArrayXd &arr, int n_pixels);

inline Float rgb2luminance(const Spectrum &rgb) {
        return rgb.x() * .2126f + rgb.y() * .7152f + rgb.z() * .0722f;
}

inline auto rayIntersectSceneAABB(const Ray &ray, const Vector &lower, const Vector &upper) {
    Vector t1 = (lower - ray.org).cwiseQuotient(ray.dir);
    Vector t2 = (upper - ray.org).cwiseQuotient(ray.dir);
    Vector t2p = t1.cwiseMax(t2);
    Vector::Index idx;
    Float t = t2p.minCoeff(&idx);
    Vector n = -ray.dir.cwiseSign();
    Float G = n.dot(-ray.dir) / (t * t);
    return std::make_tuple(t, n, G);
}

inline Vector sphdir(const Float theta, const Float phi) {
    Float sin_theta, cos_theta;
    sincos(theta, &sin_theta, &cos_theta);
    Float sin_phi, cos_phi;
    sincos(phi, &sin_phi, &cos_phi);
    return Vector(cos_phi * sin_theta, sin_phi * sin_theta, cos_theta);
}

#endif // UTILITY_H__
