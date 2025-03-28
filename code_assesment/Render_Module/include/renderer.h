#pragma once
#include <memory>
#include <string>
#include "render_target_interface.h"
#include "graphics_primitaves.h"
#include "vector2D.h"
#include "vector3D.h"
#include "projection.h"
#include "framebuffer.h"

namespace Render {
    // Forward declarations
    class WireframeObject;

    // Main renderer class (Facade pattern)
    class Renderer {
    private:
        std::shared_ptr<IRenderTarget> renderTarget;

    public:
        explicit Renderer(std::shared_ptr<IRenderTarget> target) noexcept
            : renderTarget(std::move(target)) {
        }

        void clear(const Color& color = Color::Black()) noexcept {
            renderTarget->clear(color);
        }

        void drawVertex(const Math::Vector3D& position, int radius, const Color& color) noexcept {
            // Project 3D position to 2D
            const Math::Vector2D pos2D = Math::orthographicProject(position);

            // Convert to screen coordinates
            const auto [screen_x, screen_y] = GraphicsPrimitives::worldToScreen(
                pos2D, renderTarget->getWidth(), renderTarget->getHeight());

            // Draw circle
            GraphicsPrimitives::drawCircle(*renderTarget, screen_x, screen_y, radius, color);
        }

        void drawEdge(const Math::Vector3D& start, const Math::Vector3D& end, const Color& color) noexcept {
            // Project 3D positions to 2D
            const Math::Vector2D start2D = Math::orthographicProject(start);
            const Math::Vector2D end2D = Math::orthographicProject(end);

            // Convert to screen coordinates
            const auto [start_x, start_y] = GraphicsPrimitives::worldToScreen(
                start2D, renderTarget->getWidth(), renderTarget->getHeight());
            const auto [end_x, end_y] = GraphicsPrimitives::worldToScreen(
                end2D, renderTarget->getWidth(), renderTarget->getHeight());

            // Draw line
            GraphicsPrimitives::drawLine(*renderTarget, start_x, start_y, end_x, end_y, color);
        }

        // Render a wireframe object
        void drawWireframeObject(const WireframeObject& object, int vertexRadius, const Color& color = Color::Blue()) noexcept;

        // Save the current frame
        bool saveFrame(const std::string& filenamePrefix, int frameCount) const noexcept {
            if (auto* frameBuffer = dynamic_cast<FrameBuffer*>(renderTarget.get())) {
                const std::string filename = filenamePrefix + "_" + std::to_string(frameCount) + ".ppm";
                return frameBuffer->saveToPPM(filename);
            }
            return false;
        }

        [[nodiscard]] int getWidth() const noexcept {
            return renderTarget->getWidth();
        }

        [[nodiscard]] int getHeight() const noexcept {
            return renderTarget->getHeight();
        }
    };
}