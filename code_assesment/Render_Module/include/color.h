#pragma once
#include <cstdint>

namespace Render {
    struct Color {
        uint8_t r, g, b;

        constexpr Color() noexcept : r(0), g(0), b(0) {}
        constexpr Color(uint8_t r, uint8_t g, uint8_t b) noexcept : r(r), g(g), b(b) {}

        // Predefined colors
        static constexpr Color Black() noexcept { return Color(0, 0, 0); }
        static constexpr Color White() noexcept { return Color(255, 255, 255); }
        static constexpr Color Red() noexcept { return Color(255, 0, 0); }
        static constexpr Color Green() noexcept { return Color(0, 255, 0); }
        static constexpr Color Blue() noexcept { return Color(0, 0, 255); }
    };
}