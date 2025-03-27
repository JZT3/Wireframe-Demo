#pragma once
#include <string>
#include <memory>
#include "vector2D.h"
#include "vector3D.h"
#include "framebuffer.h"

class WireFrameObject;

class Renderer {
private:
	int width, height;
	std::unique_ptr<FrameBuffer> framebuffer;
	int frameCount;

	// Drawing primatives
	void drawBresenhamLine(int x0, int x1, int y0, int y1, const Color& color);
	void drawCircle(int center_x, int center_y, int radius, const Color& color);

	[[nodiscard]] inline std::pair<int, int> worldToScreen(const Vector2D& point) const noexcept {
		return {
			static_cast<int>(point.x + width / 2.0f),
			static_cast<int>(height / 2.0f - point.y)
		};
	}


public:
	explicit Renderer(int width, int height);
	~Renderer() noexcept = default;

	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer(Renderer&&) noexcept = delete;
	Renderer& operator=(Renderer&&) noexcept = delete;

	void clear(const Color& color = Color::Black());
	void drawVertex(const Vector3D& position, int radius, const Color& color);
	void drawEdge(const Vector3D& start, const Vector3D& end, const Color& color);
	void drawWireFrameObject(const WireFrameObject& object, int vertexRadius);

	bool saveFrame(const std::string& filenamePrefix);
};

