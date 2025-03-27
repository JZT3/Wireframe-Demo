#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <stdexcept>
#include <cmath>

#include "vector3d.h"
#include "vector2d.h"
#include "matrix4x4.h"
#include "color.h"
#include "vertex.h"
#include "edge.h"
#include "wireframe.h"
#include "projection.h"
#include "transformation.h"
#include "object_loader.h"

// Window dimensions
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;

// Application constants
constexpr const wchar_t* WINDOW_CLASS_NAME = L"WireframeViewerClass";
constexpr const wchar_t* WINDOW_TITLE = L"3D Wireframe Viewer";

// Menu IDs
constexpr int IDM_FILE_EXIT = 1;
constexpr int IDM_FILE_OPEN = 2;
constexpr int IDM_VIEW_TETRAHEDRON = 3;
constexpr int IDM_HELP_ABOUT = 4;

// Global state
HINSTANCE g_hInstance = nullptr;
HWND g_hWnd = nullptr;
bool g_isDragging = false;
POINT g_lastMousePos = { 0, 0 };

// Our 3D application state
std::unique_ptr<WireFrameObject> g_object;
TransformationPipeline g_transform;

// Simple framebuffer for our rendering
class FrameBuffer {
private:
    int width, height;
    std::vector<COLORREF> pixels;
    HBITMAP hBitmap = nullptr;
    HDC hdcMem = nullptr;

public:
    FrameBuffer(int width, int height) : width(width), height(height) {
        pixels.resize(static_cast<size_t>(width) * static_cast<size_t>(height), RGB(0, 0, 0));
    }

    ~FrameBuffer() {
        if (hdcMem) {
            DeleteDC(hdcMem);
        }
        if (hBitmap) {
            DeleteObject(hBitmap);
        }
    }

    void clear(COLORREF color = RGB(0, 0, 0)) {
        std::fill(pixels.begin(), pixels.end(), color);
    }

    void setPixel(int x, int y, COLORREF color) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            pixels[static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)] = color;
        }
    }

    bool initialize(HDC hdc) {
        // Create compatible DC if needed
        if (!hdcMem) {
            hdcMem = CreateCompatibleDC(hdc);
            if (!hdcMem) {
                return false;
            }

            // Create DIB section for our bitmap
            BITMAPINFO bmi;
            ZeroMemory(&bmi, sizeof(BITMAPINFO));
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = width;
            bmi.bmiHeader.biHeight = -height; // Top-down DIB
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;

            void* pBits = nullptr;
            hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);

            if (!hBitmap) {
                DeleteDC(hdcMem);
                hdcMem = nullptr;
                return false;
            }

            SelectObject(hdcMem, hBitmap);
        }
        return true;
    }

    void draw(HDC hdc) {
        // Initialize if not already done
        if (!hdcMem && !initialize(hdc)) {
            // Can't render, so fill with a solid color instead
            RECT clientRect;
            GetClientRect(WindowFromDC(hdc), &clientRect);
            FillRect(hdc, &clientRect, (HBRUSH)GetStockObject(BLACK_BRUSH));
            return;
        }

        // Copy pixel data to bitmap
        SetBitmapBits(hBitmap, static_cast<DWORD>(pixels.size() * sizeof(COLORREF)), pixels.data());
        BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);
        // Blit to the destination DC
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }
};

// Global framebuffer
std::unique_ptr<FrameBuffer> g_framebuffer;

// Drawing functions
static void drawLine(int x0, int y0, int x1, int y1, COLORREF color) {
    // Bresenham's line algorithm
    const bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }

    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    const int dx = x1 - x0;
    const int dy = std::abs(y1 - y0);
    const int yStep = (y0 < y1) ? 1 : -1;

    int error = dx / 2;
    int y = y0;

    for (int x = x0; x <= x1; ++x) {
        if (steep) {
            g_framebuffer->setPixel(y, x, color);
        }
        else {
            g_framebuffer->setPixel(x, y, color);
        }

        error -= dy;
        if (error < 0) {
            y += yStep;
            error += dx;
        }
    }
}

static void drawCircle(int centerX, int centerY, int radius, COLORREF color) {
    // Simple filled circle algorithm
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            if (dx * dx + dy * dy <= radius * radius) {
                g_framebuffer->setPixel(centerX + dx, centerY + dy, color);
            }
        }
    }
}

// Convert from 3D world coordinates to screen coordinates
static std::pair<int, int> worldToScreen(const Vector2D& point) {
    const int width = g_framebuffer->getWidth();
    const int height = g_framebuffer->getHeight();

    // Map from [-1,1] range to screen coordinates
    const int screenX = static_cast<int>((point.x + 1.0f) * width / 2.0f);
    const int screenY = static_cast<int>((1.0f - point.y) * height / 2.0f);

    return { screenX, screenY };
}

// Render the 3D wireframe object
static void renderWireframe() {
    if (!g_object || !g_framebuffer) {
        return;
    }

    // Clear framebuffer
    g_framebuffer->clear(RGB(0, 0, 0)); // Black background

    // Create a copy of the object
    WireFrameObject transformedObject = *g_object;

    // Apply transformations
    transformedObject.transform(g_transform.getTransformMatrix());

    // Draw all edges as blue lines
    const auto& vertices = transformedObject.getVertices();
    const auto& edges = transformedObject.getEdges();

    for (const auto& edge : edges) {
        if (edge.getVertex1Index() < vertices.size() && edge.getVertex2Index() < vertices.size()) {
            const auto& pos1 = vertices[edge.getVertex1Index()].getPosition();
            const auto& pos2 = vertices[edge.getVertex2Index()].getPosition();

            // Project 3D positions to 2D
            Vector2D pos1_2D = perspectiveProject(pos1);
            Vector2D pos2_2D = perspectiveProject(pos2);

            // Convert to screen coordinates
            auto [x1, y1] = worldToScreen(pos1_2D);
            auto [x2, y2] = worldToScreen(pos2_2D);

            // Draw the edge
            drawLine(x1, y1, x2, y2, RGB(255, 0, 0)); // Blue line
        }
    }

    // Draw all vertices as small filled circles
    for (const auto& vertex : vertices) {
        const auto& pos = vertex.getPosition();

        // Project 3D position to 2D
        Vector2D pos_2D = perspectiveProject(pos);

        // Convert to screen coordinates
        auto [x, y] = worldToScreen(pos_2D);

        // Draw the vertex 
        drawCircle(x, y, 3, RGB(255, 0, 0)); // Blue circle
    }
}

