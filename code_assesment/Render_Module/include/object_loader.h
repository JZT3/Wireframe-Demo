#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <memory>
#include <map>
#include <limits>
#include <algorithm>
#include <stdexcept>
#include "wireframe.h"
#include "vector3D.h"
#include "matrix4x4.h"

#undef max
#undef min

namespace Render {
    class ObjectLoader {
    private:
        std::map<int, size_t> vertexMap;

        // Normalize object to fit in [-1,1] cube
        void normalizeObject(WireframeObject& object) const noexcept {
            const auto& vertices = object.getVertices();
            if (vertices.empty()) return;

            Math::Vector3D min{ std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max(),
                std::numeric_limits<float>::max() };
            Math::Vector3D max{ std::numeric_limits<float>::lowest(),
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

            Math::Vector3D center(
                (min.x + max.x) / 2.0f,
                (min.y + max.y) / 2.0f,
                (min.z + max.z) / 2.0f
            );

            // Calculate max dimension
            float dx = max.x - min.x;
            float dy = max.y - min.y;
            float dz = max.z - min.z;
            float maxDim = std::max({ dx, dy, dz });

            float scale = 2.0f / maxDim; // Scale to fit in [-1, 1] cube

            // Create transformation matrices
            Math::Matrix4x4 translation = Math::Matrix4x4::createTranslation(-center.x, -center.y, -center.z);
            Math::Matrix4x4 scaling = Math::Matrix4x4::createScale(scale, scale, scale);

            object.transform(translation);
            object.transform(scaling);
        }

    public:

        [[nodiscard]] std::unique_ptr<WireframeObject> loadFromCSV(const std::string& filename) {
            std::ifstream file(filename);
            if (!file.is_open()) {
                throw std::runtime_error("Failed to open file: " + filename);
            }

            auto object = std::make_unique<WireframeObject>();
            vertexMap.clear();

            std::string line;
            bool readingVertices = true;
            bool hasEdges = false;

            while (std::getline(file, line)) {
                // Skip empty lines and comments
                if (line.empty() || line[0] == '#') continue;

                std::istringstream iss(line);
                std::string token;

                // Check for section marker
                if (line == "VERTICES") {
                    readingVertices = true;
                    continue;
                }
                else if (line == "FACES" || line == "EDGES") {
                    readingVertices = false;
                    continue;
                }

                // Parse based on current section
                if (readingVertices) {
                    // Format: id,x,y,z
                    int id;
                    float x, y, z;
                    char comma;

                    if (iss >> id >> comma >> x >> comma >> y >> comma >> z) {
                        vertexMap[id] = object->getVertices().size();
                        object->addVertex(Vertex(x, y, z));
                    }
                }
                else {
                    // Format: v1,v2,v3 (for faces) or v1,v2 (for edges)
                    std::string edgeStr;
                    std::vector<int> vertices;

                    while (std::getline(iss, edgeStr, ',')) {
                        try {
                            vertices.push_back(std::stoi(edgeStr));
                        }
                        catch (const std::exception&) {
                            // Skip invalid entries
                        }
                    }

                    if (vertices.size() >= 2) {
                        // Direct edge
                        size_t index1 = vertexMap[vertices[0]];
                        size_t index2 = vertexMap[vertices[1]];
                        object->addEdge(Edge(index1, index2));

                        // For faces, add additional edges
                        if (vertices.size() >= 3) {
                            size_t index3 = vertexMap[vertices[2]];
                            object->addEdge(Edge(index2, index3));
                            object->addEdge(Edge(index3, index1));
                        }
                    }
                }
            }

            // If no edges were specified but we have vertices, generate edges automatically
    // for point cloud data
            if (!hasEdges && object->getVertices().size() > 0) {
                GenerateEdgesFromPointCloud(*object);
            }
            normalizeObject(*object);
            return object;
        }

        void GenerateEdgesFromPointCloud(WireframeObject& object) {
            const auto& vertices = object.getVertices();
            size_t vertexCount = vertices.size();

            // For small point clouds, create a complete graph
            if (vertexCount <= 20) {
                for (size_t i = 0; i < vertexCount; ++i) {
                    for (size_t j = i + 1; j < vertexCount; ++j) {
                        object.addEdge(Edge(i, j));
                    }
                }
            }
            else {
                // For larger point clouds, connect nearest neighbors
                const int MAX_CONNECTIONS = 3; // Connect each point to its 3 nearest neighbors

                for (size_t i = 0; i < vertexCount; ++i) {
                    // Calculate distances to all other vertices
                    std::vector<std::pair<float, size_t>> distances;
                    const auto& pos1 = vertices[i].getPosition();

                    for (size_t j = 0; j < vertexCount; ++j) {
                        if (i != j) {
                            const auto& pos2 = vertices[j].getPosition();
                            float dx = pos2.x - pos1.x;
                            float dy = pos2.y - pos1.y;
                            float dz = pos2.z - pos1.z;
                            float distSq = dx * dx + dy * dy + dz * dz;
                            distances.push_back({ distSq, j });
                        }
                    }

                    std::sort(distances.begin(), distances.end());

                    // Connect to nearest neighbors
                    for (int k = 0; k < std::min(MAX_CONNECTIONS, static_cast<int>(distances.size())); ++k) {
                        object.addEdge(Edge(i, distances[k].second));
                    }
                }
            }
        }
    };
}