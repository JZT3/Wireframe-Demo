#pragma once
#include "vector3d.h"
#include "vector2d.h"

[[nodiscard]] inline Vector2D orthographicProject(const Vector3D& v) noexcept {
	return Vector2D(v.x, v.y);
}
