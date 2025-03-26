#pragma once
#include <cstddef>

class Edge {
private:
	size_t vertexIndex1;
	size_t vertexIndex2;

public:
	explicit Edge(size_t v1, size_t v2) noexcept : vertexIndex1(v1), vertexIndex2(v2) {}
	
	[[nodiscard]] size_t getVertex1Index() const noexcept {
		return vertexIndex1;
	}

	[[nodiscard]] size_t getVertex2Index() const noexcept {
		return vertexIndex2;
	}
	
};