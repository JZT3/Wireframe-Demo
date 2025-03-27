#include "matrix4x4.h"
#include "vector3d.h"

// Implementation of the Matrix4x4::transform function
inline Vector3D Matrix4x4::transform(const Vector3D& v) const noexcept {
    const float x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3];
    const float y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3];
    const float z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3];
    const float w = m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3];

    if (std::abs(w) > 1e-6f) {
        return Vector3D(x / w, y / w, z / w);
    }
    return Vector3D(x, y, z);
}