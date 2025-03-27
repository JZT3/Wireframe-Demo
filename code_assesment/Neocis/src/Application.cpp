#include <iostream>
#include <cmath>
#include <memory>
#include <vector>
#include <functional>
#include "vector2d.h"
#include "vector3d.h"
#include "matrix4x4.h"
#include "wireframe.h"
#include "lazy_render.h"
#include "transformation.h"
#include "framebuffer.h"

constexpr float M_PI = 3.14159265358979323846264338327950288f;

// Lazy frame generator - creates frames on demand
class LazyFrameGenerator {
private:
    WireFrameObject baseObject;
    int totalFrames;
    TransformationPipeline pipeline;
    int width, height;
    int vertexRadius;
    Color objectColor;

    // Frame generation function - creates a single frame on demand
    std::function<std::unique_ptr<FrameBuffer>(int)> frameGenFn;

public:
    LazyFrameGenerator(
        const WireFrameObject& object,
        int totalFrames,
        int width,
        int height,
        int vertexRadius,
        const Color& color
    ) : baseObject(object), totalFrames(totalFrames), width(width), height(height),
        vertexRadius(vertexRadius), objectColor(color) {

        // Create the frame generation function
        frameGenFn = [this](int frameIndex) -> std::unique_ptr<FrameBuffer> {
            // Calculate rotation angle based on frame
            const float angle = (2.0f * M_PI * frameIndex) / this->totalFrames;

            // Create framebuffer for this frame
            auto framebuffer = std::make_unique<FrameBuffer>(this->width, this->height);
            framebuffer->clear(Color::Black());

            // Set up world-to-screen conversion
            auto worldToScreen = [this](const Vector2D& point) -> std::pair<int, int> {
                return {
                    static_cast<int>(point.x + this->width / 2.0f),
                    static_cast<int>(this->height / 2.0f - point.y)
                };
                };

            // Set up lazy renderer
            LazyRenderer renderer(
                this->width,
                this->height,
                worldToScreen,
                [&framebuffer](int x, int y, const Color& color) { framebuffer->setPixel(x, y, color); },
                [&framebuffer](int x, int y) { return framebuffer->getPixel(x, y); }
            );

            // Set up transformation pipeline
            pipeline.clear();
            pipeline.addRotationY(angle);
            pipeline.addRotationX(angle * 0.5f);

            // Create a transformed copy of the base object
            WireFrameObject transformedObject = this->baseObject;
            transformedObject.transform(pipeline.getTransformMatrix());

            // Queue the wireframe object for rendering
            renderer.queueWireframeObject(transformedObject, this->vertexRadius, this->objectColor);

            // Execute all queued rendering operations
            renderer.render();

            return framebuffer;
            };
    }

    // Get a specific frame (computed on-demand)
    std::unique_ptr<FrameBuffer> getFrame(int frameIndex) {
        if (frameIndex < 0 || frameIndex >= totalFrames) {
            throw std::out_of_range("Frame index out of bounds");
        }

        return frameGenFn(frameIndex);
    }

    // Get total number of frames
    int getFrameCount() const {
        return totalFrames;
    }
};


int main(){
	try {
		// Constants
		constexpr int width = 800;
		constexpr int height = 600;
		constexpr float rotationSpeed = 0.1f;
		constexpr int vertexRadius = 5;
		constexpr int totalFrames = 36;
        
		// Create Tetrahedron
		const float tetrahedronSize = std::min(width, height) * 0.25f;
		WireFrameObject tetrahedron = WireFrameObject::createTetrahedron(tetrahedronSize);

        // Apply initial rotation to show 3D effect
        TransformationPipeline initialPipeline;
        initialPipeline.addRotationX(0.5f);
        initialPipeline.addRotationY(0.5f);
        tetrahedron.transform(initialPipeline.getTransformMatrix());

        // Create lazy frame generator
        LazyFrameGenerator frameGenerator(
            tetrahedron,
            totalFrames,
            width,
            height,
            vertexRadius,
            Color::Blue()
        );

        std::cout << "Rendering " << totalFrames << " frames on demand..." << std::endl;

		// Generate animation frames
        for (int frame = 0; frame < totalFrames; ++frame) {
            // Get frame (computed lazily)
            auto framebuffer = frameGenerator.getFrame(frame);

            // Save frame
            const std::string filename = "tetrahedron_" + std::to_string(frame) + ".ppm";
            if (!framebuffer->saveToPPM(filename)) {
                std::cerr << "Failed to save frame " << frame << std::endl;
                return 1;
            }

            std::cout << "Frame " << frame + 1 << "/" << totalFrames << " rendered\r" << std::flush;
        }

		return 0;
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
}