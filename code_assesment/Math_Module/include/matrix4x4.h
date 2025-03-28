#pragma once
#include <cmath>
#include "Vector3D.h"

namespace Math {
    class Matrix4x4 {
    private:
        float m[4][4];

    public:
        // Initialize identity matrix
        constexpr Matrix4x4() noexcept {
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    m[i][j] = (i == j) ? 1.0f : 0.0f;
                }
            }
        }

        constexpr void set(int row, int col, float value) noexcept {
            m[row][col] = value;
        }

        [[nodiscard]] constexpr float get(int row, int col) const noexcept {
            return m[row][col];
        }

        [[nodiscard]] Matrix4x4 operator*(const Matrix4x4& other) const noexcept {
            Matrix4x4 result;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    float sum = 0.0f;
                    for (int k = 0; k < 4; k++) {
                        sum += m[i][k] * other.m[k][j];
                    }
                    result.m[i][j] = sum;
                }
            }
            return result;
        }

        [[nodiscard]] Vector3D transform(const Vector3D& v) const noexcept {
            const float x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3];
            const float y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3];
            const float z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3];
            const float w = m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3];

            if (std::abs(w) > 1e-6f) {
                return Vector3D(x / w, y / w, z / w);
            }
            return Vector3D(x, y, z);
        }

        // Static factory methods for transformation matrices
        [[nodiscard]] static Matrix4x4 createTranslation(float x, float y, float z) noexcept {
            Matrix4x4 result;
            result.m[0][3] = x;
            result.m[1][3] = y;
            result.m[2][3] = z;
            return result;
        }

        [[nodiscard]] static Matrix4x4 createRotationX(float angle) noexcept {
            const float cos_a = std::cos(angle);
            const float sin_a = std::sin(angle);

            Matrix4x4 result;
            result.m[1][1] = cos_a;
            result.m[1][2] = -sin_a;
            result.m[2][1] = sin_a;
            result.m[2][2] = cos_a;
            return result;
        }

        [[nodiscard]] static Matrix4x4 createRotationY(float angle) noexcept {
            const float cos_a = std::cos(angle);
            const float sin_a = std::sin(angle);

            Matrix4x4 result;
            result.m[0][0] = cos_a;
            result.m[0][2] = sin_a;
            result.m[2][0] = -sin_a;
            result.m[2][2] = cos_a;
            return result;
        }

        [[nodiscard]] static Matrix4x4 createRotationZ(float angle) noexcept {
            const float cos_a = std::cos(angle);
            const float sin_a = std::sin(angle);

            Matrix4x4 result;
            result.m[0][0] = cos_a;
            result.m[0][1] = -sin_a;
            result.m[1][0] = sin_a;
            result.m[1][1] = cos_a;
            return result;
        }

        [[nodiscard]] static Matrix4x4 createScale(float sx, float sy, float sz) noexcept {
            Matrix4x4 result;
            result.m[0][0] = sx;
            result.m[1][1] = sy;
            result.m[2][2] = sz;
            return result;
        }
    };
}