// Win32RiveExample.cpp - Example of using WinRive WinRT component in a Win32 application
// This demonstrates the hybrid hosting architecture that enables the same WinRT component
// to work in UWP XAML, WinUI3 XAML, and Win32 scenarios.

#include <windows.h>
#include <winrt/base.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <windows.ui.composition.interop.h>
#include <DispatcherQueue.h>
#include <winrt/WinRive.h>
#include <iostream>

// Forward declarations
winrt::Windows::System::DispatcherQueueController CreateDispatcherQueueController();
winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget CreateDesktopWindowTarget(
    winrt::Windows::UI::Composition::Compositor const& compositor, HWND window);

// Global variables
HWND g_hwnd = nullptr;
winrt::Windows::UI::Composition::Compositor g_compositor{ nullptr };
winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget g_target{ nullptr };
winrt::Windows::UI::Composition::ContainerVisual g_root{ nullptr };
winrt::WinRive::RiveControl g_riveControl{ nullptr };
winrt::Windows::System::DispatcherQueueController g_controller{ nullptr };

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        {
            try
            {
                std::wcout << L"Creating composition infrastructure...\n";
                
                // Create dispatcher queue controller for composition
                g_controller = CreateDispatcherQueueController();
                
                // Create compositor and desktop window target
                g_compositor = winrt::Windows::UI::Composition::Compositor();
                g_target = CreateDesktopWindowTarget(g_compositor, hwnd);
                
                // Create root container visual
                g_root = g_compositor.CreateContainerVisual();
                g_root.RelativeSizeAdjustment({ 1.0f, 1.0f });
                g_target.Root(g_root);
                
                // Get initial window size
                RECT rect;
                GetClientRect(hwnd, &rect);
                int width = rect.right - rect.left;
                int height = rect.bottom - rect.top;
                
                std::wcout << L"Creating WinRive control for Win32 hosting...\n";
                
                // Create WinRive control using Win32 hosting mode
                g_riveControl = winrt::WinRive::RiveControl();
                
                // Initialize for Win32 hosting - this uses the new hybrid architecture
                if (g_riveControl.InitializeForWin32(g_compositor, reinterpret_cast<uint64_t>(hwnd), width, height))
                {
                    // Get the visual from the Rive control and add to composition tree
                    auto riveVisual = g_riveControl.GetVisual();
                    if (riveVisual)
                    {
                        g_root.Children().InsertAtTop(riveVisual);
                        std::wcout << L"Rive visual added to composition tree\n";
                        
                        // Load a Rive file if available (update path as needed)
                        if (g_riveControl.LoadRiveFile(L"C:\\Users\\jeclarke\\Downloads\\meeting_ui.riv"))
                        {
                            std::wcout << L"Rive file loaded successfully\n";
                            g_riveControl.StartRenderLoop();
                        }
                        else
                        {
                            std::wcout << L"No Rive file found or failed to load\n";
                        }
                    }
                }
                else
                {
                    std::wcout << L"Failed to initialize WinRive control for Win32\n";
                }
            }
            catch (winrt::hresult_error const& ex)
            {
                std::wcout << L"Error during creation: " << ex.message().c_str() << L"\n";
            }
        }
        break;
        
    case WM_SIZE:
        {
            if (g_riveControl)
            {
                RECT rect;
                GetClientRect(hwnd, &rect);
                int width = rect.right - rect.left;
                int height = rect.bottom - rect.top;
                g_riveControl.SetSize(width, height);
            }
        }
        break;
        
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        {
            // Forward mouse messages to the Win32 input provider
            // This demonstrates how the abstracted input system works with Win32
            // The WinRive control's Win32InputProvider will handle these messages
            // and forward them to the Rive renderer through the unified interface
        }
        break;
        
    case WM_DESTROY:
        {
            // Cleanup
            if (g_riveControl)
            {
                g_riveControl.Shutdown();
                g_riveControl = nullptr;
            }
            
            g_target = nullptr;
            g_root = nullptr;
            g_compositor = nullptr;
            g_controller = nullptr;
            
            PostQuitMessage(0);
        }
        break;
        
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    
    return 0;
}

// Helper function to create DesktopWindowTarget
winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget CreateDesktopWindowTarget(
    winrt::Windows::UI::Composition::Compositor const& compositor, HWND window)
{
    namespace abi = ABI::Windows::UI::Composition::Desktop;
    auto interop = compositor.as<abi::ICompositorDesktopInterop>();
    winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget target{ nullptr };
    winrt::check_hresult(interop->CreateDesktopWindowTarget(window, false, 
        reinterpret_cast<abi::IDesktopWindowTarget**>(winrt::put_abi(target))));
    return target;
}

// Helper function to create DispatcherQueueController
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
    winrt::check_hresult(CreateDispatcherQueueController(options, 
        reinterpret_cast<abi::IDispatcherQueueController**>(winrt::put_abi(controller))));
    return controller;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    // Initialize WinRT
    winrt::init_apartment();
    
    // Enable console for debugging
    if (AllocConsole())
    {
        FILE* pCout;
        freopen_s(&pCout, "CONOUT$", "w", stdout);
        std::wcout.sync_with_stdio(true);
        std::wcout << L"Win32 WinRive Example - Hybrid Hosting Architecture\n";
        std::wcout << L"This demonstrates the enhanced WinRT component working in Win32\n\n";
    }
    
    // Enable high DPI support
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    
    // Register window class
    const wchar_t CLASS_NAME[] = L"Win32RiveWindow";
    
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    
    RegisterClass(&wc);
    
    // Create window
    g_hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"WinRive Win32 Example - Hybrid Hosting",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr, nullptr, hInstance, nullptr
    );
    
    if (g_hwnd == nullptr)
    {
        std::wcout << L"Failed to create window\n";
        return 0;
    }
    
    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);
    
    std::wcout << L"Window created and shown\n";
    std::wcout << L"The same WinRive WinRT component now supports:\n";
    std::wcout << L"- UWP XAML (via InitializeForUWP)\n";
    std::wcout << L"- WinUI3 XAML (via InitializeForWinUI3)\n";
    std::wcout << L"- Win32 (via InitializeForWin32) - as demonstrated here\n\n";
    
    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return 0;
}
