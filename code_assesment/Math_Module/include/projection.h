#pragma once
#include "Vector3D.h"
#include "Vector2D.h"

namespace Math {
    // Orthographic projection
    [[nodiscard]] inline Vector2D orthographicProject(const Vector3D& v) noexcept {
        return Vector2D(v.x, v.y);
    }

    // Perspective projection
    [[nodiscard]] inline Vector2D perspectiveProject(const Vector3D& v, float focalLength = 1.0f) noexcept {
        // Avoid division by zero
        if (std::abs(v.z) < 1e-6f) {
            return Vector2D(v.x, v.y);
        }

        // Scale X and Y based on Z distance
        float scale = focalLength / (focalLength + v.z);
        return Vector2D(v.x * scale, v.y * scale);
    }
}