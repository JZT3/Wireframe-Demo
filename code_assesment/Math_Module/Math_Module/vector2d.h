#pragma once

class Vector2D {
public:
    float x, y;

    constexpr Vector2D() noexcept : x(0.0f), y(0.0f) {}
    explicit constexpr Vector2D(float x, float y) noexcept : x(x), y(y) {}

    [[nodiscard]] inline Vector2D operator+(const Vector2D& v) const noexcept {
        return Vector2D(x + v.x, y + v.y);
    }

    [[nodiscard]] inline Vector2D operator-(const Vector2D& v) const noexcept {
        return Vector2D(x - v.x, y - v.y);
    }

    [[nodiscard]] inline Vector2D operator*(float scalar) const noexcept {
        return Vector2D(x * scalar, y * scalar);
    }
};