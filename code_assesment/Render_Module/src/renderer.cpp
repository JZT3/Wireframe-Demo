#include "renderer.h"
#include "wireframe.h"
#include "projection.h"
#include <cmath>
#include <string>
#include <algorithm>

Renderer::Renderer(int width, int height)
	: width(width), height(height), framecount(0) {
	framebuffer = std::make_unique<FrameBuffer>(width, height);
}

void Renderer::drawBresenhamLine(int x0, int y0, int x1, int y1, const Color& color) {
	const bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);

	if (steep) {
		std::swap(x0, y0);
		std::swap(x1, y1);
	}

	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(x1, y1);
	}

	const int dx = x1 - x0;
	const int dy = std::abs(y1 - y0);
	const int y_step = (y0 < y1);

	int error = dx / 2;
	int y = y0;

	for (int x = x0; x <= x1; ++x) {
		if (steep) {
			framebuffer->setPixel(y, x, color);
		}
		else {
			framebuffer->setPixel(x, y, color);
		}

		error -= dy;
		if (error < 0) {
			y += y_step;
			error += dx;
		}
	}
}

// Implement Midpoint circle algorithm
void Renderer::drawCircle(int center_x, int center_y, int radius, const Color& color) {
	int x = radius;
	int y = 0;
	int err = 0;

	while (x >= y) {
		framebuffer->setPixel(center_x + x, center_y + y, color);
		framebuffer->setPixel(center_x + y, center_y + x, color);
		framebuffer->setPixel(center_x - y, center_y + x, color);
		framebuffer->setPixel(center_x - x, center_y + y, color);
		framebuffer->setPixel(center_x - x, center_y - y, color);
		framebuffer->setPixel(center_x - y, center_y - x, color);
		framebuffer->setPixel(center_x + y, center_y - x, color);
		framebuffer->setPixel(center_x + x, center_y - y, color);

		if (err <= 0) {
			y += 1;
			err += 2 * y + 1;
		}

		if (err > 0) {
			x -= 1;
			err -= 2 * x + 1;
		}
	}

	for (int cy = -radius; cy <= radius; cy++) {
		for (int cx = -radius; cx <= radius; cx++) {
			if (cx * cx + cy * cy <= radius * radius) {
				framebuffer->setPixel(center_x + cx, center_y + cy, color);
			}
		}
	}
}


void Renderer::clear(const Color& color) {
	framebuffer->clear(color);
}

void Renderer::drawVertex(const Vector3D& position, int radius, const Color& color) {
	const Vector2D pos2D = orthographicProject(position); // Project 3D position to 2D

	const auto [screen_x, screen_y] = worldToScreen(pos2D); // Convert to screen coordinates

	drawCircle(screen_x, screen_y, radius, color);
}

void Renderer::drawEdge(const Vector3D& start, const Vector3D& end, const Color& color) {
	// Project 3D positions into 2D
	const Vector2D start2D = orthographicProject(start);
	const Vector2D end2D = orthographicProject(end);

	// convert to screen coordinates
	const auto [start_x, start_y] = worldToScreen(start2D);
	const auto [end_x, end_y] = worldToScreen(end2D);

	drawBresenhamLine(start_x, start_y, end_x, end_y, color);
	
}


void Renderer::drawWireFrameObject(const WireFrameObject& object, int vertexRadius) {
	const auto& vertices = object.getVertices();
	const auto& edges    = object.getEdges();

	for (const auto& edge : edges) {
		if (edge.getVertex1Index() < vertices.size() && edge.getVertex2Index() < vertices.size()) {
			const auto& v1 = vertices[edge.getVertex1Index()].getPosition();
			const auto& v2 = vertices[edge.getVertex2Index()].getPosition();
			drawEdge(v1, v2, Color::Blue());
		}
	}

	for (const auto& vertex : vertices) {
		drawVertex(vertex.getPosition(), vertexRadius, Color::Blue());
	}
}


bool Renderer::saveFrame(const std::string& filenamePrefix) {
	const std::string filename = filenamePrefix + "_" + std::to_string(frameCount++) + ".ppm";
	return framebuffer->saveToPPM(filename);
}