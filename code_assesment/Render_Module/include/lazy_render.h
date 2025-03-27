#pragma once
#include <functional>
#include <variant>
#include <vector>
#include <memory>
#include <optional>
#include "vector3D.h"
#include "vector2D.h"
#include "projection.h"
#include "wireframe.h"
#include "color.h"

// Algebraic data type for representing different kinds of primitives
struct Point {
    Vector3D position;
    int size;
    Color color;
};

struct Line {
    Vector3D start;
    Vector3D end;
    Color color;
};

struct Circle {
    Vector3D center;
    float radius;
    Color color;
};

// Define the RenderPrimitive variant
using RenderPrimitive = std::variant<Point, Line, Circle>;

// A lazily evaluated rendering pipeline
class LazyRenderer {
private:
    // Viewport dimensions
    int width, height;

    // Collection of deferred rendering operations
    std::vector<std::function<void()>> renderOps;

    // Function to convert from world to screen coordinates
    std::function<std::pair<int, int>(const Vector2D&)> worldToScreenFn;

    // Framebuffer accessor functions
    std::function<void(int, int, const Color&)> setPixelFn;
    std::function<Color(int, int)> getPixelFn;

    // Rendering primitive implementations
    void renderPoint(const Point& point) {
        // Project 3D position to 2D
        Vector2D pos2D = orthographicProject(point.position);

        // Convert to screen coordinates
        auto [x, y] = worldToScreenFn(pos2D);

        // Draw circle at point
        for (int cy = -point.size; cy <= point.size; cy++) {
            for (int cx = -point.size; cx <= point.size; cx++) {
                if (cx * cx + cy * cy <= point.size * point.size) {
                    setPixelFn(x + cx, y + cy, point.color);
                }
            }
        }
    }

    void renderLine(const Line& line) {
        // Project 3D positions to 2D
        Vector2D start2D = orthographicProject(line.start);
        Vector2D end2D = orthographicProject(line.end);

        // Convert to screen coordinates
        auto [x0, y0] = worldToScreenFn(start2D);
        auto [x1, y1] = worldToScreenFn(end2D);

        // Bresenham's line algorithm
        const bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
        if (steep) {
            std::swap(x0, y0);
            std::swap(x1, y1);
        }

        if (x0 > x1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
        }

        const int dx = x1 - x0;
        const int dy = std::abs(y1 - y0);
        const int yStep = (y0 < y1) ? 1 : -1;

        int error = dx / 2;
        int y = y0;

        for (int x = x0; x <= x1; ++x) {
            if (steep) {
                setPixelFn(y, x, line.color);
            }
            else {
                setPixelFn(x, y, line.color);
            }

            error -= dy;
            if (error < 0) {
                y += yStep;
                error += dx;
            }
        }
    }

    void renderCircle(const Circle& circle) {
        // Project 3D position to 2D
        Vector2D center2D = orthographicProject(circle.center);

        // Convert to screen coordinates
        auto [centerX, centerY] = worldToScreenFn(center2D);

        int radius = static_cast<int>(circle.radius);

        // Midpoint circle algorithm
        int x = radius;
        int y = 0;
        int err = 0;

        while (x >= y) {
            setPixelFn(centerX + x, centerY + y, circle.color);
            setPixelFn(centerX + y, centerY + x, circle.color);
            setPixelFn(centerX - y, centerY + x, circle.color);
            setPixelFn(centerX - x, centerY + y, circle.color);
            setPixelFn(centerX - x, centerY - y, circle.color);
            setPixelFn(centerX - y, centerY - x, circle.color);
            setPixelFn(centerX + y, centerY - x, circle.color);
            setPixelFn(centerX + x, centerY - y, circle.color);

            if (err <= 0) {
                y += 1;
                err += 2 * y + 1;
            }

            if (err > 0) {
                x -= 1;
                err -= 2 * x + 1;
            }
        }
    }

    // Visitor for rendering primitives
    struct RenderVisitor {
        LazyRenderer& renderer;

        void operator()(const Point& point) const {
            renderer.renderPoint(point);
        }

        void operator()(const Line& line) const {
            renderer.renderLine(line);
        }

        void operator()(const Circle& circle) const {
            renderer.renderCircle(circle);
        }
    };

public:
    LazyRenderer(
        int width,
        int height,
        std::function<std::pair<int, int>(const Vector2D&)> worldToScreenFn,
        std::function<void(int, int, const Color&)> setPixelFn,
        std::function<Color(int, int)> getPixelFn
    ) : width(width), height(height),
        worldToScreenFn(std::move(worldToScreenFn)),
        setPixelFn(std::move(setPixelFn)),
        getPixelFn(std::move(getPixelFn)) {
    }

    // Queue a rendering primitive for later evaluation
    void queuePrimitive(const RenderPrimitive& primitive) {
        renderOps.emplace_back([this, primitive]() {
            std::visit(RenderVisitor{ *this }, primitive);
            });
    }

    // Queue a wireframe object for rendering
    void queueWireframeObject(const WireFrameObject& object, int vertexRadius, const Color& color) {
        const auto& vertices = object.getVertices();
        const auto& edges = object.getEdges();

        // Queue edges
        for (const auto& edge : edges) {
            if (edge.getVertex1Index() < vertices.size() && edge.getVertex2Index() < vertices.size()) {
                const auto& v1 = vertices[edge.getVertex1Index()].getPosition();
                const auto& v2 = vertices[edge.getVertex2Index()].getPosition();
                queuePrimitive(Line{ v1, v2, color });
            }
        }

        // Queue vertices
        for (const auto& vertex : vertices) {
            queuePrimitive(Point{ vertex.getPosition(), vertexRadius, color });
        }
    }

    // Execute all queued rendering operations
    void render() {
        for (auto& op : renderOps) {
            op();
        }
        renderOps.clear();
    }

    // Clear the rendering queue
    void clear() {
        renderOps.clear();
    }
};