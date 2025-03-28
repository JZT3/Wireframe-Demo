#include "window_render.h"
#include <windowsx.h>
#include <commdlg.h>
#include <shlobj_core.h>
#include "renderer.h"
#include "framebuffer.h"
#include "wireframe.h"
#include "object_loader.h"
#include "transformation.h"

#define IDM_FILE_OPEN 1001
#define IDM_FILE_EXIT 1002
#define WM_RENDER_FRAME (WM_USER + 1)  // continuous rendering

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
        bool isDragging;
        float viewDistance;

        UINT_PTR renderTimer;

        // Constructor
        Impl(HWND hwnd, int width, int height)
            : hwnd(hwnd), width(width), height(height),
            objectLoaded(false), mouseDown(false),
            rotationX(0.0f), rotationY(0.0f),
            memDC(NULL), memBitmap(NULL), oldBitmap(NULL) {

            HMENU hMenu = CreateMenu();
            HMENU hFileMenu = CreatePopupMenu();

            // Add menu items
            AppendMenu(hFileMenu, MF_STRING, IDM_FILE_OPEN, L"&Open...");
            AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hFileMenu, MF_STRING, IDM_FILE_EXIT, L"E&xit");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"&File");

            // Set menu to window
            SetMenu(hwnd, hMenu);

            // Get device context
            hdc = GetDC(hwnd);

            // Initialize double buffering
            CreateBackBuffer();

            // Initialize timing
            QueryPerformanceFrequency(&frequency);
            QueryPerformanceCounter(&lastTime);

            renderTimer = SetTimer(hwnd, 1, 16, NULL);
        }

        // Destructor
        ~Impl() {

            if (renderTimer) {
                KillTimer(hwnd, renderTimer);
            }

            // Clean up GDI resources
            if (memDC) {
                SelectObject(memDC, oldBitmap);
                DeleteObject(memBitmap);
                DeleteDC(memDC);
            }

            ReleaseDC(hwnd, hdc);
        }

        void ResetView() {
            rotationX = 0.0f;
            rotationY = 0.0f;
            AdjustViewForObject();
            UpdateTransformation();
            InvalidateRect(hwnd, NULL, TRUE);
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
            if (objectLoaded && object && !object->getVertices().empty()) {
                // Create a copy of the object
                WireframeObject transformedObject = *object;

                // Apply transformations
                transformedObject.transform(transformPipeline.getTransformMatrix());

                //Check if transformed object has valid coordinates
                bool validObject = true;
                for (const auto& vertex : transformedObject.getVertices()) {
                    const auto& pos = vertex.getPosition();
                    if (std::isnan(pos.x) || std::isnan(pos.y) || std::isnan(pos.z) ||
                        !std::isfinite(pos.x) || !std::isfinite(pos.y) || !std::isfinite(pos.z)) {
                        validObject = false;
                        break;
                    }
                }


                // Render the transformed object only if valid coordinates
                if (validObject) {
                    renderer.drawWireframeObject(transformedObject, 3, Color::Blue());
                }
                else {
                    ResetView();
                    return;
                }
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
            DrawText(memDC, TEXT("Left-click and drag to rotate."), -1, &textRect, DT_LEFT);

            // Blit to the window
            BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);
        }

        // Mouse movement handler
        void OnMouseMove(int x, int y) {
            if (mouseDown && objectLoaded) {
                isDragging = true;

                // Calculate mouse movement delta
                int deltaX = x - lastMouseX;
                int deltaY = y - lastMouseY;

                // Update rotation angles
                const float rotationFactor = 0.005f;
                rotationY += deltaX * rotationFactor;
                rotationX += deltaY * rotationFactor;

                // Clamp rotation angles
                const float PI = 3.14159265359f;
                while (rotationX > PI) rotationX -= 2.0f * PI;
                while (rotationX < -PI) rotationX += 2.0f * PI;
                while (rotationY > PI) rotationY -= 2.0f * PI;
                while (rotationY < -PI) rotationY += 2.0f * PI;

                // Update transformation
                UpdateTransformation();

                // Update mouse position
                lastMouseX = x;
                lastMouseY = y;

                // Request redraw
                InvalidateRect(hwnd, NULL, FALSE);
            }
        }

        void AdjustViewForObject() {
            if (!object) return;

            float maxDist = 0.0f;
            const auto& vertices = object->getVertices();

            if (!vertices.empty()) {
                for (const auto& vertex : vertices) {
                    const auto& pos = vertex.getPosition();
                    float dist = std::sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
                    maxDist = std::max(maxDist, dist);
                }

                viewDistance = std::max(3.0f, maxDist * 2.5f);
            }
            else {
                viewDistance = 5.0f;
            }
            UpdateTransformation();
        }

        void UpdateTransformation() {
            transformPipeline.clear();

            // Apply rotations
            transformPipeline.addRotationX(rotationX);
            transformPipeline.addRotationY(rotationY);

            transformPipeline.addTranslation(0.0f, 0.0f, -viewDistance);
        }
    };

    // Static window procedure implementation - redirects to instance method
    LRESULT CALLBACK WindowRenderer::StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        WindowRenderer* instance = reinterpret_cast<WindowRenderer*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        
        // Get the object pointer from window's user data or creation params
        if (instance) {
            // Store the WindowRenderer pointer during creation
            return instance->HandleMessage(hwnd, uMsg, wParam, lParam);
        }
        

        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    // Instance method to handle window messages
    LRESULT WindowRenderer::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        // Defer messages that need pImpl until initialization is complete
        if (!initialized) {
            switch (uMsg) {
            case WM_CLOSE:
            case WM_DESTROY:
            case WM_NCCREATE:
            case WM_CREATE:
                break; // Handle these below
            default:
                return DefWindowProc(hwnd, uMsg, wParam, lParam); // Defer all others
            }
        }
        
        switch (uMsg) {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_SIZE:
            if (initialized && pImpl) {
                Resize(LOWORD(lParam), HIWORD(lParam));
            }
            return 0;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            if (initialized && pImpl) {
                pImpl->RenderFrame();
            }
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_TIMER:
            // Render on timer for smooth animation
            if (initialized && pImpl) {
                pImpl->RenderFrame();
            }
            return 0;

        case WM_COMMAND:
            // Handle menu commands
            switch (LOWORD(wParam)) {
            case IDM_FILE_OPEN:
                if (initialized) {
                    OpenFileDialog();
                }
                return 0;

            case IDM_FILE_EXIT:
                DestroyWindow(hwnd);
                return 0;
            }
            break;

        case WM_LBUTTONDOWN:
            if (initialized && pImpl) {
                pImpl->mouseDown = true;
                pImpl->isDragging = false;
                pImpl->lastMouseX = LOWORD(lParam);
                pImpl->lastMouseY = HIWORD(lParam);
                SetCapture(hwnd);
            }
            return 0;

        case WM_LBUTTONUP:
            if (initialized && pImpl) {
                pImpl->mouseDown = false;
                pImpl->isDragging = false;
                ReleaseCapture();
            }
            return 0;

        case WM_MOUSEMOVE:
            if (initialized && pImpl) {
                pImpl->OnMouseMove(LOWORD(lParam), HIWORD(lParam));
            }
            return 0;

        case WM_KEYDOWN: // Add Reset
            if (wParam == VK_ESCAPE || wParam == 'R') {
                if (initialized && pImpl) {
                    pImpl->ResetView();
                }
            }
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
        //SetProp(GetDesktopWindow(), L"CurrentRenderer", reinterpret_cast<HANDLE>(this));

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

        // Set instance pointer
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

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
        if (initialized && pImpl) {
            pImpl->object = std::move(newObject);
            pImpl->objectLoaded = true;

            // Reset rotation
            pImpl->rotationX = 0.0f;
            pImpl->rotationY = 0.0f;

            pImpl->AdjustViewForObject();

            // Trigger redraw
            InvalidateRect(pImpl->hwnd, NULL, TRUE);
        }
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
        if (!initialized || !pImpl) return;

        OPENFILENAMEA ofn;
        char szFile[260] = { 0 };
        char szInitialDir[MAX_PATH] = { 0 };

        SHGetFolderPathA(NULL, CSIDL_MYDOCUMENTS, NULL, 0, szInitialDir);

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