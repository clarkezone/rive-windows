#pragma once

#include "pch.h"
#include <functional>
#include <chrono>
#include <memory>

namespace WinRive::Implementation
{
    // Input event structure for unified input handling
    struct InputEvent {
        enum Type { Move, Press, Release };
        Type type;
        float x, y;  // Relative to control bounds (0,0 to width,height)
        std::chrono::steady_clock::time_point timestamp;
    };

    // Abstract base class for input providers
    class IInputProvider
    {
    public:
        virtual ~IInputProvider() = default;
        
        // Initialize the input provider
        virtual bool Initialize() = 0;
        
        // Cleanup resources
        virtual void Shutdown() = 0;
        
        // Set the bounds for coordinate transformation
        virtual void SetBounds(int width, int height) = 0;
        
        // Event callback type
        using InputEventCallback = std::function<void(const InputEvent&)>;
        
        // Set the callback for input events
        virtual void SetInputEventCallback(InputEventCallback callback) = 0;
    };

    // CoreWindow-based input provider for UWP scenarios
    class CoreWindowInputProvider : public IInputProvider
    {
    public:
        CoreWindowInputProvider(winrt::Windows::UI::Core::CoreWindow const& coreWindow);
        ~CoreWindowInputProvider();

        bool Initialize() override;
        void Shutdown() override;
        void SetBounds(int width, int height) override;
        void SetInputEventCallback(InputEventCallback callback) override;

    private:
        winrt::Windows::UI::Core::CoreWindow m_coreWindow{ nullptr };
        winrt::event_token m_pointerMovedToken;
        winrt::event_token m_pointerPressedToken;
        winrt::event_token m_pointerReleasedToken;
        
        InputEventCallback m_callback;
        int m_width{ 0 };
        int m_height{ 0 };
        
        void OnPointerMoved(winrt::Windows::UI::Core::CoreWindow const& sender,
                           winrt::Windows::UI::Core::PointerEventArgs const& args);
        void OnPointerPressed(winrt::Windows::UI::Core::CoreWindow const& sender,
                             winrt::Windows::UI::Core::PointerEventArgs const& args);
        void OnPointerReleased(winrt::Windows::UI::Core::CoreWindow const& sender,
                              winrt::Windows::UI::Core::PointerEventArgs const& args);
        
        bool IsPointInBounds(winrt::Windows::Foundation::Point const& point);
    };

    // Win32 HWND-based input provider
    class Win32InputProvider : public IInputProvider
    {
    public:
        Win32InputProvider(HWND hwnd);
        ~Win32InputProvider();

        bool Initialize() override;
        void Shutdown() override;
        void SetBounds(int width, int height) override;
        void SetInputEventCallback(InputEventCallback callback) override;

        // Called from Win32 window procedure
        void HandleWin32Message(UINT message, WPARAM wParam, LPARAM lParam);

    private:
        HWND m_hwnd;
        InputEventCallback m_callback;
        int m_width{ 0 };
        int m_height{ 0 };
        bool m_isTracking{ false };
        
        void TrackMouseLeave();
        bool IsPointInBounds(int x, int y);
    };

    // Null input provider for scenarios without input handling
    class NullInputProvider : public IInputProvider
    {
    public:
        bool Initialize() override { return true; }
        void Shutdown() override {}
        void SetBounds(int width, int height) override {}
        void SetInputEventCallback(InputEventCallback callback) override {}
    };

    // Factory for creating input providers
    class InputProviderFactory
    {
    public:
        static std::unique_ptr<IInputProvider> CreateForUWP(winrt::Windows::UI::Core::CoreWindow const& coreWindow);
        static std::unique_ptr<IInputProvider> CreateForWinUI3();
        static std::unique_ptr<IInputProvider> CreateForWin32(HWND hwnd);
    };
}
