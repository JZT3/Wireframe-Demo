#include "window_render.h"
#include <windowsx.h>
#include <commdlg.h>
#include "renderer.h"
#include "framebuffer.h"
#include "wireframe.h"
#include "object_loader.h"
#include "transformation.h"


namespace Render {
    // Definition of the implementation class
    class WindowRenderer::Impl {
    public:
        // Window and rendering resources
        HWND hwnd;
        HDC hdc;
        HDC memDC;
        HBITMAP memBitmap;
        HBITMAP oldBitmap;
        int width;
        int height;
        LARGE_INTEGER frequency;
        LARGE_INTEGER lastTime;

        // Object and transformation state
        std::unique_ptr<WireframeObject> object;
        Math::TransformationPipeline transformPipeline;
        bool objectLoaded;

        // Mouse interaction state
        bool mouseDown;
        int lastMouseX;
        int lastMouseY;
        float rotationX;
        float rotationY;

        // Constructor
        // Replace the old constructor with this new version
        Impl(HWND hwnd, int width, int height)
            : hwnd(hwnd), width(width), height(height),
            objectLoaded(false), mouseDown(false),
            rotationX(0.0f), rotationY(0.0f),
            memDC(NULL), memBitmap(NULL), oldBitmap(NULL) {

            // Get device context
            hdc = GetDC(hwnd);

            // Initialize double buffering
            CreateBackBuffer();

            // Initialize timing
            QueryPerformanceFrequency(&frequency);
            QueryPerformanceCounter(&lastTime);
        }

        // Destructor
        ~Impl() {
            // Clean up GDI resources
            if (memDC) {
                SelectObject(memDC, oldBitmap);
                DeleteObject(memBitmap);
                DeleteDC(memDC);
            }

            ReleaseDC(hwnd, hdc);
        }

        // Create back buffer for double buffering
        void CreateBackBuffer() {
            if (memDC) {
                SelectObject(memDC, oldBitmap);
                DeleteObject(memBitmap);
                DeleteDC(memDC);
            }

            memDC = CreateCompatibleDC(hdc);
            memBitmap = CreateCompatibleBitmap(hdc, width, height);
            oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);
        }

