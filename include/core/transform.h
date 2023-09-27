#pragma once
#include <core/fwd.h>

NAMESPACE_BEGIN(psdr)

namespace transform {
/// Create a translation transformation
static inline Matrix4x4 translate(const Vector &v) {
    return Eigen::Affine3d(
               Eigen::Translation3d(v.x(), v.y(), v.z()))
        .matrix();
}

/// Create a scale transformation
static inline Matrix4x4 scale(const Vector &v) {
    return Eigen::Affine3d(
               Eigen::Scaling(v.x(), v.y(), v.z()))
        .matrix();
}

/// Create a rotation transformation around an arbitrary axis in 3D. The angle is specified in degrees
static inline Matrix4x4 rotate(const Vector &axis, float angle) {
    return Eigen::Affine3d(
               Eigen::AngleAxisd(
                   angle * M_PI / 180.0, axis.normalized()))
        .matrix();
}

/** \brief Create a perspective transformation.
 *   (Maps [near, far] to [0, 1])
 *
 *  Projects vectors in camera space onto a plane at z=1:
 *
 *  x_proj = x / z
 *  y_proj = y / z
 *  z_proj = (far * (z - near)) / (z * (far-near))
 *
 *  Camera-space depths are not mapped linearly!
 *
 * \param fov Field of view in degrees
 * \param near Near clipping plane
 * \param far  Far clipping plane
 */
static inline Matrix4x4 perspective(Float fov, Float near_, Float far_) {
    Float     cot = 1.0 / std::tan(fov * M_PI / 360.0);
    Float     nf  = far_ / (far_ - near_);
    Matrix4x4 m;
    m << cot, 0, 0, 0,
        0, cot, 0, 0,
        0, 0, nf, -near_ * nf,
        0, 0, 1., 0;
    return m;
}

/** \brief Create a look-at camera transformation
 *
 * \param origin Camera position
 * \param target Target vector
 * \param up     Up vector
 */
static inline Matrix4x4 look_at(const Vector &origin, const Vector &target, const Vector &up) {
    Vector  dir    = (target - origin).normalized();
    Vector  left   = up.cross(dir).normalized();
    Vector  new_up = dir.cross(left);
    Vector4 dir_, left_, new_up_, origin_;
    dir_ << dir, 0.f;
    left_ << left, 0.f;
    new_up_ << new_up, 0.f;
    origin_ << origin, 1.f;
    Matrix4x4 m;
    // left handed system
    m << -left_, new_up_, dir_, origin_;
    return m;
}

} // namespace transform

static inline Vector transform_pos(const Matrix4x4 &mat, const Vector &vec) {
    Vector4 tmp = mat * vec.homogeneous();
    return tmp.head<3>() / tmp.w();
}

static inline Vector transform_dir(const Matrix4x4 &mat, const Vector &vec) {
    return (mat * vec.homogeneous()).head<3>();
}

NAMESPACE_END(psdr)