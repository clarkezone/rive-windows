#pragma once
#include "pch.h"
#include "win32_window.h"
using namespace winrt;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Composition::Desktop;

class DXWindow : public Win32Window {
private:
    // Composition API
    DesktopWindowTarget m_target{ nullptr };
    Windows::System::DispatcherQueueController m_controller{ nullptr };
    Compositor m_compositor{ nullptr };
    ContainerVisual m_root{ nullptr };
    SpriteVisual m_dxVisual{ nullptr };
    
    // DirectX 11 resources
    winrt::com_ptr<::ID3D11Device1> m_d3dDevice;
    winrt::com_ptr<::ID3D11DeviceContext1> m_d3dContext;
    winrt::com_ptr<::IDXGISwapChain1> m_swapChain;
    winrt::com_ptr<::ID3D11Texture2D> m_backBuffer;
    
    // Direct2D resources
    winrt::com_ptr<::ID2D1Factory1> m_d2dFactory;
    winrt::com_ptr<::ID2D1Device> m_d2dDevice;
    winrt::com_ptr<::ID2D1DeviceContext> m_d2dContext;
    winrt::com_ptr<::ID2D1Bitmap1> m_d2dTargetBitmap;
    
    // DirectWrite resources
    winrt::com_ptr<::IDWriteFactory> m_dwriteFactory;
    winrt::com_ptr<::IDWriteTextFormat> m_textFormat;
    
    // Brushes and resources
    winrt::com_ptr<::ID2D1SolidColorBrush> m_clockBrush;
    winrt::com_ptr<::ID2D1SolidColorBrush> m_backgroundBrush;
    winrt::com_ptr<::ID2D1SolidColorBrush> m_textBrush;
    
    // Threading
    std::thread m_renderThread;
    std::atomic<bool> m_shouldRender{ true };
    std::atomic<bool> m_isPaused{ false };
    std::mutex m_deviceMutex;
    
    // Window state
    int m_windowWidth = 0;
    int m_windowHeight = 0;
    bool m_deviceLost = false;

public:
    void StartRenderThread();
    void StopRenderThread();

protected:
    DXWindow();
    virtual ~DXWindow();

    virtual void OnPointerDown(int x, int y) override;
    virtual void OnDpiChanged(int dpi) override;
    virtual void OnResize(int width, int height) override;
    virtual void WindowCreated() override;
    
private:
    // Composition setup
    Windows::System::DispatcherQueueController CreateDispatcherQueueCont();
    void PrepareVisuals();
    void CreateCompositionSurface();
    
    // DirectX initialization
    void CreateDeviceResources();
    void CreateSwapChain();
    void CreateRenderTarget();
    void RecreateDeviceResources();
    
    // Rendering
    void RenderLoop();
    void RenderClock();
    void DrawClockFace(float centerX, float centerY, float radius);
    void DrawClockHand(float centerX, float centerY, float angle, float length, float thickness);
    void DrawDigitalTime(float x, float y);
    
    // Device management
    bool CheckDeviceLost();
    void HandleDeviceLost();
    void PauseRendering();
    void ResumeRendering();
    
    // Resource cleanup
    void CleanupDeviceResources();
    void CleanupRenderingResources();
};
