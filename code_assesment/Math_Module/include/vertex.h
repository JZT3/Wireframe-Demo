#pragma once
#include "vector3D.h"
#include "matrix4x4.h"

namespace Render {
    class Vertex {
    private:
        Math::Vector3D position;

    public:
        explicit Vertex(float x, float y, float z) noexcept : position(Math::Vector3D(x, y, z)) {}
        explicit Vertex(const Math::Vector3D& pos) noexcept : position(pos) {}

        [[nodiscard]] const Math::Vector3D& getPosition() const noexcept {
            return position;
        }

        void setPosition(const Math::Vector3D& pos) noexcept {
            position = pos;
        }

        void transform(const Math::Matrix4x4& matrix) noexcept {
            position = matrix.transform(position);
        }
    };
}