// Update the window
static void updateWindow() {
    if (!g_hWnd) {
        return;
    }

    // Render the wireframe
    renderWireframe();

    // Update the window
    InvalidateRect(g_hWnd, nullptr, FALSE);
}

// Load a demo tetrahedron
static void loadTetrahedron() {
    g_object = std::make_unique<WireFrameObject>(WireFrameObject::createTetrahedron(1.0f));

    // Reset transformation
    g_transform.clear();

    // Move it back a bit so it's visible with perspective projection
    g_transform.addTranslation(0.0f, 0.0f, 5.0f);

    // Update the window
    updateWindow();
}

// Forward declarations of window procedures
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Create main window
static bool createMainWindow() {
    // Register window class
    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = g_hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = WINDOW_CLASS_NAME;

    if (!RegisterClassEx(&wc)) {
        MessageBox(nullptr, L"Window Registration Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    // Create the window
    g_hWnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        WINDOW_CLASS_NAME,
        WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
        nullptr, nullptr, g_hInstance, nullptr
    );

    if (!g_hWnd) {
        MessageBox(nullptr, L"Window Creation Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    // Create menu
    HMENU hMenu = CreateMenu();
    HMENU hFileMenu = CreatePopupMenu();
    HMENU hViewMenu = CreatePopupMenu();
    HMENU hHelpMenu = CreatePopupMenu();

    AppendMenu(hFileMenu, MF_STRING, IDM_FILE_OPEN, L"&Open...");
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(hFileMenu, MF_STRING, IDM_FILE_EXIT, L"E&xit");

    AppendMenu(hViewMenu, MF_STRING, IDM_VIEW_TETRAHEDRON, L"Load &Tetrahedron");

    AppendMenu(hHelpMenu, MF_STRING, IDM_HELP_ABOUT, L"&About");

    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"&File");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hViewMenu, L"&View");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelpMenu, L"&Help");

    SetMenu(g_hWnd, hMenu);

    // Create framebuffer
    g_framebuffer = std::make_unique<FrameBuffer>(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Show the window
    ShowWindow(g_hWnd, SW_SHOWNORMAL);
    UpdateWindow(g_hWnd);

    return true;
}

// Open file dialog to load a 3D object
static void openFileDialog() {
    OPENFILENAMEA ofn;
    char szFile[260] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g_hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Object Files\0*.obj;*.txt\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn)) {
        try {
            ObjectLoader loader;
            g_object = std::make_unique<WireFrameObject>(loader.loadObject(ofn.lpstrFile));

            // Reset transformation
            g_transform.clear();

            // Move it back a bit so it's visible with perspective projection
            g_transform.addTranslation(0.0f, 0.0f, 5.0f);

            // Update the window
            updateWindow();
        }
        catch (const std::exception& e) {
            MessageBoxA(g_hWnd, e.what(), "Error", MB_ICONERROR);
        }
    }
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_FILE_EXIT:
            DestroyWindow(hwnd);
            return 0;

        case IDM_FILE_OPEN:
            openFileDialog();
            return 0;

        case IDM_VIEW_TETRAHEDRON:
            loadTetrahedron();
            return 0;

        case IDM_HELP_ABOUT:
            MessageBox(hwnd, L"3D Wireframe Viewer\nCreated from scratch", L"About", MB_OK);
            return 0;
        }
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Draw the framebuffer to the window
        if (g_framebuffer) {
            g_framebuffer->draw(hdc);
        }

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_SIZE:
    {
        // Resize the framebuffer when the window is resized
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        if (width > 0 && height > 0) {
            g_framebuffer = std::make_unique<FrameBuffer>(width, height);
            updateWindow();
        }
        return 0;
    }

    case WM_LBUTTONDOWN:
    {
        // Start dragging
        g_isDragging = true;
        g_lastMousePos.x = GET_X_LPARAM(lParam);
        g_lastMousePos.y = GET_Y_LPARAM(lParam);
        SetCapture(hwnd);
        return 0;
    }

    case WM_LBUTTONUP:
    {
        // Stop dragging
        g_isDragging = false;
        ReleaseCapture();
        return 0;
    }

    case WM_MOUSEMOVE:
    {
        // Handle rotation when dragging
        if (g_isDragging && g_object) {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            // Calculate mouse movement delta
            int deltaX = x - g_lastMousePos.x;
            int deltaY = y - g_lastMousePos.y;

            // Apply rotation based on mouse movement
            const float rotationFactor = 0.01f;
            g_transform.addRotationY(deltaX * rotationFactor);
            g_transform.addRotationX(deltaY * rotationFactor);

            // Update mouse position
            g_lastMousePos.x = x;
            g_lastMousePos.y = y;

            // Update the window
            updateWindow();
        }
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Application entry point
int CALLBACK WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    // Store the instance handle
    g_hInstance = hInstance;

    // Create the main window
    if (!createMainWindow()) {
        return 1;
    }

    // Load the default tetrahedron
    loadTetrahedron();

    // Main message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}