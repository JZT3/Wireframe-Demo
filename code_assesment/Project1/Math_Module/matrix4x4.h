#pragma once

// Forward declaration
class Vector3D;

class Matrix4x4 {
private:
	float m[4][4];



public:
	constexpr Matrix4x4() noexcept {
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				if (i == j) {
					m[i][j] = 1.0f;
				} 
				else {
					m[i][j] = 0.0f;
				}
			}
		}
	}


	inline void set(int row, int col, float value) noexcept {
		m[row][col] = value;
	}

	[[nodiscard]] inline float get(int row, int col) const noexcept {
		return m[row][col];
	}


	[[nodiscard]] Matrix4x4 operator*(const Matrix4x4& other) const noexcept {
		Matrix4x4 result;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				result.m[i][j] = 0;
				for (int k = 0; k < 4; k++) {
					result.m[i][j] += m[i][k] * other.m[k][j];
				}
			}
		}
		return result;
	}

	[[nodiscard]] Vector3D transform(const Vector3D& v) const noexcept;

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