        // Render current frame
        void RenderFrame() {
            // Create frame buffer for the current size
            auto frameBuffer = std::make_shared<FrameBuffer>(width, height);
            Renderer renderer(frameBuffer);

            // Clear with black background
            renderer.clear(Color::Black());

            // Render object if loaded
            if (objectLoaded && object) {
                // Create a copy of the object
                WireframeObject transformedObject = *object;

                // Apply transformations
                transformedObject.transform(transformPipeline.getTransformMatrix());

                // Render the transformed object
                renderer.drawWireframeObject(transformedObject, 3, Color::Blue());
            }

            // Copy frame buffer to the device context
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    Color color = frameBuffer->getPixel(x, y);
                    SetPixel(memDC, x, y, RGB(color.r, color.g, color.b));
                }
            }

            // Display instructions
            SetTextColor(memDC, RGB(255, 255, 255));
            SetBkMode(memDC, TRANSPARENT);
            RECT textRect = { 10, 10, width - 10, 30 };
            DrawText(memDC, TEXT("Left-click and drag to rotate. Right-click to load object."), -1, &textRect, DT_LEFT);

            // Blit to the window
            BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);
        }

        // Mouse movement handler
        void OnMouseMove(int x, int y) {
            if (mouseDown && objectLoaded) {
                // Calculate mouse movement delta
                int deltaX = x - lastMouseX;
                int deltaY = y - lastMouseY;

                // Update rotation angles
                const float rotationFactor = 0.01f;
                rotationY += deltaX * rotationFactor;
                rotationX += deltaY * rotationFactor;

                // Update transformation
                transformPipeline.clear();
                transformPipeline.addTranslation(0.0f, 0.0f, 3.0f);
                transformPipeline.addRotationX(rotationX);
                transformPipeline.addRotationY(rotationY);

                // Update mouse position
                lastMouseX = x;
                lastMouseY = y;

                // Request redraw
                InvalidateRect(hwnd, NULL, TRUE);
            }
        }
    };

    // Static window procedure implementation - redirects to instance method
    LRESULT CALLBACK WindowRenderer::StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        WindowRenderer* instance = nullptr;
        
        // Get the object pointer from window's user data or creation params
        if (uMsg == WM_CREATE || uMsg == WM_CREATE) {
            // Store the WindowRenderer pointer during creation
            CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
            instance = reinterpret_cast<WindowRenderer*>(cs->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(instance));
        }
        else {
            instance = reinterpret_cast<WindowRenderer*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }

        // Forward to instance method if available
        if (instance) {
            return instance->HandleMessage(hwnd, uMsg, wParam, lParam);
        }

        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    // Instance method to handle window messages
    LRESULT WindowRenderer::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_SIZE:
            Resize(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            pImpl->RenderFrame();
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_LBUTTONDOWN:
            pImpl->mouseDown = true;
            pImpl->lastMouseX = LOWORD(lParam);
            pImpl->lastMouseY = HIWORD(lParam);
            SetCapture(hwnd);
            return 0;

        case WM_LBUTTONUP:
            pImpl->mouseDown = false;
            ReleaseCapture();
            return 0;

        case WM_MOUSEMOVE:
            pImpl->OnMouseMove(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_RBUTTONDOWN:
            OpenFileDialog();
            return 0;
        }

        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    // Constructor implementation
    WindowRenderer::WindowRenderer(int width, int height, const std::wstring& title) {
        WNDCLASSEX wc = { 0 };
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = StaticWindowProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wc.lpszClassName = L"WireframeRendererClass";

        RegisterClassEx(&wc);

        // Set this instance in user data
        SetProp(GetDesktopWindow(), L"CurrentRenderer", reinterpret_cast<HANDLE>(this));

        // Now create window
        HWND hwnd = CreateWindowEx(
            0,                      // Optional window styles
            L"WireframeRendererClass", // Window class
            title.c_str(),          // Window title
            WS_OVERLAPPEDWINDOW,    // Window style
            CW_USEDEFAULT, CW_USEDEFAULT, width, height, // Size and position
            NULL, NULL, GetModuleHandle(NULL), this     // Pass this as creation parameter
        );

        if (!hwnd) {
            MessageBoxA(NULL, "Failed to create window", "Error", MB_ICONERROR);
            throw std::runtime_error("Failed to create window");
        }

        // After window is created, then create implementation
        pImpl = std::make_unique<Impl>(hwnd, width, height);
        initialized = true;

        // Show the window now that everything is initialized
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
    }

    // Destructor implementation
    WindowRenderer::~WindowRenderer() = default;

    // Move constructor
    WindowRenderer::WindowRenderer(WindowRenderer&& other) noexcept = default;

    // Move assignment
    WindowRenderer& WindowRenderer::operator=(WindowRenderer&&) noexcept = default;

    // Forward public methods to implementation
    void WindowRenderer::LoadObject(std::unique_ptr<WireframeObject> newObject) {
        // Null Check
        if (!pImpl) return;
        pImpl->object = std::move(newObject);
        pImpl->objectLoaded = true;

        // Reset rotation
        pImpl->rotationX = 0.0f;
        pImpl->rotationY = 0.0f;

        // Clear transformations and add default ones
        pImpl->transformPipeline.clear();
        pImpl->transformPipeline.addTranslation(0.0f, 0.0f, 3.0f); // Move back so object is visible

        // Trigger redraw
        InvalidateRect(pImpl->hwnd, NULL, TRUE);
    }

    void WindowRenderer::LoadTetrahedron() {
        LoadObject(WireframeObject::createTetrahedron(1.0f));
    }

    int WindowRenderer::Run() {
        MSG msg = { 0 };
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return static_cast<int>(msg.wParam);
    }

    // Add null checks to all methods that use pImpl
    void WindowRenderer::Resize(int width, int height) {
        if (pImpl) {  // Check if pImpl is valid
            pImpl->width = width;
            pImpl->height = height;
            pImpl->CreateBackBuffer();
            InvalidateRect(pImpl->hwnd, NULL, TRUE);
        }
    }

    void WindowRenderer::OpenFileDialog() {
        OPENFILENAMEA ofn;
        char szFile[260] = { 0 };

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = pImpl->hwnd;
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = "Object Files\0*.obj;*.csv;*.txt\0All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        if (GetOpenFileNameA(&ofn)) {
            try {
                ObjectLoader loader;
                auto newObject = loader.loadFromCSV(ofn.lpstrFile);
                LoadObject(std::move(newObject));
            }
            catch (const std::exception& e) {
                MessageBoxA(pImpl->hwnd, e.what(), "Error", MB_ICONERROR);
            }
        }
    }
}