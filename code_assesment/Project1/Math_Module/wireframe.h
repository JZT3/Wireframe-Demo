#pragma once
#include <vector>
#include <cmath>
#include "vertex.h"
#include "edge.h"
#include "matrix4x4.h"

class WireFrameObject {
private:
	std::vector<Vertex> vertices;
	std::vector<Edge> edges;

public:
	WireFrameObject() noexcept = default;


	WireFrameObject(const WireFrameObject&) = default;
	WireFrameObject& operator=(const WireFrameObject&) = default;
	WireFrameObject(WireFrameObject&&) noexcept = default;
	WireFrameObject& operator=(WireFrameObject&&) noexcept = default;
	~WireFrameObject() noexcept = default;

	void addVertex(const Vertex& vertex) {
		vertices.push_back(vertex);
	}

	void addEdge(const Edge& edge) {
		edges.push_back(edge);
	}


	[[nodiscard]] const std::vector<Vertex>& getVertices() const noexcept {
		return vertices;
	}

	[[nodiscard]] const std::vector<Edge>& getEdges() const noexcept {
		return edges;
	}


	void transform(const Matrix4x4& matrix) {
		for (auto& vertex : vertices) {
			vertex.transform(matrix);
		}
	}


	// Factory method to create a tetrahedron
	[[nodiscard]] static WireFrameObject createTetrahedron(float size) {
		WireFrameObject tetrahedron;

		const float h = size * std::sqrt(2.0f/3.0f);
		const float h_third = h/3.0f;
		const float half_size = size/2.0f;
		const float divisor = 2.0f * std::sqrt(3.0f);

		tetrahedron.addVertex(Vertex(0, h, 0)); // Top vertex
		tetrahedron.addVertex(Vertex(-half_size, -h_third, -size / divisor)); // Bottom left
		tetrahedron.addVertex(Vertex(half_size, -h_third, -size / divisor)); // Bottom right
		tetrahedron.addVertex(Vertex(0, -h_third, size / std::sqrt(3.0f))); // Bottom back

		// Define edges
		tetrahedron.addEdge(Edge(0, 1)); // Top to bottom left
		tetrahedron.addEdge(Edge(0, 2)); // Top to bottom right
		tetrahedron.addEdge(Edge(0, 3)); // Top to bottom back
		tetrahedron.addEdge(Edge(1, 2)); // Bottom left to bottom right
		tetrahedron.addEdge(Edge(1, 3)); // Bottom left to bottom back
		tetrahedron.addEdge(Edge(2, 3)); // Bottom right to bottom back

		return tetrahedron;

	}
};