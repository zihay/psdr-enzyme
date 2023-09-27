#include <core/utils.h>
#include <core/math_func.h>
#include <Eigen/Geometry>
#include <iostream>
#include <core/logger.h>

Vector3 xfm_point(const Matrix4x4 &T, const Vector3 &pos)
{
    Vector4 pos_homo(pos.x(), pos.y(), pos.z(), 1.0f);
    pos_homo = T * pos_homo;
    return pos_homo.head(3) / pos_homo.w();
}

Vector3 xfm_vector(const Matrix4x4 &T, const Vector3 &vec)
{
    Matrix3x3 TT = T.block(0, 0, 3, 3);
    return TT * vec;
}

Vector2 squareToUniformDiskConcentric(const Vector2 &sample)
{
    Float r1 = 2.0f * sample.x() - 1.0f;
    Float r2 = 2.0f * sample.y() - 1.0f;
    Float phi, r;
    if (r1 == 0 && r2 == 0)
    {
        r = phi = 0;
    }
    else if (r1 * r1 > r2 * r2)
    {
        r = r1;
        phi = (M_PI / 4.0f) * (r2 / r1);
    }
    else
    {
        r = r2;
        phi = (M_PI / 2.0f) - (r1 / r2) * (M_PI / 4.0f);
    }
    Float sinPhi, cosPhi;
    math::sincos(phi, sinPhi, cosPhi);
    return Vector2(r * cosPhi, r * sinPhi);
}

Vector squareToCosineHemisphere(const Vector2 &sample)
{
    Vector2 p = squareToUniformDiskConcentric(sample);
    Float z = std::sqrt(
        std::max(static_cast<Float>(0.0f), 1.0f - p.x() * p.x() - p.y() * p.y()));
    return Vector(p.x(), p.y(), z);
}

Vector squareToCosineSphere(const Vector2 &_sample)
{
    Vector2 sample(_sample);
    bool flip;
    if (sample[0] < 0.5f)
    {
        flip = false;
        sample[0] *= 2.0f;
    }
    else
    {
        flip = true;
        sample[0] = 2.0f * (sample[0] - 0.5f);
    }
    Vector ret = squareToCosineHemisphere(sample);
    if (flip)
        ret[2] = -ret[2];
    return ret;
}

Vector squareToUniformSphere(const Vector2 &sample)
{
    Float z = 1.0f - 2.0f * sample.y();
    Float r = std::sqrt(std::max(static_cast<Float>(0.0f), 1.0f - z * z));
    Float sinPhi, cosPhi;
    math::sincos(2.0f * M_PI * sample.x(), sinPhi, cosPhi);
    return Vector(r * cosPhi, r * sinPhi, z);
}

Float squareToCosineHemispherePdf(const Vector &d) { return INV_PI * d.z(); }

Float squareToCosineSpherePdf(const Vector &d)
{
    return 0.5f * INV_PI * std::abs(d.z());
}

void coordinateSystem(const Vector &_n, Vector &s, Vector &t)
{
    Matrix3x3 randRot(
        Eigen::AngleAxis<Float>(0.1f, Vector(0.1f, 0.2f, 0.3f).normalized())
            .toRotationMatrix());
    Matrix3x3 randRotInv = randRot.transpose();
    Vector n = randRot * _n;
    if (std::abs(n.x()) > std::abs(n.y()))
    {
        Float invLen = 1.0f / std::sqrt(n.x() * n.x() + n.z() * n.z());
        t = Vector(n.z() * invLen, 0.0f, -n.x() * invLen);
    }
    else
    {
        Float invLen = 1.0f / std::sqrt(n.y() * n.y() + n.z() * n.z());
        t = Vector(0.0, n.z() * invLen, -n.y() * invLen);
    }
    s = t.cross(n);
    s = randRotInv * s;
    t = randRotInv * t;
}

// void coordinateSystem(const Vector &a, Vector &b, Vector &c) {
//     if (std::abs(a.x()) > std::abs(a.y())) {
//         Float invLen = 1.0f / std::sqrt(a.x() * a.x() + a.z() * a.z());
//         c = Vector(a.z() * invLen, 0.0f, -a.x() * invLen);
//     } else {
//         Float invLen = 1.0f / std::sqrt(a.y() * a.y() + a.z() * a.z());
//         c = Vector(0.0f, a.z() * invLen, -a.y() * invLen);
//     }
//     b = c.cross(a);
// }

Float luminance(const Vector &v)
{
    return 0.212671f * v(0) + 0.715160f * v(1) + 0.072169f * v(2);
}

// bool isPixelValueValid(const Spectrum3f &val)
// {
//     for (int i = 0; i < 3; i++)
//     {
//         if (val(i) < -1e-6 || std::isinf(val(i)) || std::isnan(val(i)))
//         {
//             std::cout << "[invalid pixel value] " << val << std::endl;
//             return false;
//         }
//     }
//     return true;
// }

void progressIndicator(Float progress, int barWidth)
{
    printf("[");
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i)
    {
        if (i < pos)
            printf("=");
        else if (i == pos)
            printf(">");
        else
            printf(" ");
    }
    printf("] %.0lf %%\r", progress * 100.);
    fflush(stdout);
}

