#pragma once
#include <cstddef>

namespace Render {
    class Edge {
    private:
        std::size_t vertexIndex1;
        std::size_t vertexIndex2;

    public:
        explicit Edge(std::size_t v1, std::size_t v2) noexcept : vertexIndex1(v1), vertexIndex2(v2) {}

        [[nodiscard]] std::size_t getVertex1Index() const noexcept {
            return vertexIndex1;
        }

        [[nodiscard]] std::size_t getVertex2Index() const noexcept {
            return vertexIndex2;
        }
    };
}