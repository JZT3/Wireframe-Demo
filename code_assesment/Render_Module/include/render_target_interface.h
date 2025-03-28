#pragma once
#include "Color.h"

namespace Render {
    // Interface for render targets (Abstract Factory pattern)
    class IRenderTarget {
    public:
        virtual void setPixel(int x, int y, const Color& color) noexcept = 0;
        [[nodiscard]] virtual Color getPixel(int x, int y) const noexcept = 0;
        [[nodiscard]] virtual int getWidth() const noexcept = 0;
        [[nodiscard]] virtual int getHeight() const noexcept = 0;
        virtual void clear(const Color& color = Color::Black()) noexcept = 0;
        virtual ~IRenderTarget() = default;
    };
}