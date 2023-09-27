#pragma once
#ifndef RAY_H__
#define RAY_H__

#include <limits>
#include <core/fwd.h>

struct Ray
{
    inline Ray() {}
    inline Ray(const Vector &org, const Vector &dir) : org(org), dir(dir) {}
    inline Ray(const Vector &org, const Vector &dir, Float tmin, Float tmax)
        : org(org), dir(dir), tmin(tmin), tmax(tmax) {}
    inline Ray(const Ray &ray) : org(ray.org), dir(ray.dir), tmin(ray.tmin), tmax(ray.tmax) {}

    inline Vector operator()(Float t) const { return org + t * dir; }
    inline Ray flipped() const { return Ray(org, -dir); }
    inline Ray shifted(Float t = ShadowEpsilon) const
    {
        Ray r = *this;
        r.tmin = t;
        return r;
    }
    inline Vector at(Float t) const { return org + t * dir; }

    Vector org, dir;

    Float tmin = 0.,
          tmax = std::numeric_limits<Float>::infinity();
};

#endif
