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
		constexpr float M_pi = 3.14159265358979323846264338327950288f;

		// Create renderer
		Renderer renderer(width, height);

		// Create Tetrahedron
		const float tetrahedronSize = std::min(width, height) * 0.25f;
		WireFrameObject tetrahedron = WireFrameObject::createTetrahedron(tetrahedronSize);

		// Apply rotation
		const Matrix4x4 initRotation = Matrix4x4::createRotationX(0.5f) * Matrix4x4::createRotationY(0.5f);
		tetrahedron.transform(initRotation);

		std::cout << "Rendering" << totalFrames << " frames..." << std::endl;

		// Generate animation frames
		for (int frame = 0; frame < totalFrames; ++frame) {
			// Clear framebuffer
			renderer.clear(Color::Black());

			// Calculate rotation angle based on frame
			const float angle = (2.0f * M_pi * frame) / totalFrames;

			// Create rotation matrices
			const Matrix4x4 rotMatrixY = Matrix4x4::createRotationY(angle);
			const Matrix4x4 rotMatrixX = Matrix4x4::createRotationX(angle * 0.5f);
			const Matrix4x4 combinedMatrix = rotMatrixX * rotMatrixY;

			// Copy Tetrahedron and copy it and apply rotation then draw wireframe
			WireFrameObject rotated_tetra = tetrahedron;
			rotated_tetra.transform(combinedMatrix);
			renderer.drawWireFrameObject(rotated_tetra, vertexRadius);

			// Save frame
			if (!renderer.saveFrame("tetrahedron")) {
				std::cerr << "Failed to save frame" << frame << std::endl;
				return 1;
			}

			std::cout << "Frame " << frame + 1 << "/" << totalFrames << "rendered\r" << std::flush;
		}

		return 0;
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
}