#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdexcept>
#include "Window_Render.h"

// Application entry point
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    try {        
        // Create window renderer with 800x600 resolution
        Render::WindowRenderer renderer(800, 600, L"3D Wireframe Viewer");

        // Load the default tetrahedron
        renderer.LoadTetrahedron();

        // Run the main message loop
        return renderer.Run();
    }
    catch (const std::exception& e) {
        MessageBoxA(NULL, e.what(), "Error", MB_ICONERROR);
        return 1;
    }
}