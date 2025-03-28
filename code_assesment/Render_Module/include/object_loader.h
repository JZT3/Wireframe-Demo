#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <memory>
#include <map>
#include <limits>
#include <functional>
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
        std::function<std::unique_ptr<WireframeObject>()> objectFactory; // Injected factory

        void normalizeObject(std::unique_ptr<WireframeObject>& object) const noexcept {
            if (!object || object->getVertices().empty()) return;

            const auto& vertices = object->getVertices();
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

            float maxDim = std::max({ max.x - min.x, max.y - min.y, max.z - min.z });
            float scale = 2.0f / maxDim; // Scale to fit in [-1,1] cube

            object->transform(Math::Matrix4x4::createTranslation(-center.x, -center.y, -center.z));
            object->transform(Math::Matrix4x4::createScale(scale, scale, scale));
        }

    public:
        explicit ObjectLoader(std::function<std::unique_ptr<WireframeObject>()> factory = []() {
            return std::make_unique<WireframeObject>();
            }) : objectFactory(std::move(factory)) {
        }

            [[nodiscard]] std::unique_ptr<WireframeObject> loadFromCSV(const std::string& filename) {
                std::ifstream file(filename);
                if (!file.is_open()) {
                    throw std::runtime_error("Failed to open file: " + filename);
                }

                auto object = objectFactory(); // Use injected factory
                vertexMap.clear();

                std::string line;
                if (!std::getline(file, line)) {
                    throw std::runtime_error("Empty file or failed to read header");
                }

                std::istringstream headerStream(line);
                int vertexCount, faceCount;
                char comma;

                if (headerStream >> vertexCount >> comma >> faceCount && comma == ',') {
                    
                }
                else {
                    headerStream.clear();
                    headerStream.seekg(0);
                    if (!(headerStream >> vertexCount >> faceCount)) {
                        throw std::runtime_error("Invalid header format: expected two integers");
                    }
                }

                // Read vertices
                for (int i = 0; i < vertexCount; ++i) {
                    if (!std::getline(file, line)) {
                        throw std::runtime_error("Unexpected end of file while reading vertices");
                    }

                    // Skip empty lines and comments
                    if (line.empty() || line[0] == '#' || line[0] == '%') {
                        --i; // Don't count this line as a vertex
                        continue;
                    }

                    std::istringstream vertexStream(line);
                    int id;
                    float x, y, z;
                    char comma;

                    if (vertexStream >> id >> comma >> x >> comma >> y >> comma >> z) {
                        vertexMap[id] = object->getVertices().size();
                        object->addVertex(Vertex(x, y, z));
                    }
                    else {
                        throw std::runtime_error("Invalid vertex format at line " + std::to_string(i + 2));
                    }
                }


                // Read Faces
                for (int i = 0; i < faceCount; ++i) {
                    if (!std::getline(file, line)) {
                        throw std::runtime_error("Unexpected end of file while reading faces");
                    }

                    // Skip empty lines and comments
                    if (line.empty() || line[0] == '#' || line[0] == '%') {
                        --i; // Don't count this as a face
                        continue;
                    }

                    std::istringstream faceStream(line);
                    int v1, v2, v3;
                    char comma;
                    bool validFormat = false;

                    // Try comma-separated first
                    if (faceStream >> v1 >> comma >> v2 >> comma >> v3 && comma == ',') {
                        validFormat = true;
                    }
                    else {
                        // Reset stream and try space-separated
                        faceStream.clear();
                        faceStream.seekg(0);
                        if (faceStream >> v1 >> v2 >> v3) {
                            validFormat = true;
                        }
                    }

                    if (!validFormat) {
                        throw std::runtime_error("Invalid face format at line " +
                            std::to_string(vertexCount + i + 2));
                    }
                    
                    if (vertexMap.find(v1) == vertexMap.end() ||
                        vertexMap.find(v2) == vertexMap.end() ||
                        vertexMap.find(v3) == vertexMap.end()) {
                        throw std::runtime_error("Face references non-existent vertex ID at line " +
                            std::to_string(vertexCount + i + 2));
                    }

                    // Get corresponding vertex indices
                    size_t index1 = vertexMap[v1];
                    size_t index2 = vertexMap[v2];
                    size_t index3 = vertexMap[v3];

                    // Validate indices are in range
                    size_t vertexSize = object->getVertices().size();
                    if (index1 >= vertexSize || index2 >= vertexSize || index3 >= vertexSize) {
                        throw std::runtime_error("Invalid vertex index mapping at line " +
                            std::to_string(vertexCount + i + 2));
                    }

                    // Add edges to create triangle
                    object->addEdge(Edge(vertexMap[v1], vertexMap[v2]));
                    object->addEdge(Edge(vertexMap[v2], vertexMap[v3]));
                    object->addEdge(Edge(vertexMap[v3], vertexMap[v1]));
                }
                    
                normalizeObject(object);
                return object;
            }

            void GenerateEdgesFromPointCloud(std::shared_ptr<WireframeObject> object) {
                if (!object) return;

                const auto& vertices = object->getVertices();
                size_t vertexCount = vertices.size();

                if (vertexCount <= 20) {
                    for (size_t i = 0; i < vertexCount; ++i) {
                        for (size_t j = i + 1; j < vertexCount; ++j) {
                            object->addEdge(Edge(i, j));
                        }
                    }
                }
                else {
                    constexpr int MAX_CONNECTIONS = 3;
                    for (size_t i = 0; i < vertexCount; ++i) {
                        std::vector<std::pair<float, size_t>> distances;
                        const auto& pos1 = vertices[i].getPosition();

                        for (size_t j = 0; j < vertexCount; ++j) {
                            if (i != j) {
                                const auto& pos2 = vertices[j].getPosition();
                                float dx = pos2.x - pos1.x;
                                float dy = pos2.y - pos1.y;
                                float dz = pos2.z - pos1.z;
                                float distSq = dx * dx + dy * dy + dz * dz;
                                distances.emplace_back(distSq, j);
                            }
                        }

                        std::sort(distances.begin(), distances.end());

                        for (int k = 0; k < std::min(MAX_CONNECTIONS, static_cast<int>(distances.size())); ++k) {
                            object->addEdge(Edge(i, distances[k].second));
                        }
                    }
                }
            }
    };
}