#pragma once
#include "vector3D.h"
#include "matrix4x4.h"

class Vertex {
private:
	Vector3D position;

public:
	explicit Vertex(float x, float y, float z) noexcept : position(Vector3D(x, y, z)) {}
	explicit Vertex(const Vector3D& pos) noexcept : position(pos) {}

	[[nodiscard]] const Vector3D& getPosition() const noexcept {
		return position;
	}

	void setPosition(const Vector3D& pos) noexcept {
		position = pos;
	}

	void transform(const Matrix4x4& matrix) noexcept {
		position = matrix.transform(position);
	}
	
};