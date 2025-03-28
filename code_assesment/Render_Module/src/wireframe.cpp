#include "Wireframe.h"
#include "Renderer.h"

namespace Render {
    void WireframeObject::render(Renderer& renderer) const noexcept {
        constexpr int vertexRadius = 3;
        constexpr Color edgeColor = Color::Blue();
        constexpr Color vertexColor = Color::Red();

        // Draw all edges
        for (const auto& edge : edges) {
            if (edge.getVertex1Index() < vertices.size() && edge.getVertex2Index() < vertices.size()) {
                const auto& v1 = vertices[edge.getVertex1Index()].getPosition();
                const auto& v2 = vertices[edge.getVertex2Index()].getPosition();
                renderer.drawEdge(v1, v2, edgeColor);
            }
        }

        // Draw all vertices
        for (const auto& vertex : vertices) {
            renderer.drawVertex(vertex.getPosition(), vertexRadius, vertexColor);
        }
    }
}