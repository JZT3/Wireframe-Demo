#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include "render_target_interface.h"

namespace Render {
    class FrameBuffer final : public IRenderTarget {
    private:
        int width, height;
        std::vector<Color> pixels;

    public:
        explicit FrameBuffer(int width, int height) : width(width), height(height) {
            pixels.resize(static_cast<size_t>(width) * static_cast<size_t>(height), Color::Black());
        }

        void setPixel(int x, int y, const Color& color) noexcept override {
            if (x >= 0 && x < width && y >= 0 && y < height) {
                size_t index = static_cast<size_t>(y) * width + static_cast<size_t>(x);
                pixels[index] = color;
            }
        }

        [[nodiscard]] Color getPixel(int x, int y) const noexcept override {
            if (x >= 0 && x < width && y >= 0 && y < height) {
                size_t index = static_cast<size_t>(y) * width + static_cast<size_t>(x);
                return pixels[index];
            }
            return Color::Black();
        }

        [[nodiscard]] int getWidth() const noexcept override { return width; }
        [[nodiscard]] int getHeight() const noexcept override { return height; }

        void clear(const Color& color = Color::Black()) noexcept override {
            std::fill(pixels.begin(), pixels.end(), color);
        }

        bool saveToPPM(const std::string& filename) const noexcept {
            std::ofstream file(filename, std::ios::binary);
            if (!file) {
                return false;
            }

            file << "P6\n" << width << " " << height << "\n255\n";

            for (const auto& pixel : pixels) {
                file.write(reinterpret_cast<const char*>(&pixel.r), 1);
                file.write(reinterpret_cast<const char*>(&pixel.g), 1);
                file.write(reinterpret_cast<const char*>(&pixel.b), 1);
            }

            return file.good();
        }
    };
}