#pragma once

#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include "vector3d.h"
#include "wireframe.h"

#undef max
#undef min

class ObjectLoader {
private:
	std::map<int, size_t> vertexMap;

public:
	WireFrameObject loadObject(const std::string& filename) {
		std::ifstream file(filename);
		if (!file.is_open()) {
			throw std::runtime_error("Failed to open file: " + filename);
		}

		WireFrameObject object;
		vertexMap.clear();

		// Read Header: num of vertices and faces
		int numVertices = 0;
		int numFaces = 0;

		file >> numVertices >> numFaces;

		// Read vertices
		for (int i = 0; i < numVertices; ++i) {
			int id;
			float x, y, z;
			file >> id >> x >> y >> z;

			vertexMap[id] = object.getVertices().size();
			object.addVertex(Vertex(x, y, z));
		}

		// Read faces
		for (int i = 0; i < numFaces; ++i) {
			int v1, v2, v3;
			file >> v1 >> v2 >> v3;

			// Convert vertex ids into indices using map
			size_t index1 = vertexMap[v1];
			size_t index2 = vertexMap[v2];
			size_t index3 = vertexMap[v3];

			// Add edges
			object.addEdge(Edge(index1, index2));
			object.addEdge(Edge(index2, index3));
			object.addEdge(Edge(index3, index1));
		}

		normalizeObject(object);

		return object;
	}

private:
    void normalizeObject(WireFrameObject& object) {
        const auto& vertices = object.getVertices();
        if (vertices.empty()) return;

        Vector3D min{ std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max() };
        Vector3D max{ std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::lowest() };

        for (const auto& vertex : vertices) {
            const auto& pos = vertex.getPosition();
            min.x = std::min(min.x, pos.x);
            min.y = std::min(min.y, pos.y);
            min.z = std::min(min.z, pos.z);
            max.x = std::max(max.x, pos.x);
            max.y = std::max(max.y, pos.y);
            max.z = std::max(max.z, pos.z);
        }

        Vector3D center(
            (min.x + max.x) / 2.0f,
            (min.y + max.y) / 2.0f,
            (min.z + max.z) / 2.0f
        );

        // calc max dimension
        float dx = max.x - min.x;
        float dy = max.y - min.y;
        float dz = max.z - min.z;
		float dw = (std::max(dx, dy));
        float maxDim = std::max(dw, dz);

        float scale = 2.0f / maxDim; // Scale to fit in [-1, 1] cube

        // Create translation and scaling matrices
        Matrix4x4 translation = Matrix4x4::createTranslation(-center.x, -center.y, -center.z);
        Matrix4x4 scaling = Matrix4x4::createScale(scale, scale, scale);

        object.transform(translation);
        object.transform(scaling);
    }

};