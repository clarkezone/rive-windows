#include "pch.h"
#include "InputProvider.h"
#include <windowsx.h>

namespace WinRive::Implementation
{
    // CoreWindowInputProvider implementation
    CoreWindowInputProvider::CoreWindowInputProvider(winrt::Windows::UI::Core::CoreWindow const& coreWindow)
        : m_coreWindow(coreWindow)
    {
    }

    CoreWindowInputProvider::~CoreWindowInputProvider()
    {
        Shutdown();
    }

    bool CoreWindowInputProvider::Initialize()
    {
        if (!m_coreWindow)
            return false;

        try
        {
            // Register for pointer events
            m_pointerMovedToken = m_coreWindow.PointerMoved({ this, &CoreWindowInputProvider::OnPointerMoved });
            m_pointerPressedToken = m_coreWindow.PointerPressed({ this, &CoreWindowInputProvider::OnPointerPressed });
            m_pointerReleasedToken = m_coreWindow.PointerReleased({ this, &CoreWindowInputProvider::OnPointerReleased });
            
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    void CoreWindowInputProvider::Shutdown()
    {
        if (m_coreWindow)
        {
            m_coreWindow.PointerMoved(m_pointerMovedToken);
            m_coreWindow.PointerPressed(m_pointerPressedToken);
            m_coreWindow.PointerReleased(m_pointerReleasedToken);
        }
    }

    void CoreWindowInputProvider::SetBounds(int width, int height)
    {
        m_width = width;
        m_height = height;
    }

    void CoreWindowInputProvider::SetInputEventCallback(InputEventCallback callback)
    {
        m_callback = callback;
    }

    void CoreWindowInputProvider::OnPointerMoved(winrt::Windows::UI::Core::CoreWindow const& sender,
                                                 winrt::Windows::UI::Core::PointerEventArgs const& args)
    {
        if (!m_callback || !IsPointInBounds(args.CurrentPoint().Position()))
            return;

        InputEvent event;
        event.type = InputEvent::Move;
        event.x = static_cast<float>(args.CurrentPoint().Position().X);
        event.y = static_cast<float>(args.CurrentPoint().Position().Y);
        event.timestamp = std::chrono::steady_clock::now();

        m_callback(event);
    }

    void CoreWindowInputProvider::OnPointerPressed(winrt::Windows::UI::Core::CoreWindow const& sender,
                                                   winrt::Windows::UI::Core::PointerEventArgs const& args)
    {
        if (!m_callback || !IsPointInBounds(args.CurrentPoint().Position()))
            return;

        InputEvent event;
        event.type = InputEvent::Press;
        event.x = static_cast<float>(args.CurrentPoint().Position().X);
        event.y = static_cast<float>(args.CurrentPoint().Position().Y);
        event.timestamp = std::chrono::steady_clock::now();

        m_callback(event);
    }

    void CoreWindowInputProvider::OnPointerReleased(winrt::Windows::UI::Core::CoreWindow const& sender,
                                                    winrt::Windows::UI::Core::PointerEventArgs const& args)
    {
        if (!m_callback)
            return;

        InputEvent event;
        event.type = InputEvent::Release;
        event.x = static_cast<float>(args.CurrentPoint().Position().X);
        event.y = static_cast<float>(args.CurrentPoint().Position().Y);
        event.timestamp = std::chrono::steady_clock::now();

        m_callback(event);
    }

    bool CoreWindowInputProvider::IsPointInBounds(winrt::Windows::Foundation::Point const& point)
    {
        return point.X >= 0 && point.Y >= 0 && 
               point.X < m_width && point.Y < m_height;
    }

    // Win32InputProvider implementation
    Win32InputProvider::Win32InputProvider(HWND hwnd)
        : m_hwnd(hwnd)
    {
    }

    Win32InputProvider::~Win32InputProvider()
    {
        Shutdown();
    }

    bool Win32InputProvider::Initialize()
    {
        return m_hwnd != nullptr;
    }

    void Win32InputProvider::Shutdown()
    {
        // No special cleanup needed for Win32 input
    }

    void Win32InputProvider::SetBounds(int width, int height)
    {
        m_width = width;
        m_height = height;
    }

    void Win32InputProvider::SetInputEventCallback(InputEventCallback callback)
    {
        m_callback = callback;
    }

    void Win32InputProvider::HandleWin32Message(UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (!m_callback)
            return;

        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);

        InputEvent event;
        event.timestamp = std::chrono::steady_clock::now();

        switch (message)
        {
        case WM_MOUSEMOVE:
            if (IsPointInBounds(x, y))
            {
                event.type = InputEvent::Move;
                event.x = static_cast<float>(x);
                event.y = static_cast<float>(y);
                m_callback(event);
                
                // Track mouse leave events
                if (!m_isTracking)
                {
                    TrackMouseLeave();
                    m_isTracking = true;
                }
            }
            break;

        case WM_LBUTTONDOWN:
            if (IsPointInBounds(x, y))
            {
                event.type = InputEvent::Press;
                event.x = static_cast<float>(x);
                event.y = static_cast<float>(y);
                m_callback(event);
                SetCapture(m_hwnd);
            }
            break;

        case WM_LBUTTONUP:
            event.type = InputEvent::Release;
            event.x = static_cast<float>(x);
            event.y = static_cast<float>(y);
            m_callback(event);
            ReleaseCapture();
            break;

        case WM_MOUSELEAVE:
            m_isTracking = false;
            break;
        }
    }

    void Win32InputProvider::TrackMouseLeave()
    {
        TRACKMOUSEEVENT tme = {};
        tme.cbSize = sizeof(tme);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = m_hwnd;
        TrackMouseEvent(&tme);
    }

    bool Win32InputProvider::IsPointInBounds(int x, int y)
    {
        return x >= 0 && y >= 0 && x < m_width && y < m_height;
    }

    // InputProviderFactory implementation
    std::unique_ptr<IInputProvider> InputProviderFactory::CreateForUWP(winrt::Windows::UI::Core::CoreWindow const& coreWindow)
    {
        return std::make_unique<CoreWindowInputProvider>(coreWindow);
    }

    std::unique_ptr<IInputProvider> InputProviderFactory::CreateForWinUI3()
    {
        // For WinUI3, we'll use a null provider for now since input is typically handled at the XAML level
        // This can be enhanced later with specific WinUI3 input handling if needed
        return std::make_unique<NullInputProvider>();
    }

    std::unique_ptr<IInputProvider> InputProviderFactory::CreateForWin32(HWND hwnd)
    {
        return std::make_unique<Win32InputProvider>(hwnd);
    }
}
