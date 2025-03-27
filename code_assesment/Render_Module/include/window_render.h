#pragma once
#include <Windows.h>
#include <string>
#include <memory>
#include <functional>
#include "vector3D.h"
#include "matrix4x4.h"
#include "wireframe.h"
#include "transformation.h"
#include "object_loader.h"
#include "color.h"
#include "projection.h"

// Forward declaration
class WindowRenderer;

// Global pointer to renderer for window procedure
WindowRenderer* g_pCurrentRenderer = nullptr;

class WindowRenderer {
private:
    // Window handles and dimensions
    HWND m_hwnd;
    HDC m_hdc;
    int m_width;
    int m_height;


    // Double buffering
    HDC m_memDC;
    HBITMAP m_memBitmap;
    HBITMAP m_oldBitmap;

    // Current 3D object and transformations
    WireFrameObject m_object;
    TransformationPipeline m_transformPipeline;
    bool m_objectLoaded = false;

    // Mouse interaction state
    bool m_mouseDown = false;
    int m_lastMouseX = 0;
    int m_lastMouseY = 0;
    float m_rotationX = 0.0f;
    float m_rotationY = 0.0f;

    // Frame timing
    LARGE_INTEGER m_lastTime;
    LARGE_INTEGER m_frequency;

    // Window procedure
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if (g_pCurrentRenderer != nullptr) {
            switch (uMsg) {
            case WM_CLOSE:
                DestroyWindow(hwnd);
                return 0;

            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;

            case WM_SIZE:
                g_pCurrentRenderer->Resize(LOWORD(lParam), HIWORD(lParam));
                return 0;

            case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);

                // Draw the framebuffer to the window
                g_pCurrentRenderer->Render();

                EndPaint(hwnd, &ps);
            }
            return 0;

            case WM_LBUTTONDOWN:
                g_pCurrentRenderer->OnMouseDown(LOWORD(lParam), HIWORD(lParam));
                return 0;

            case WM_LBUTTONUP:
                g_pCurrentRenderer->OnMouseUp(LOWORD(lParam), HIWORD(lParam));
                return 0;

            case WM_MOUSEMOVE:
                g_pCurrentRenderer->OnMouseMove(LOWORD(lParam), HIWORD(lParam));
                return 0;

            case WM_KEYDOWN:
                g_pCurrentRenderer->OnKeyDown(wParam);
                return 0;
            }
        }

        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    // Convert world coordinates to screen coordinates
    std::pair<int, int> WorldToScreen(const Vector2D& point) const {
        return {
            static_cast<int>(point.x * m_width / 2 + m_width / 2),
            static_cast<int>(m_height / 2 - point.y * m_height / 2)
        };
    }

    // Create double-buffered drawing context
    void CreateBackBuffer() {
        if (m_memDC != NULL) {
            SelectObject(m_memDC, m_oldBitmap);
            DeleteObject(m_memBitmap);
            DeleteDC(m_memDC);
        }

        m_memDC = CreateCompatibleDC(m_hdc);
        m_memBitmap = CreateCompatibleBitmap(m_hdc, m_width, m_height);
        m_oldBitmap = (HBITMAP)SelectObject(m_memDC, m_memBitmap);
    }

