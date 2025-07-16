#include "pch.h"

extern "C" IMAGE_DOS_HEADER __ImageBase;

using namespace winrt;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Composition::Desktop;

auto CreateDispatcherQueueController()
{
    namespace abi = ABI::Windows::System;

    DispatcherQueueOptions options
    {
        sizeof(DispatcherQueueOptions),
        DQTYPE_THREAD_CURRENT,
        DQTAT_COM_STA
    };

    Windows::System::DispatcherQueueController controller{ nullptr };
    check_hresult(CreateDispatcherQueueController(options, reinterpret_cast<abi::IDispatcherQueueController**>(put_abi(controller))));
    return controller;
}

DesktopWindowTarget CreateDesktopWindowTarget(Compositor const& compositor, HWND window)
{
    namespace abi = ABI::Windows::UI::Composition::Desktop;

    auto interop = compositor.as<abi::ICompositorDesktopInterop>();
    DesktopWindowTarget target{ nullptr };
    check_hresult(interop->CreateDesktopWindowTarget(window, true, reinterpret_cast<abi::IDesktopWindowTarget**>(put_abi(target))));
    return target;
}

template <typename T>
struct DesktopWindow
{
    static T* GetThisFromHandle(HWND const window) noexcept
    {
        return reinterpret_cast<T *>(GetWindowLongPtr(window, GWLP_USERDATA));
    }

    static LRESULT __stdcall WndProc(HWND const window, UINT const message, WPARAM const wparam, LPARAM const lparam) noexcept
    {
        WINRT_ASSERT(window);

        if (WM_NCCREATE == message)
        {
            auto cs = reinterpret_cast<CREATESTRUCT *>(lparam);
            T* that = static_cast<T*>(cs->lpCreateParams);
            WINRT_ASSERT(that);
            WINRT_ASSERT(!that->m_window);
            that->m_window = window;
            SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(that));
        }
        else if (T* that = GetThisFromHandle(window))
        {
            return that->MessageHandler(message, wparam, lparam);
        }

        return DefWindowProc(window, message, wparam, lparam);
    }

    LRESULT MessageHandler(UINT const message, WPARAM const wparam, LPARAM const lparam) noexcept
    {
        if (WM_DESTROY == message)
        {
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProc(m_window, message, wparam, lparam);
    }

protected:

    using base_type = DesktopWindow<T>;
    HWND m_window = nullptr;
};

struct Window : DesktopWindow<Window>
{
    Window() noexcept
    {
        WNDCLASS wc{};
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hInstance = reinterpret_cast<HINSTANCE>(&__ImageBase);
        wc.lpszClassName = L"RiveWindow";
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WndProc;
        RegisterClass(&wc);
        WINRT_ASSERT(!m_window);

        WINRT_VERIFY(CreateWindow(wc.lpszClassName,
            L"Rive Desktop Window", 
            WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
            CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, 
            nullptr, nullptr, wc.hInstance, this));

        WINRT_ASSERT(m_window);
    }

    ~Window()
    {
        if (m_riveControl)
        {
            m_riveControl.Shutdown();
            m_riveControl = nullptr;
        }
    }

    LRESULT MessageHandler(UINT const message, WPARAM const wparam, LPARAM const lparam) noexcept
    {
        switch (message)
        {
        case WM_SIZE:
            if (m_riveControl)
            {
                RECT rect;
                GetClientRect(m_window, &rect);
                int width = rect.right - rect.left;
                int height = rect.bottom - rect.top;
                m_riveControl.SetSize(width, height);
            }
            break;

        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
            // Forward mouse messages to WinRive for Rive interaction
            // The WinRive control handles input through its Win32 hosting mode
            break;
        }

        return base_type::MessageHandler(message, wparam, lparam);
    }

    void PrepareVisuals()
    {
        Compositor compositor;
        m_target = CreateDesktopWindowTarget(compositor, m_window);
        auto root = compositor.CreateSpriteVisual();
        root.RelativeSizeAdjustment({ 1.0f, 1.0f });
        root.Brush(compositor.CreateColorBrush({ 0xFF, 0x2D, 0x2D, 0x30 })); // Dark background
        m_target.Root(root);

        // Get initial window size
        RECT rect;
        GetClientRect(m_window, &rect);
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        try
        {
            // Create WinRive control
            m_riveControl = winrt::WinRive::RiveControl();
            
            // Initialize for Win32 hosting with HWND
            if (m_riveControl.InitializeForWin32(compositor, reinterpret_cast<uint64_t>(m_window), width, height))
            {
                // Get the visual from the Rive control and add to composition tree
                auto riveVisual = m_riveControl.GetVisual();
                if (riveVisual)
                {
                    auto visuals = root.Children();
                    visuals.InsertAtTop(riveVisual);
                    
                    // Try to load a Rive file (this path should be updated to point to an actual .riv file)
                    // For now, this will demonstrate the integration even without a file
                    try 
                    {
                        // Try to load from a common location - update this path as needed
                        if (m_riveControl.LoadRiveFile(L"C:\\Users\\jeclarke\\Desktop\\meeting_ui.riv"))
                        {
                            m_riveControl.StartRenderLoop();
                        }
                        else
                        {
                            // If no file found, still start the render loop to show the component is working
                            m_riveControl.StartRenderLoop();
                        }
                    }
                    catch (...)
                    {
                        // Continue without file - the visual will still be created
                        m_riveControl.StartRenderLoop();
                    }
                }
            }
        }
        catch (winrt::hresult_error const&)
        {
            // If WinRive fails to initialize, fall back to showing colored visuals
            // This ensures the window still shows something even if Rive integration fails
            auto visuals = root.Children();
            AddVisual(visuals, 100.0f, 100.0f);
            AddVisual(visuals, 220.0f, 100.0f);
            AddVisual(visuals, 100.0f, 220.0f);
            AddVisual(visuals, 220.0f, 220.0f);
        }
    }

    void AddVisual(VisualCollection const& visuals, float x, float y)
    {
        auto compositor = visuals.Compositor();
        auto visual = compositor.CreateSpriteVisual();

        static Color colors[] =
        {
            { 0xDC, 0x5B, 0x9B, 0xD5 },
            { 0xDC, 0xFF, 0xC0, 0x00 },
            { 0xDC, 0xED, 0x7D, 0x31 },
            { 0xDC, 0x70, 0xAD, 0x47 },
        };

        static unsigned last = 0;
        unsigned const next = ++last % _countof(colors);
        visual.Brush(compositor.CreateColorBrush(colors[next]));
        visual.Size({ 100.0f, 100.0f });
        visual.Offset({ x, y, 0.0f, });

        visuals.InsertAtTop(visual);
    }

private:
    DesktopWindowTarget m_target{ nullptr };
    winrt::WinRive::RiveControl m_riveControl{ nullptr };
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
    init_apartment(apartment_type::single_threaded);
    auto controller = CreateDispatcherQueueController();

    Window window;
    window.PrepareVisuals();
    MSG message;

    while (GetMessage(&message, nullptr, 0, 0))
    {
        DispatchMessage(&message);
    }
}
