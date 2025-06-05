#include "pch.h"
#include "win32_window.h"
#include "../../shared/rive_renderer.h"

// Forward declaration for dispatcher queue controller
winrt::Windows::System::DispatcherQueueController CreateDispatcherQueueController();

// CreateDesktopWindowTarget implementation
winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget CreateDesktopWindowTarget(
    winrt::Windows::UI::Composition::Compositor const& compositor, HWND window)
{
    namespace abi = ABI::Windows::UI::Composition::Desktop;

    auto interop = compositor.as<abi::ICompositorDesktopInterop>();
    winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget target{ nullptr };
    winrt::check_hresult(interop->CreateDesktopWindowTarget(window, false, reinterpret_cast<abi::IDesktopWindowTarget**>(winrt::put_abi(target))));
    return target;
}

// Global variables for composition and renderer
std::unique_ptr<RiveRenderer> g_riveRenderer;
winrt::Windows::UI::Composition::Compositor g_compositor{ nullptr };
winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget g_target{ nullptr };
winrt::Windows::UI::Composition::ContainerVisual g_root{ nullptr };

// Custom window class to handle events
class CompositionWindow : public Win32Window {
private:
    winrt::Windows::System::DispatcherQueueController m_controller{ nullptr };

public:
    CompositionWindow() = default;

protected:
    void WindowCreated() override {
        std::cout << "CompositionWindow::WindowCreated() called\n";
        
        try {
            // Create dispatcher queue controller
            m_controller = CreateDispatcherQueueController();
            std::cout << "Created dispatcher queue controller\n";
            
            // Get initial window size
            RECT rect;
            GetClientRect(window_handle_, &rect);
            int windowWidth = rect.right - rect.left;
            int windowHeight = rect.bottom - rect.top;
            std::cout << "Initial window size: " << windowWidth << "x" << windowHeight << "\n";
            
            // Create compositor and target
            g_compositor = winrt::Windows::UI::Composition::Compositor();
            g_target = CreateDesktopWindowTarget(g_compositor, window_handle_);
            
            // Create root container visual
            g_root = g_compositor.CreateContainerVisual();
            g_root.RelativeSizeAdjustment({ 1.0f, 1.0f });
            g_target.Root(g_root);
            
            std::cout << "Composition setup complete\n";
            
            // Create and initialize RiveRenderer
            g_riveRenderer = std::make_unique<RiveRenderer>();
            if (g_riveRenderer->Initialize(g_compositor, windowWidth, windowHeight)) {
                // Get the visual from the renderer and add it to our root
                auto riveVisual = g_riveRenderer->GetVisual();
                if (riveVisual) {
                    g_root.Children().InsertAtTop(riveVisual);
                    
                    // Load a Rive file if available
                    if (g_riveRenderer->LoadRiveFile("C:\\Users\\jeclarke\\Downloads\\meeting_ui.riv")) {
                        std::cout << "Rive file loaded successfully\n";
                    } else {
                        std::cout << "Failed to load Rive file or no file available\n";
                    }
                    
                    // Start the render thread
                    g_riveRenderer->StartRenderThread();
                    
                    std::cout << "RiveRenderer initialized and added to composition tree\n";
                }
            } else {
                std::cout << "Failed to initialize RiveRenderer\n";
            }
        }
        catch (winrt::hresult_error const& ex) {
            std::wcout << L"Failed to initialize composition: " << ex.message().c_str() << L" (0x" << std::hex << ex.code() << L")\n";
        }
    }

    void OnResize(int width, int height) override {
        Win32Window::OnResize(width, height);
        
        if (g_riveRenderer) {
            g_riveRenderer->SetSize(width, height);
        }
    }
    
    void OnPointerDown(int x, int y) override {
        Win32Window::OnPointerDown(x, y);
        // Could forward input to Rive renderer here if needed
    }
    
    void OnDpiChanged(int dpi) override {
        Win32Window::OnDpiChanged(dpi);
        // Handle DPI changes if needed
    }
};

winrt::Windows::System::DispatcherQueueController CreateDispatcherQueueController()
{
    namespace abi = ABI::Windows::System;

    DispatcherQueueOptions options
    {
        sizeof(DispatcherQueueOptions),
        DQTYPE_THREAD_CURRENT,
        DQTAT_COM_STA
    };

    winrt::Windows::System::DispatcherQueueController controller{ nullptr };
    winrt::check_hresult(CreateDispatcherQueueController(options, reinterpret_cast<abi::IDispatcherQueueController**>(winrt::put_abi(controller))));
    return controller;
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE prev_instance, wchar_t* command_line, int show_command) {
    // Enable console output first for debugging
    winrt::init_apartment();
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

    // Create composition window
    auto window = std::make_unique<CompositionWindow>();

    Win32Window::Point origin{ CW_USEDEFAULT, CW_USEDEFAULT };
    Win32Window::Size size{ 800, 600 };

    std::cout << "Creating window...\n";

    if (!window->Create(L"Rive Window - Composition Pattern - Windows.UI.Composition", origin, size)) {
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

    // Cleanup
    if (g_riveRenderer) {
        g_riveRenderer->Shutdown();
        g_riveRenderer.reset();
    }

    return static_cast<int>(msg.wParam);
}
