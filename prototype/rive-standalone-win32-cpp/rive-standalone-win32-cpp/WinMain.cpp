#include "pch.h"
#include "app_window.h"

void EnableHighDPISupport() {
    // Enable per-monitor DPI awareness
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE prev_instance, wchar_t* command_line, int show_command) {
    // Enable console output first for debugging
    if (AllocConsole()) {
        FILE* stdout_file;
        freopen_s(&stdout_file, "CONOUT$", "w", stdout);
        std::cout.sync_with_stdio();
        std::cout << "Console initialized\n";
    }

    // Enable high DPI support
    HRESULT hr = SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    if (FAILED(hr)) {
        std::cout << "Failed to set DPI awareness, trying fallback\n";
        SetProcessDPIAware();
    }

    // Create application window
    auto window = std::make_unique<AppWindow>();

    Win32Window::Point origin{ CW_USEDEFAULT, CW_USEDEFAULT };
    Win32Window::Size size{ 800, 600 };

    std::cout << "Creating window...\n";

    if (!window->Create(L"C++20 Win32 Application", origin, size)) {
        std::cout << "Failed to create window\n";
        system("pause");
        return -1;
    }

    std::cout << "Window created successfully\n";
    window->Show();

    std::cout << "Window created with initial DPI: "
        << static_cast<int>(window->GetDpiScale() * 96) << "\n";

    // Message loop
    MSG msg{};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}