public:
    WindowRenderer(HWND hwnd, int width, int height)
        : m_hwnd(hwnd), m_width(width), m_height(height) {
        // Store the device context
        m_hdc = GetDC(hwnd);

        // Create double buffer
        m_memDC = NULL;
        CreateBackBuffer();

        // Initialize timing
        QueryPerformanceFrequency(&m_frequency);
        QueryPerformanceCounter(&m_lastTime);

        // Set global pointer for window procedure
        g_pCurrentRenderer = this;
    }

    ~WindowRenderer() {
        // Clean up GDI resources
        SelectObject(m_memDC, m_oldBitmap);
        DeleteObject(m_memBitmap);
        DeleteDC(m_memDC);
        ReleaseDC(m_hwnd, m_hdc);

        // Reset global pointer
        if (g_pCurrentRenderer == this) {
            g_pCurrentRenderer = nullptr;
        }
    }

    void Resize(int width, int height) {
        m_width = width;
        m_height = height;
        CreateBackBuffer();
        InvalidateRect(m_hwnd, NULL, TRUE);
    }

    // Load a 3D object
    bool LoadObject(const WireFrameObject& object) {
        m_object = object;
        m_objectLoaded = true;
        m_rotationX = 0.0f;
        m_rotationY = 0.0f;
        m_transformPipeline.clear();
        InvalidateRect(m_hwnd, NULL, TRUE);
        return true;
    }

    // Load built-in tetrahedron
    void LoadTetrahedron() {
        m_object = WireFrameObject::createTetrahedron(0.8f);
        m_objectLoaded = true;
        m_rotationX = 0.0f;
        m_rotationY = 0.0f;
        m_transformPipeline.clear();
        InvalidateRect(m_hwnd, NULL, TRUE);
    }

    // Mouse event handlers
    void OnMouseDown(int x, int y) {
        m_mouseDown = true;
        m_lastMouseX = x;
        m_lastMouseY = y;
    }

    void OnMouseUp(int x, int y) {
        m_mouseDown = false;
    }

    void OnMouseMove(int x, int y) {
        if (m_mouseDown) {
            // Calculate mouse movement
            int deltaX = x - m_lastMouseX;
            int deltaY = y - m_lastMouseY;

            // Update rotation angles
            const float rotationFactor = 0.01f;
            m_rotationY += deltaX * rotationFactor;
            m_rotationX += deltaY * rotationFactor;

            // Update transformation
            m_transformPipeline.clear();
            m_transformPipeline.addRotationX(m_rotationX);
            m_transformPipeline.addRotationY(m_rotationY);

            // Update mouse position
            m_lastMouseX = x;
            m_lastMouseY = y;

            // Request redraw
            InvalidateRect(m_hwnd, NULL, TRUE);
        }
    }

    // Keyboard event handler
    void OnKeyDown(WPARAM key) {
        if (key == VK_SPACE) {
            // Reset view
            m_rotationX = 0.0f;
            m_rotationY = 0.0f;
            m_transformPipeline.clear();
            InvalidateRect(m_hwnd, NULL, TRUE);
        }
    }

    // Drawing primitives
    void Clear(COLORREF color) const {
        HBRUSH brush = CreateSolidBrush(color);
        RECT rect = { 0, 0, m_width, m_height };
        FillRect(m_memDC, &rect, brush);
        DeleteObject(brush);
    }

    void DrawPixel(int x, int y, COLORREF color) const {
        if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
            SetPixel(m_memDC, x, y, color);
        }
    }

    void DrawCircle(int x, int y, int radius, COLORREF color) const {
        // Fill circle
        for (int cy = -radius; cy <= radius; cy++) {
            for (int cx = -radius; cx <= radius; cx++) {
                if (cx * cx + cy * cy <= radius * radius) {
                    DrawPixel(x + cx, y + cy, color);
                }
            }
        }
    }

    void DrawLine(int x1, int y1, int x2, int y2, COLORREF color) const {
        // Bresenham's line algorithm
        const bool steep = std::abs(y2 - y1) > std::abs(x2 - x1);
        if (steep) {
            std::swap(x1, y1);
            std::swap(x2, y2);
        }

        if (x1 > x2) {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }

        const int dx = x2 - x1;
        const int dy = std::abs(y2 - y1);
        const int yStep = (y1 < y2) ? 1 : -1;

        int error = dx / 2;
        int y = y1;

        for (int x = x1; x <= x2; ++x) {
            if (steep) {
                DrawPixel(y, x, color);
            }
            else {
                DrawPixel(x, y, color);
            }

            error -= dy;
            if (error < 0) {
                y += yStep;
                error += dx;
            }
        }
    }

    // Draw a 3D vertex
    void DrawVertex(const Vector3D& position, int radius, COLORREF color) {
        // Project to 2D
        Vector2D pos2D = orthographicProject(position);

        // Convert to screen coordinates
        auto [screenX, screenY] = WorldToScreen(pos2D);

        // Draw circle
        DrawCircle(screenX, screenY, radius, color);
    }

    // Draw a 3D edge
    void DrawEdge(const Vector3D& start, const Vector3D& end, COLORREF color) {
        // Project to 2D
        Vector2D start2D = orthographicProject(start);
        Vector2D end2D = orthographicProject(end);

        // Convert to screen coordinates
        auto [startX, startY] = WorldToScreen(start2D);
        auto [endX, endY] = WorldToScreen(end2D);

        // Draw line
        DrawLine(startX, startY, endX, endY, color);
    }

    // Main render function
    void Render() {
        // Calculate delta time
        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);
        float deltaTime = static_cast<float>(currentTime.QuadPart - m_lastTime.QuadPart) / m_frequency.QuadPart;
        m_lastTime = currentTime;

        // Clear background
        Clear(RGB(0, 0, 0));

        // Draw wireframe if loaded
        if (m_objectLoaded) {
            // Create a copy of the object and apply transformations
            WireFrameObject transformedObject = m_object;
            transformedObject.transform(m_transformPipeline.getTransformMatrix());

            const auto& vertices = transformedObject.getVertices();
            const auto& edges = transformedObject.getEdges();

            // Draw edges
            for (const auto& edge : edges) {
                if (edge.getVertex1Index() < vertices.size() && edge.getVertex2Index() < vertices.size()) {
                    const auto& v1 = vertices[edge.getVertex1Index()].getPosition();
                    const auto& v2 = vertices[edge.getVertex2Index()].getPosition();
                    DrawEdge(v1, v2, RGB(0, 0, 255));
                }
            }

            // Draw vertices
            for (const auto& vertex : vertices) {
                DrawVertex(vertex.getPosition(), 3, RGB(0, 0, 255));
            }
        }

        // Draw UI text
        SetTextColor(m_memDC, RGB(255, 255, 255));
        SetBkMode(m_memDC, TRANSPARENT);
        RECT textRect = { 10, 10, m_width - 10, 30 };
        DrawText(m_memDC, TEXT("Click and drag to rotate, SPACE to reset view"), -1, &textRect, DT_LEFT);

        // Blit to window
        BitBlt(m_hdc, 0, 0, m_width, m_height, m_memDC, 0, 0, SRCCOPY);
    }

    // Create the window and start main loop
    static int Run(int width, int height, const std::string& title) {
        // Register the window class
        const wchar_t CLASS_NAME[] = L"WireframeRendererClass";

        WNDCLASS wc = { 0 };
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = CLASS_NAME;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

        RegisterClass(&wc);

        // Convert title to wide string
        std::wstring wideTitle(title.begin(), title.end());

        // Create the window
        HWND hwnd = CreateWindowEx(
            0,                      // Optional window styles
            CLASS_NAME,             // Window class
            wideTitle.c_str(),      // Window text
            WS_OVERLAPPEDWINDOW,    // Window style

            // Size and position
            CW_USEDEFAULT, CW_USEDEFAULT, width, height,

            NULL,       // Parent window    
            NULL,       // Menu
            GetModuleHandle(NULL),  // Instance handle
            NULL        // Additional application data
        );

        if (hwnd == NULL) {
            return 0;
        }

        // Main message loop
        MSG msg = { 0 };
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        return static_cast<int>(msg.wParam);
    }
};