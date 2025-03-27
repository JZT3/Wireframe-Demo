#pragma once
#include <variant>
#include <vector>
#include <memory>
#include <functional>
#include "Vector3D.h"
#include "Matrix4x4.h"

// Forward declarations
class Vertex;

// Algebraic data type for different transformation types
struct Translation {
    float x, y, z;
};

struct Rotation {
    enum class Axis { X, Y, Z };
    Axis axis;
    float angle;
};

struct Scale {
    float x, y, z;
};

// Using std::variant to create an algebraic data type for transformations
using TransformOp = std::variant<Translation, Rotation, Scale, Matrix4x4>;

class TransformationPipeline {
private:
    // Store a sequence of transformation operations
    std::vector<TransformOp> operations;

    // Cached transformation matrix - only computed when needed
    mutable bool matrixDirty = true;
    mutable Matrix4x4 cachedMatrix;

    // Visitor to apply a transformation operation
    struct ApplyTransform {
        Matrix4x4& result;

        Matrix4x4 operator()(const Translation& t) const {
            return Matrix4x4::createTranslation(t.x, t.y, t.z) * result;
        }

        Matrix4x4 operator()(const Rotation& r) const {
            switch (r.axis) {
            case Rotation::Axis::X: return Matrix4x4::createRotationX(r.angle) * result;
            case Rotation::Axis::Y: return Matrix4x4::createRotationY(r.angle) * result;
            case Rotation::Axis::Z: return Matrix4x4::createRotationZ(r.angle) * result;
            }
            return result; // Unreachable but avoids warning
        }

        Matrix4x4 operator()(const Scale& s) const {
            return Matrix4x4::createScale(s.x, s.y, s.z) * result;
        }

        Matrix4x4 operator()(const Matrix4x4& m) const {
            return m * result;
        }
    };

public:
    TransformationPipeline() = default;

    // Add operations to the pipeline
    void addTranslation(float x, float y, float z) {
        operations.emplace_back(Translation{ x, y, z });
        matrixDirty = true;
    }

    void addRotationX(float angle) {
        operations.emplace_back(Rotation{ Rotation::Axis::X, angle });
        matrixDirty = true;
    }

    void addRotationY(float angle) {
        operations.emplace_back(Rotation{ Rotation::Axis::Y, angle });
        matrixDirty = true;
    }

    void addRotationZ(float angle) {
        operations.emplace_back(Rotation{ Rotation::Axis::Z, angle });
        matrixDirty = true;
    }

    void addRotation(Rotation::Axis axis, float angle) {
        operations.emplace_back(Rotation{ axis, angle });
        matrixDirty = true;
    }

    void addScale(float x, float y, float z) {
        operations.emplace_back(Scale{ x, y, z });
        matrixDirty = true;
    }

    void addMatrix(const Matrix4x4& matrix) {
        operations.emplace_back(matrix);
        matrixDirty = true;
    }

    // Lazy evaluation - only compute the matrix when requested
    [[nodiscard]] const Matrix4x4& getTransformMatrix() const {
        if (matrixDirty) {
            cachedMatrix = Matrix4x4(); // Start with identity

            // Apply all operations in sequence
            for (const auto& op : operations) {
                cachedMatrix = std::visit(ApplyTransform{ cachedMatrix }, op);
            }

            matrixDirty = false;
        }

        return cachedMatrix;
    }

    // Apply the transformation to a vector
    [[nodiscard]] Vector3D transform(const Vector3D& v) const {
        return getTransformMatrix().transform(v);
    }

    // Reset the pipeline
    void clear() {
        operations.clear();
        matrixDirty = true;
    }
};