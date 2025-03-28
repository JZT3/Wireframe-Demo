#pragma once

namespace Render {
    // Forward declaration
    class Renderer;

    // Strategy pattern for rendering different objects
    class IRenderable {
    public:
        virtual void render(Renderer& renderer) const noexcept = 0;
        virtual ~IRenderable() = default;
    };
}