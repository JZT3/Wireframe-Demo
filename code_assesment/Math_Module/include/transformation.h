#pragma once
#include <vector>
#include <variant>
#include <functional>
#include "matrix4x4.h"

namespace Math {
    // Define transformation types
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

    // Transformation variant type for command pattern
    using TransformOp = std::variant<Translation, Rotation, Scale, Matrix4x4>;

    class TransformationPipeline {
    private:
        std::vector<TransformOp> operations;
        mutable bool matrixDirty = true;
        mutable Matrix4x4 cachedMatrix;

        // Visitor for applying transformations
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
                default: return result;
                }
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
        void addTranslation(float x, float y, float z) noexcept {
            operations.emplace_back(Translation{ x, y, z });
            matrixDirty = true;
        }

        void addRotationX(float angle) noexcept {
            operations.emplace_back(Rotation{ Rotation::Axis::X, angle });
            matrixDirty = true;
        }

        void addRotationY(float angle) noexcept {
            operations.emplace_back(Rotation{ Rotation::Axis::Y, angle });
            matrixDirty = true;
        }

        void addRotationZ(float angle) noexcept {
            operations.emplace_back(Rotation{ Rotation::Axis::Z, angle });
            matrixDirty = true;
        }

        void addScale(float x, float y, float z) noexcept {
            operations.emplace_back(Scale{ x, y, z });
            matrixDirty = true;
        }

        // Lazy evaluation - only compute when needed
        [[nodiscard]] const Matrix4x4& getTransformMatrix() const noexcept {
            if (matrixDirty) {
                cachedMatrix = Matrix4x4(); // Start with identity
                for (const auto& op : operations) {
                    cachedMatrix = std::visit(ApplyTransform{ cachedMatrix }, op);
                }
                matrixDirty = false;
            }
            return cachedMatrix;
        }

        // Reset the pipeline
        void clear() noexcept {
            operations.clear();
            matrixDirty = true;
        }
    };
}