#pragma once
#include <cmath>
#include <algorithm>
#include <utility>
#include "Vector2D.h"
#include "render_target_interface.h"

namespace Render {
    namespace GraphicsPrimitives {
        // Bresenham's line algorithm
        inline void drawLine(IRenderTarget& target, int x0, int y0, int x1, int y1, const Color& color) noexcept {
            bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);

            if (steep) {
                std::swap(x0, y0);
                std::swap(x1, y1);
            }

            if (x0 > x1) {
                std::swap(x0, x1);
                std::swap(y0, y1);
            }

            int dx = x1 - x0;
            int dy = std::abs(y1 - y0);
            int error = dx / 2;
            int yStep = (y0 < y1) ? 1 : -1;
            int y = y0;

            for (int x = x0; x <= x1; ++x) {
                steep ? target.setPixel(y, x, color) : target.setPixel(x, y, color);

                error -= dy;
                if (error < 0) {
                    y += yStep;
                    error += dx;
                }
            }
        }

        // Midpoint circle algorithm with filling
        inline void drawCircle(IRenderTarget& target, int centerX, int centerY, int radius, const Color& color) noexcept {
            // Fill circle
            for (int y = -radius; y <= radius; y++) {
                for (int x = -radius; x <= radius; x++) {
                    if (x * x + y * y <= radius * radius) {
                        target.setPixel(centerX + x, centerY + y, color);
                    }
                }
            }
        }

        // Convert from world to screen coordinates
        [[nodiscard]] inline std::pair<int, int> worldToScreen(const Math::Vector2D& point, int width, int height) noexcept {
            return {
                static_cast<int>((point.x + 1.0f) * width / 2.0f),
                static_cast<int>((1.0f - point.y) * height / 2.0f)
            };
        }
    }
}