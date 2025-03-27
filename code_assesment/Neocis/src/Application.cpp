#include <iostream>
#include <cmath>
#include <algorithm>
#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include "vector3d.h"
#include "matrix4x4.h"
#include "wireframe.h"
#include "renderer.h"

int main(){
	try {
		// Constants
		constexpr int width = 800;
		constexpr int height = 600;
		constexpr float rotationSpeed = 0.1f;
		constexpr int vertexRadius = 5;
		constexpr int totalFrames = 36;

		// Create renderer
		Renderer renderer(width, height);

		// Create Tetrahedron
		const float tetrahedronSize = std::min(width, height) * 0.25f;
		WireFrameObject tetrahedron = WireFrameObject::createTetrahedron(tetrahedronSize);

		// Apply rotation
		const Matrix4x4 initRotation = Matrix4x4::createRotationX(0.5f) * Matrix4x4::cr
	}
}