#pragma once
#include <cmath>

class Vector3D {
public:
    float x, y, z;

    constexpr Vector3D() noexcept : x(0.0f), y(0.0f), z(0.0f) {}
    explicit constexpr Vector3D(float x, float y, float z) noexcept : x(x), y(y), z(z) {}

    [[nodiscard]] inline Vector3D operator+(const Vector3D& v) const noexcept {
        return Vector3D(x + v.x, y + v.y, z + v.z);
    }

    [[nodiscard]] inline Vector3D operator-(const Vector3D& v) const noexcept {
        return Vector3D(x - v.x, y - v.y, z - v.z);
    }

    [[nodiscard]] inline Vector3D operator*(float scalar) const noexcept {
        return Vector3D(x * scalar, y * scalar, z * scalar);
    }

    [[nodiscard]] inline float dot(const Vector3D& v) const noexcept {
        return x * v.x + y * v.y + z * v.z;
    }

    [[nodiscard]] inline Vector3D cross(const Vector3D& v) const noexcept {
        return Vector3D(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        );
    }

    [[nodiscard]] inline float length() const noexcept {
        return std::sqrt(x * x + y * y + z * z);
    }

    [[nodiscard]] inline Vector3D normalize() const noexcept {
        const float len = length();
        if (len > 0) {
            return Vector3D(x / len, y / len, z / len);
        }
        return *this;
    }
};