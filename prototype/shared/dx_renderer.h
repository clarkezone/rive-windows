#pragma once

// Prevent Windows macros from conflicting with C++ standard library
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

// Windows headers
#include <windows.h>

// WinRT headers
#include <winrt/base.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Composition.h>
#include <windows.ui.composition.interop.h>

// DirectX headers
#include <d3d11_1.h>
#include <d2d1_1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <dxgi1_2.h>

// C++ Standard Library headers
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

using namespace winrt;
using namespace Windows::UI;
using namespace Windows::UI::Composition;

class DXRenderer {
private:
    // Composition API
    Compositor m_compositor{ nullptr };
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
    
    // Rendering state
    int m_renderWidth = 800;
    int m_renderHeight = 600;
    bool m_deviceLost = false;

public:
    DXRenderer();
    virtual ~DXRenderer();
    
    // Prevent copying
    DXRenderer(const DXRenderer&) = delete;
    DXRenderer& operator=(const DXRenderer&) = delete;

    // Initialization
    bool Initialize(const Compositor& compositor, int width = 800, int height = 600);
    void Shutdown();
    
    // Visual management
    SpriteVisual GetVisual() const { return m_dxVisual; }
    void SetSize(int width, int height);
    
    // Rendering control
    void StartRenderThread();
    void StopRenderThread();
    void PauseRendering();
    void ResumeRendering();

private:
    // Composition setup
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
    
    // Resource cleanup
    void CleanupDeviceResources();
    void CleanupRenderingResources();
};
