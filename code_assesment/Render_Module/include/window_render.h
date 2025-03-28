#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include <memory>

// Forward declarations
namespace Render {
    class WireframeObject;
}

namespace Render {
    class WindowRenderer {
    private:
        // Forward declaration of implementation class
        class Impl;

        // Opaque pointer to implementation
        std::unique_ptr<Impl> pImpl;

        // Window procedure handler - needs to be a member of WindowRenderer
        LRESULT HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        // Static window procedure callback
        static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        bool initialized = false;

    public:
        // Constructor/destructor
        WindowRenderer(int width, int height, const std::wstring& title);
        ~WindowRenderer();

        // Prevent copying
        WindowRenderer(const WindowRenderer&) = delete;
        WindowRenderer& operator=(const WindowRenderer&) = delete;

        // Allow moving
        WindowRenderer(WindowRenderer&&) noexcept;
        WindowRenderer& operator=(WindowRenderer&&) noexcept;

        // Public interface
        void LoadObject(std::unique_ptr<WireframeObject> newObject);
        void LoadTetrahedron();
        int Run();
        void Resize(int width, int height);
        void OpenFileDialog();
    };
}