Float fresnelDielectricExt(Float cosThetaI_, Float &cosThetaT_, Float eta)
{
    if (std::abs(eta - 1.0f) < Epsilon)
    {
        cosThetaT_ = -cosThetaI_;
        return 0.0f;
    }
    /* Using Snell's law, calculate the squared sine of the
     angle between the normal and the transmitted ray */
    Float scale = (cosThetaI_ > 0.0f) ? 1.0f / eta : eta,
          cosThetaTSqr =
              1.0f - (1.0f - cosThetaI_ * cosThetaI_) * (scale * scale);

    /* Check for total internal reflection */
    if (cosThetaTSqr < Epsilon)
    {
        cosThetaT_ = 0.0f;
        return 1.0f;
    }

    /* Find the absolute cosines of the incident/transmitted rays */
    Float cosThetaI = std::abs(cosThetaI_);
    Float cosThetaT = std::sqrt(cosThetaTSqr);

    Float Rs = (cosThetaI - eta * cosThetaT) / (cosThetaI + eta * cosThetaT);
    Float Rp = (eta * cosThetaI - cosThetaT) / (eta * cosThetaI + cosThetaT);

    cosThetaT_ = (cosThetaI_ > 0.0f) ? -cosThetaT : cosThetaT;

    /* No polarization -- return the unpolarized reflectance */
    return 0.5f * (Rs * Rs + Rp * Rp);
}

Vector refract(const Vector &wi, const Vector &n, Float eta, Float cosThetaT)
{
    if (cosThetaT < 0)
        eta = 1.0f / eta;
    return n * (wi.dot(n) * eta + cosThetaT) - wi * eta;
}

Vector refract(const Vector &wi, const Vector &n, Float eta)
{
    assert(std::abs(eta - 1.0) > Epsilon);

    Float cosThetaI = wi.dot(n);
    if (cosThetaI > 0)
        eta = 1.0f / eta;

    /* Using Snell's law, calculate the squared sine of the
     angle between the normal and the transmitted ray */
    Float cosThetaTSqr = 1.0f - (1.0f - cosThetaI * cosThetaI) * (eta * eta);

    /* Check for total internal reflection */
    if (cosThetaTSqr < Epsilon)
        return Vector::Zero();

    return n * (cosThetaI * eta -
                math::signum(cosThetaI) * std::sqrt(cosThetaTSqr)) -
           wi * eta;
}

Vector refract(const Vector &wi, const Vector &n, Float eta, Float &cosThetaT,
               Float &F)
{
    Float cosThetaI = wi.dot(n);
    F = fresnelDielectricExt(cosThetaI, cosThetaT, eta);

    if (std::abs(F - 1.0) < Epsilon) /* Total internal reflection */
        return Vector::Zero();

    if (cosThetaT < 0)
        eta = 1.0f / eta;
    return n * (eta * cosThetaI + cosThetaT) - wi * eta;
}

Array rayIntersectTriangle(const Vector &v0, const Vector &v1, const Vector &v2,
                           const Ray &ray)
{
    const Vector e1 = v1 - v0, e2 = v2 - v0;
    const Vector pvec = ray.dir.cross(e2);
    Float divisor = pvec.dot(e1);
    // Hack
    if (std::abs(divisor) < Epsilon)
        divisor = (divisor > 0) ? Epsilon : -Epsilon;
    const Vector s = ray.org - v0;
    const Float dot_s_pvec = s.dot(pvec);
    const Float u = dot_s_pvec / divisor;
    const Vector qvec = s.cross(e1);
    const Float dot_dir_qvec = ray.dir.dot(qvec);
    const Float v = dot_dir_qvec / divisor;
    const Float dot_e2_qvec = e2.dot(qvec);
    const Float t = dot_e2_qvec / divisor;
    return Vector(u, v, t);
}

bool rayIntersectTriangle(const Vector &v0, const Vector &v1, const Vector &v2,
                          const Ray &ray,
                          Float &u, Float &v, Float &t) {
    const Vector e1 = v1 - v0, e2 = v2 - v0;
    const Vector pvec = ray.dir.cross(e2);

    // lie in plane of triangle
    Float det = e1.dot(pvec);
    if (det > -1e-8f && det < 1e-8f)
        return false;

    Float divisor = pvec.dot(e1);
    // Hack
    if (std::abs(divisor) < Epsilon)
        divisor = (divisor > 0) ? Epsilon : -Epsilon;
    const Vector s          = ray.org - v0;
    const Float  dot_s_pvec = s.dot(pvec);
    u                       = dot_s_pvec / divisor;

    if (u < 0.0 || u > 1.0)
        return false;

    const Vector qvec         = s.cross(e1);
    const Float  dot_dir_qvec = ray.dir.dot(qvec);
    v                         = dot_dir_qvec / divisor;

    if (v < 0.0 || u + v > 1.0)
        return false;

    const Float dot_e2_qvec = e2.dot(qvec);
    t                       = dot_e2_qvec / divisor;

    return t >= ray.tmin && t <= ray.tmax;
}

