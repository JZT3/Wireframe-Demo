//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
//#include <windowsx.h>
//#include <commdlg.h>
//#include <memory>
//#include <string>
//#include <stdexcept>
//#include <algorithm>
//
//#include "vector3d.h"
//#include "vector2d.h"
//#include "matrix4x4.h"
//#include "vertex.h"
//#include "edge.h"
//#include "wireframe.h"
//#include "color.h"
//#include "projection.h"
//#include "transformation.h"
//#include "object_loader.h"
//#include "framebuffer.h"
//
//// Window constants
//constexpr const wchar_t* APP_TITLE = L"3D Wireframe Viewer";
//constexpr const wchar_t* WINDOW_CLASS = L"WireframeViewerClass";
//constexpr int DEFAULT_WIDTH = 800;
//constexpr int DEFAULT_HEIGHT = 600;
//
//// Menu ID constants
//constexpr int IDM_FILE_EXIT = 101;
//constexpr int IDM_FILE_OPEN = 102;
//constexpr int IDM_VIEW_TETRAHEDRON = 103;
//constexpr int IDM_HELP_ABOUT = 104;
//
//// Global application state
//class Application {
//private:
//    HINSTANCE hInstance;
//    HWND hWnd;
//    std::unique_ptr<WireFrameObject> currentObject;
//    std::unique_ptr<ObjectLoader> objectLoader;
//    std::unique_ptr<TransformationPipeline> transformPipeline;
//    std::unique_ptr<FrameBuffer> frameBuffer;
//
//    // Mouse tracking
//    bool isDragging = false;
//    POINT lastMousePos = { 0, 0 };
//
//public:
//    Application(HINSTANCE hInst) :
//        hInstance(hInst),
//        hWnd(nullptr),
//        objectLoader(std::make_unique<ObjectLoader>()),
//        transformPipeline(std::make_unique<TransformationPipeline>()) {
//    }
//
//    ~Application() = default;
//
//    bool initialize() {
//        // Register window class
//        WNDCLASSEX wc = {};
//        wc.cbSize = sizeof(WNDCLASSEX);
//        wc.style = CS_HREDRAW | CS_VREDRAW;
//        wc.lpfnWndProc = WindowProc;
//        wc.hInstance = hInstance;
//        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
//        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
//        wc.lpszClassName = WINDOW_CLASS;
//
//        if (!RegisterClassEx(&wc)) {
//            MessageBox(nullptr, L"Window Registration Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
//            return false;
//        }
//
//        // Create the window
//        hWnd = CreateWindowEx(
//            0,                              // Extended style
//            WINDOW_CLASS,                   // Class name
//            APP_TITLE,                      // Window title
//            WS_OVERLAPPEDWINDOW,            // Window style
//            CW_USEDEFAULT, CW_USEDEFAULT,   // Position
//            DEFAULT_WIDTH, DEFAULT_HEIGHT,  // Size
//            nullptr,                        // Parent window
//            nullptr,                        // Menu
//            hInstance,                      // Instance handle
//            this                            // Additional data
//        );
//
//        if (!hWnd) {
//            MessageBox(nullptr, L"Window Creation Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
//            return false;
//        }
//
//        // Create menu
//        HMENU hMenu = CreateMenu();
//        HMENU hFileMenu = CreatePopupMenu();
//        HMENU hViewMenu = CreatePopupMenu();
//        HMENU hHelpMenu = CreatePopupMenu();
//
//        AppendMenu(hFileMenu, MF_STRING, IDM_FILE_OPEN, L"&Open...");
//        AppendMenu(hFileMenu, MF_SEPARATOR, 0, nullptr);
//        AppendMenu(hFileMenu, MF_STRING, IDM_FILE_EXIT, L"E&xit");
//
//        AppendMenu(hViewMenu, MF_STRING, IDM_VIEW_TETRAHEDRON, L"Load &Tetrahedron");
//
//        AppendMenu(hHelpMenu, MF_STRING, IDM_HELP_ABOUT, L"&About");
//
//        AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"&File");
//        AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hViewMenu, L"&View");
//        AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelpMenu, L"&Help");
//
//        SetMenu(hWnd, hMenu);
//
//        // Create framebuffer
//        HDC hdc = GetDC(hWnd);
//        frameBuffer = std::make_unique<FrameBuffer>(DEFAULT_WIDTH, DEFAULT_HEIGHT);
//        ReleaseDC(hWnd, hdc);
//
//        // Show the window
//        ShowWindow(hWnd, SW_SHOW);
//        UpdateWindow(hWnd);
//
//        // Load the tetrahedron demo
//        loadTetrahedron();
//
//        return true;
//    }
//
//    void run() {
//        MSG msg = {};
//
//        // Main message loop
//        while (GetMessage(&msg, nullptr, 0, 0)) {
//            TranslateMessage(&msg);
//            DispatchMessage(&msg);
//        }
//    }
//
//    void loadTetrahedron() {
//        currentObject = std::make_unique<WireFrameObject>(WireFrameObject::createTetrahedron(1.0f));
//
//        // Reset transformation
//        transformPipeline->clear();
//
//        // Apply initial transformations (move it back a bit)
//        transformPipeline->addTranslation(0.0f, 0.0f, 2.0f);
//
//        // Update the window
//        updateDisplay();
//    }
//
//    void openFile() {
//        OPENFILENAMEA ofn = {};
//        char szFile[260] = { 0 };
//
//        ofn.lStructSize = sizeof(ofn);
//        ofn.hwndOwner = hWnd;
//        ofn.lpstrFile = szFile;
//        ofn.nMaxFile = sizeof(szFile);
//        ofn.lpstrFilter = "Object Files\0*.obj;*.txt\0All Files\0*.*\0";
//        ofn.nFilterIndex = 1;
//        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
//
//        if (GetOpenFileNameA(&ofn)) {
//            try {
//                // Load the object
//                currentObject = std::make_unique<WireFrameObject>(objectLoader->loadObject(ofn.lpstrFile));
//
//                // Reset transformation
//                transformPipeline->clear();
//                transformPipeline->addTranslation(0.0f, 0.0f, 2.0f);
//
//                // Update the window
//                updateDisplay();
//
//                // Update status (optional)
//                std::string status = "Loaded: ";
//                status += ofn.lpstrFile;
//                SetWindowTextA(hWnd, status.c_str());
//            }
//            catch (const std::exception& e) {
//                MessageBoxA(hWnd, e.what(), "Error Loading Object", MB_ICONERROR);
//            }
//        }
//    }
//
//    void updateDisplay() {
//        if (!currentObject || !frameBuffer) {
//            return;
//        }
//
//        // Clear the framebuffer
//        frameBuffer->clear(Color::White());
//
//        // Apply transformation to object
//        WireFrameObject transformedObject = *currentObject;
//        transformedObject.transform(transformPipeline->getTransformMatrix());
//
//        // Draw the wireframe
//        const auto& vertices = transformedObject.getVertices();
//        const auto& edges = transformedObject.getEdges();
//
//        // Draw all edges
//        for (const auto& edge : edges) {
//            if (edge.getVertex1Index() < vertices.size() && edge.getVertex2Index() < vertices.size()) {
//                const auto& v1 = vertices[edge.getVertex1Index()].getPosition();
//                const auto& v2 = vertices[edge.getVertex2Index()].getPosition();
//                drawLine(v1, v2, Color::Blue());
//            }
//        }
//
//        // Draw all vertices
//        for (const auto& vertex : vertices) {
//            drawVertex(vertex.getPosition(), 3, Color::Blue());
//        }
//
//        // Update the window
//        InvalidateRect(hWnd, nullptr, FALSE);
//    }
//
//    void drawVertex(const Vector3D& position, int radius, const Color& color) {
//        // Project 3D point to 2D
//        Vector2D pos2D = perspectiveProject(position);
//
//        // Convert to screen coordinates
//        auto [x, y] = worldToScreen(pos2D);
//
//        // Draw circle
//        for (int dy = -radius; dy <= radius; dy++) {
//            for (int dx = -radius; dx <= radius; dx++) {
//                if (dx * dx + dy * dy <= radius * radius) {
//                    frameBuffer->setPixel(x + dx, y + dy, color);
//                }
//            }
//        }
//    }
//
//    void drawLine(const Vector3D& start, const Vector3D& end, const Color& color) {
//        // Project 3D positions to 2D
//        Vector2D start2D = perspectiveProject(start);
//        Vector2D end2D = perspectiveProject(end);
//
//        // Convert to screen coordinates
//        auto [x0, y0] = worldToScreen(start2D);
//        auto [x1, y1] = worldToScreen(end2D);
//
//        // Bresenham's line algorithm
//        bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
//        if (steep) {
//            std::swap(x0, y0);
//            std::swap(x1, y1);
//        }
//
//        if (x0 > x1) {
//            std::swap(x0, x1);
//            std::swap(y0, y1);
//        }
//
//        int dx = x1 - x0;
//        int dy = std::abs(y1 - y0);
//        int error = dx / 2;
//        int ystep = (y0 < y1) ? 1 : -1;
//        int y = y0;
//
//        for (int x = x0; x <= x1; x++) {
//            if (steep) {
//                frameBuffer->setPixel(y, x, color);
//            }
//            else {
//                frameBuffer->setPixel(x, y, color);
//            }
//
//            error -= dy;
//            if (error < 0) {
//                y += ystep;
//                error += dx;
//            }
//        }
//    }
//
//    std::pair<int, int> worldToScreen(const Vector2D& point) const {
//        int width = frameBuffer->getWidth();
//        int height = frameBuffer->getHeight();
//
//        // Map from [-1,1] range to screen coordinates
//        int x = static_cast<int>((point.x + 1.0f) * width / 2.0f);
//        int y = static_cast<int>((1.0f - point.y) * height / 2.0f);
//
//        return { x, y };
//    }
//
//    void onMouseDown(int x, int y) {
//        isDragging = true;
//        lastMousePos.x = x;
//        lastMousePos.y = y;
//        SetCapture(hWnd);
//    }
//
//    void onMouseUp() {
//        isDragging = false;
//        ReleaseCapture();
//    }
//
//    void onMouseMove(int x, int y) {
//        if (isDragging && currentObject) {
//            // Calculate mouse movement delta
//            int deltaX = x - lastMousePos.x;
//            int deltaY = y - lastMousePos.y;
//
//            // Apply rotation based on mouse movement
//            const float rotationFactor = 0.01f;
//            transformPipeline->addRotationY(deltaX * rotationFactor);
//            transformPipeline->addRotationX(deltaY * rotationFactor);
//
//            // Update mouse position
//            lastMousePos.x = x;
//            lastMousePos.y = y;
//
//            // Update the display
//            updateDisplay();
//        }
//    }
//
//    void onResize(int width, int height) {
//        if (width > 0 && height > 0) {
//            // Recreate the framebuffer
//            frameBuffer = std::make_unique<FrameBuffer>(width, height);
//
//            // Update the display
//            updateDisplay();
//        }
//    }
//
//    // Static window procedure
//    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
//        Application* app = nullptr;
//
//        if (uMsg == WM_CREATE) {
//            // Store the application pointer with the window
//            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
//            app = reinterpret_cast<Application*>(pCreate->lpCreateParams);
//            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
//        }
//        else {
//            // Retrieve the application pointer
//            app = reinterpret_cast<Application*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
//        }
//
//        // Process messages
//        if (app) {
//            switch (uMsg) {
//            case WM_COMMAND:
//                switch (LOWORD(wParam)) {
//                case IDM_FILE_EXIT:
//                    DestroyWindow(hwnd);
//                    return 0;
//
//                case IDM_FILE_OPEN:
//                    app->openFile();
//                    return 0;
//
//                case IDM_VIEW_TETRAHEDRON:
//                    app->loadTetrahedron();
//                    return 0;
//
//                case IDM_HELP_ABOUT:
//                    MessageBox(hwnd, L"3D Wireframe Viewer\nCreated with Modern C++", L"About", MB_OK);
//                    return 0;
//                }
//                break;
//
//            case WM_PAINT:
//            {
//                PAINTSTRUCT ps;
//                HDC hdc = BeginPaint(hwnd, &ps);
//
//                // Draw the framebuffer to the window
//                if (app->frameBuffer) {
//                    app->frameBuffer->saveToPPM("test.ppm"); // For testing
//
//                    // TODO: Implement display function
//                    // For now, just clear with white
//                    RECT rc;
//                    GetClientRect(hwnd, &rc);
//                    FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
//                }
//
//                EndPaint(hwnd, &ps);
//                return 0;
//            }
//
//            case WM_SIZE:
//                app->onResize(LOWORD(lParam), HIWORD(lParam));
//                return 0;
//
//            case WM_LBUTTONDOWN:
//                app->onMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
//                return 0;
//
//            case WM_LBUTTONUP:
//                app->onMouseUp();
//                return 0;
//
//            case WM_MOUSEMOVE:
//                app->onMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
//                return 0;
//
//            case WM_DESTROY:
//                PostQuitMessage(0);
//                return 0;
//            }
//        }
//
//        return DefWindowProc(hwnd, uMsg, wParam, lParam);
//    }
//};
//
//// Application entry point
//int CALLBACK WINAPI WinMain(
//    _In_ HINSTANCE hInstance,
//    _In_opt_ HINSTANCE hPrevInstance,
//    _In_ LPSTR lpCmdLine,
//    _In_ int nCmdShow)
//{
//    UNREFERENCED_PARAMETER(hPrevInstance);
//    UNREFERENCED_PARAMETER(lpCmdLine);
//    UNREFERENCED_PARAMETER(nCmdShow);
//
//    try {
//        // Create and initialize application
//        Application app(hInstance);
//
//        if (app.initialize()) {
//            // Main message loop
//            app.run();
//        }
//    }
//    catch (const std::exception& e) {
//        MessageBoxA(nullptr, e.what(), "Unhandled Exception", MB_ICONERROR);
//        return 1;
//    }
//
//    return 0;
//}