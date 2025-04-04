#include "renderer.h"
#include "wireframe.h"

namespace Render {
    void Renderer::drawWireframeObject(const WireframeObject& object, int vertexRadius, const Color& color) noexcept {
        const auto& vertices = object.getVertices();
        const auto& edges = object.getEdges();

        // Draw all edges
        for (const auto& edge : edges) {
            if (edge.getVertex1Index() < vertices.size() && edge.getVertex2Index() < vertices.size()) {
                const auto& v1 = vertices[edge.getVertex1Index()].getPosition();
                const auto& v2 = vertices[edge.getVertex2Index()].getPosition();
                drawEdge(v1, v2, color);
            }
        }

        // Draw all vertices
        for (const auto& vertex : vertices) {
            drawVertex(vertex.getPosition(), vertexRadius, color);
        }
    }
}