Vector rayIntersectTriangleAD(const Vector &v0, const Vector &v1, const Vector &v2, const Ray &ray)
{
    const Vector e1 = v1 - v0, e2 = v2 - v0;
    const Vector pvec = ray.dir.cross(e2);
    Float divisor = pvec.dot(e1);
    // Hack
    if (std::abs(divisor) < Epsilon)
        divisor = (divisor > 0) ? Epsilon : -Epsilon;
    const Vector s = ray.org - v0;
    const Float dot_s_pvec = s.dot(pvec);
    const Float u = dot_s_pvec / divisor;
    const Vector qvec = s.cross(e1);
    const Float dot_dir_qvec = ray.dir.dot(qvec);
    const Float v = dot_dir_qvec / divisor;
    return (1.0f - u - v) * v0 + u * v1 + v * v2;
}
// INACTIVE_FN(rayIntersectTriangle, rayIntersectTriangle);

Float computeIntersectionInTri(const Vector &a, const Vector &b0,
                               const Vector &c0, const Vector &b1,
                               const Vector &c1, Float t0)
{
    // b0 = a + coeff_b * (b1-a) + 0 * (c1-a)
    Float coeff_b = (b0 - a).norm() / (b1 - a).norm();
    // c0 = a +    0 * (b1-a)    + coeff_c * (c1-a)
    Float coeff_c = (c0 - a).norm() / (c1 - a).norm();
    // p0 = b0 + t0 * (c0 - b0)
    Vector2 barycentric((1 - t0) * coeff_b, t0 * coeff_c);
    barycentric /= (barycentric.x() + barycentric.y());
    // p = a + bary.x * (b1-a) + bary.y * (c1-a)
    return barycentric.y();
}

Vector squareToEdgeRayDirection(const Vector2 &sample, const Vector &n0,
                                const Vector &n1, Float &pdf)
{
    Float tmp = n0.dot(n1);
    PSDR_WARN(tmp > -1.0f + EdgeEpsilon && tmp < 1.0f - EdgeEpsilon);
    Float phi0 = std::acos(tmp);
    pdf = 1.0f / (4.0f * phi0);

    Vector Z = (n0 + n1).normalized();
    Vector Y = n0.cross(Z).normalized();
    Vector X = Y.cross(Z);

    Float phi = (sample[0] - 0.5f) * phi0;

    // Avoid sampling directions that are almost within either plane
    phi = clamp(phi, -0.5f * phi0 + EdgeEpsilon, 0.5f * phi0 - EdgeEpsilon);

    Vector X1 = X * std::cos(phi) + Z * std::sin(phi);

    Float a, b;
    if (sample[1] > 0.5f)
    {
        b = 4.0f * sample[1] - 3.0f;
        a = -math::safeSqrt(1.0f - b * b);
    }
    else
    {
        b = 4.0f * sample[1] - 1.0f;
        a = math::safeSqrt(1.0f - b * b);
    }

    Vector ret = X1 * a + Y * b;
    // assert(math::signum(ret.dot(n0))*math::signum(ret.dot(n1)) < -0.5f);
    return ret;
}

// Convert std::vector<Spectrum> to ArrayXd
ArrayXd from_spectrum_list_to_tensor(const std::vector<Spectrum> &spec_list, int n_pixels)
{
    ArrayXd arr = ArrayXd::Zero(n_pixels * 3);

    for (int i = 0; i < n_pixels; ++i)
    {
        Eigen::Array3d s = spec_list[i].array();
        arr[3 * i + 0] = s[0];
        arr[3 * i + 1] = s[1];
        arr[3 * i + 2] = s[2];
    }

    return arr;
}

// template <class T>
// void from_spectrum_list_to_ptr(const std::vector<T>& spec_list, int n_pixels,
//     ptr<float> p_image) {
//   for (int i = 0; i < n_pixels; ++i) {
//     p_image[3*i] = spec_list[i][0](0);
//     p_image[3*i+1] = spec_list[i][1](0);
//     p_image[3*i+2] = spec_list[i][2](0);
//   }
// }

// Convert ArrayXD to std::vector<Spectrum>
std::vector<Spectrum> from_tensor_to_spectrum_list(const ArrayXd &arr, int n_pixels)
{
    std::vector<Spectrum> spec_list;
    spec_list.reserve(n_pixels);

    for (int i = 0; i < n_pixels; ++i)
    {
        spec_list.push_back(Spectrum(arr(3 * i), arr(3 * i + 1), arr(3 * i + 2)));
    }

    return spec_list;
}

INACTIVE_FN(squareToCosineHemisphere, squareToCosineHemisphere);
INACTIVE_FN(squareToCosineHemispherePdf, squareToCosineHemispherePdf);
INACTIVE_FN(detach_Float, detach<Float>);
INACTIVE_FN(detach_Vector2, detach<Vector2>);
INACTIVE_FN(detach_Vector, detach<Vector>);
INACTIVE_FN(detach_Matrix, detach<Matrix3x3>);
