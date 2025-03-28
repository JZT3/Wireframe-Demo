#pragma once
#include <cmath>

namespace Math {
    class Vector2D {
    public:
        float x, y;

        constexpr Vector2D() noexcept : x(0.0f), y(0.0f) {}
        constexpr explicit Vector2D(float x, float y) noexcept : x(x), y(y) {}

        [[nodiscard]] constexpr inline Vector2D operator+(const Vector2D& v) const noexcept {
            return Vector2D(x + v.x, y + v.y);
        }

        [[nodiscard]] constexpr inline Vector2D operator-(const Vector2D& v) const noexcept {
            return Vector2D(x - v.x, y - v.y);
        }

        [[nodiscard]] constexpr inline Vector2D operator*(float scalar) const noexcept {
            return Vector2D(x * scalar, y * scalar);
        }

        [[nodiscard]] inline float length() const noexcept {
            return std::sqrt(x * x + y * y);
        }

        [[nodiscard]] Vector2D normalize() const noexcept {
            const float len = length();
            return (len > 0.0f) ? Vector2D(x / len, y / len) : *this;
        }
    };
}