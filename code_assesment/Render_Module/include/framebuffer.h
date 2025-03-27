#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include "color.h"

class FrameBuffer {
private:
	int width, height;
	std::vector<Color> pixels;

public:
	FrameBuffer(int width, int height) : width(width), height(height) {
		std::size_t bufferSize = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
		pixels.resize(bufferSize, Color::Black());
	}

	void clear(const Color& color = Color::Black()) {
		std::fill(pixels.begin(), pixels.end(), color);
	}

	void setPixel(int x, int y, const Color& color) {
		if (x >= 0 && x < width && y >= 0 && y < height) {
			std::size_t index = static_cast<std::size_t>(y) *
				static_cast<std::size_t>(width) +
				static_cast<std::size_t>(x);
			
			pixels[index] = color;
		}
	}


	[[nodiscard]] Color getPixel(int x, int y) const {
		if (x >= 0 && x < width && y >= 0 && y < height) {
			std::size_t index = static_cast<std::size_t>(y) *
				static_cast<std::size_t>(width) +
				static_cast<std::size_t>(x);

			return pixels[index];
		}
		return Color::Black();
	}

	[[nodiscard]] int getWidth() const noexcept { return width; }
	[[nodiscard]] int getHeight() const noexcept { return height; }


	bool saveToPPM(const std::string& filename) const {
		std::ofstream file(filename, std::ios::binary);
		if (!file) {
			return false;
		}

		file << "P6\n" << width << " " << height << "\n255\n";

		// Write pixel data
		for (const auto& pixel : pixels) {
			file.write(reinterpret_cast<const char*>(&pixel.r), 1);
			file.write(reinterpret_cast<const char*>(&pixel.g), 1);
			file.write(reinterpret_cast<const char*>(&pixel.b), 1);
		}

		return file.good();
	}
};
