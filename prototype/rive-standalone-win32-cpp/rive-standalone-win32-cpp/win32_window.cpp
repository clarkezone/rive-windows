#include "pch.h"
#include "win32_window.h"

Win32Window::Win32Window() {
    instance_ = GetModuleHandle(nullptr);
    window_class_name_ = std::wstring(kWindowClassName);  // Initialize here
    std::cout << "Win32Window constructor - instance: " << instance_ << "\n";
    std::wcout << L"Window class name: " << window_class_name_ << L"\n";
}

Win32Window::~Win32Window() {
    Destroy();

    // Only unregister if we registered it
    if (class_registered_) {
        UnregisterWindowClass();
        class_registered_ = false;
    }
}

bool Win32Window::Create(std::wstring_view title, const Point& origin, const Size& size) {
    Destroy();

    // Ensure window_class_name_ is set
    if (window_class_name_.empty()) {
        window_class_name_ = std::wstring(kWindowClassName);
    }

    // Try to register window class
    if (!class_registered_) {
        std::cout << "Attempting to register window class...\n";
        if (!RegisterWindowClass()) {
            std::cout << "Failed to register window class\n";
            return false;
        }
        class_registered_ = true;
    }

    // Initialize DPI before creating window
    current_dpi_ = GetDpiForSystem();

    // Calculate scaled dimensions
    auto scale = GetDpiScale();
    auto scaled_width = static_cast<int>(size.width * scale);
    auto scaled_height = static_cast<int>(size.height * scale);

    std::wcout << L"Creating window with class: " << window_class_name_ << L"\n";
    std::cout << "Instance handle: " << instance_ << "\n";

    window_handle_ = CreateWindowExW(
        0,
        window_class_name_.c_str(),
        title.data(),
        WS_OVERLAPPEDWINDOW,
        origin.x == CW_USEDEFAULT ? CW_USEDEFAULT : origin.x,
        origin.y == CW_USEDEFAULT ? CW_USEDEFAULT : origin.y,
        scaled_width,
        scaled_height,
        nullptr,
        nullptr,
        instance_,
        this
    );

    if (!window_handle_) {
        DWORD error = GetLastError();
        std::cout << "CreateWindowEx failed with error: " << error << "\n";

        // Additional diagnostics
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(WNDCLASSEXW);
        if (!GetClassInfoExW(instance_, window_class_name_.c_str(), &wc)) {
            std::cout << "Window class not found in instance\n";
        }

        return false;
    }

    std::cout << "Window created successfully\n";
    return true;
}

void Win32Window::Show() {
    ShowWindow(window_handle_, SW_SHOW);
    UpdateWindow(window_handle_);
}

float Win32Window::GetDpiScale() const {
    return static_cast<float>(current_dpi_) / 96.0f;
}

void Win32Window::Destroy() {
    if (window_handle_) {
        DestroyWindow(window_handle_);
        window_handle_ = nullptr;
    }
}

bool Win32Window::RegisterWindowClass() {
    window_class_name_ = std::wstring(kWindowClassName);
    std::wcout << L"Registering class with name: " << window_class_name_ << L"\n";

    WNDCLASSEXW window_class{};
    window_class.cbSize = sizeof(WNDCLASSEXW);
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = WndProc;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = instance_;
    window_class.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
    window_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    window_class.lpszMenuName = nullptr;
    window_class.lpszClassName = window_class_name_.c_str();
    window_class.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    ATOM result = RegisterClassExW(&window_class);
    if (!result) {
        DWORD error = GetLastError();
        std::cout << "RegisterClassEx failed with error: " << error << "\n";

        // Check if class already exists (ERROR_CLASS_ALREADY_EXISTS = 1410)
        if (error == ERROR_CLASS_ALREADY_EXISTS) {
            std::cout << "Window class already exists, using existing registration\n";
            return true;
        }

        return false;
    }
    else {
        std::cout << "Window class registered successfully: " << result << "\n";
        return true;
    }
}

void Win32Window::UnregisterWindowClass() {
    if (!window_class_name_.empty()) {
        UnregisterClassW(window_class_name_.c_str(), instance_);
        std::cout << "Window class unregistered\n";
    }
}

LRESULT CALLBACK Win32Window::WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    if (message == WM_NCCREATE) {
        auto create_struct = reinterpret_cast<CREATESTRUCT*>(lparam);
        auto window = static_cast<Win32Window*>(create_struct->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));

        window->window_handle_ = hwnd;
        window->current_dpi_ = GetDpiForWindow(hwnd);
    }
    else if (auto* window = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA))) {
        return window->MessageHandler(hwnd, message, wparam, lparam);
    }

    return DefWindowProc(hwnd, message, wparam, lparam);
}

LRESULT Win32Window::MessageHandler(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
    case WM_DESTROY:
        window_handle_ = nullptr;
        PostQuitMessage(0);
        return 0;

    case WM_LBUTTONDOWN: {
        auto x = GET_X_LPARAM(lparam);
        auto y = GET_Y_LPARAM(lparam);
        OnPointerDown(x, y);
        return 0;
    }

    case WM_DPICHANGED: {
        current_dpi_ = HIWORD(wparam);
        OnDpiChanged(current_dpi_);

        auto* rect = reinterpret_cast<RECT*>(lparam);
        UpdateWindowSize(*rect);
        return 0;
    }

    case WM_SIZE: {
        current_width_ = LOWORD(lparam);
        current_height_ = HIWORD(lparam);
        OnResize(current_width_, current_height_);
        return 0;
    }
    }

    return DefWindowProc(hwnd, message, wparam, lparam);
}

void Win32Window::UpdateWindowSize(const RECT& rect) {
    SetWindowPos(
        window_handle_,
        nullptr,
        rect.left,
        rect.top,
        rect.right - rect.left,
        rect.bottom - rect.top,
        SWP_NOZORDER | SWP_NOACTIVATE
    );
}

void Win32Window::OnPointerDown(int x, int y) {
    // Default implementation - subclasses can override
}

void Win32Window::OnDpiChanged(int dpi) {
    // Default implementation - subclasses can override
}

void Win32Window::OnResize(int width, int height) {
    // Default implementation - subclasses can override
}