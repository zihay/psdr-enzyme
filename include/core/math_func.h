#pragma once
#ifndef MATH_H__
#define MATH_H__

#include <core/fwd.h>
#include <cmath>

namespace math
{

    inline void sincos(float theta, float &sinTheta, float &cosTheta) {
#if defined(_MSC_VER)
        sinTheta = std::sin(theta);
        cosTheta = std::cos(theta);
#elif defined(__APPLE__)
        __sincosf(theta, &sinTheta, &cosTheta);
#else
        ::sincosf(theta, &sinTheta, &cosTheta);
#endif
    }

    inline void sincos(double theta, double &sinTheta, double &cosTheta) {
#if defined(_MSC_VER)
        sinTheta = std::sin(theta);
        cosTheta = std::cos(theta);
#elif defined(__APPLE__)
        __sincos(theta, &sinTheta, &cosTheta);
#else
        ::sincos(theta, &sinTheta, &cosTheta);
#endif
    }

    inline Float signum(Float value) {
#if defined(SINGLE_PRECISION)
        return copysignf(1.0f, value);
#elif defined(DOUBLE_PRECISION)
        return copysign(1.0, value);
#endif
    }

    inline Float safeSqrt(Float value) {
        return std::sqrt(std::max(static_cast<Float>(0.0f), value));
    }

    Float erf(Float x);
    Float erfinv(Float x);

} //namespace math

#endif //MATH_H__
