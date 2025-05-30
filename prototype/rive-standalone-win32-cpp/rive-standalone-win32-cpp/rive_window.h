#pragma once
#include "pch.h"
#include "win32_window.h"

#include "rive/renderer/rive_renderer.hpp"
#include "rive/renderer/d3d11/render_context_d3d_impl.hpp"
#include "rive/renderer/d3d11/d3d11.hpp"
#include "rive/artboard.hpp"
#include "rive/file.hpp"
#include "rive/animation/linear_animation_instance.hpp"
#include "rive/animation/state_machine_instance.hpp"
#include "rive/static_scene.hpp"

#include <fstream>
#include <vector>

using namespace winrt;
using namespace winrt::Windows::UI;
using namespace winrt::Windows::UI::Composition;
using namespace winrt::Windows::UI::Composition::Desktop;

class RiveWindow : public Win32Window {
private:
    // Composition API
    DesktopWindowTarget m_target{ nullptr };
    winrt::Windows::System::DispatcherQueueController m_controller{ nullptr };
    Compositor m_compositor{ nullptr };
    ContainerVisual m_root{ nullptr };
    SpriteVisual m_dxVisual{ nullptr };
    
    // DirectX 11 resources
    winrt::com_ptr<::ID3D11Device> m_d3dDevice;
    winrt::com_ptr<::ID3D11DeviceContext1> m_d3dContext;
    winrt::com_ptr<::IDXGISwapChain1> m_swapChain;
    winrt::com_ptr<::ID3D11Texture2D> m_backBuffer;
    winrt::com_ptr<::IDXGIFactory2> m_dxgiFactory;
    
    // Rive rendering resources (TODO: enable when Rive headers are available)
    ComPtr<::ID3D11Device> m_riveGpu;
    ComPtr<::ID3D11DeviceContext> m_riveGpuContext;
    std::unique_ptr<rive::gpu::RenderContext> m_riveRenderContext;
    rive::rcp<rive::gpu::RenderTargetD3D> m_riveRenderTarget;
    std::unique_ptr<rive::Renderer> m_riveRenderer;
    
    // Rive content (TODO: enable when Rive headers are available)
    std::unique_ptr<rive::File> m_riveFile;
    std::unique_ptr<rive::Artboard> m_artboard;
    std::unique_ptr<rive::Scene> m_scene;
    rive::rcp<rive::ViewModelInstance> m_viewModelInstance;
    
    // Placeholder for Rive file data
    std::vector<uint8_t> m_riveFileData;
    std::string m_riveFilePath;
    
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
    RiveWindow();
    virtual ~RiveWindow();
    
    void StartRenderThread();
    void StopRenderThread();
    bool LoadRiveFile(const std::string& filePath);

protected:

    virtual void OnPointerDown(int x, int y) override;
    virtual void OnDpiChanged(int dpi) override;
    virtual void OnResize(int width, int height) override;
    virtual void WindowCreated() override;
    
private:
    // Composition setup
    winrt::Windows::System::DispatcherQueueController CreateDispatcherQueueCont();
    void PrepareVisuals();
    void CreateCompositionSurface();
    
    // DirectX initialization
    void CreateDeviceResources();
    void CreateSwapChain();
    void CreateRenderTarget();
    void RecreateDeviceResources();
    
    // Rive setup
    void CreateRiveContext();
    void CreateRiveContent();
    void ClearScene();
    void MakeScene();
    
    // Rendering
    void RenderLoop();
    void RenderRive();
    
    // Device management
    bool CheckDeviceLost();
    void HandleDeviceLost();
    void PauseRendering();
    void ResumeRendering();
    
    // Resource cleanup
    void CleanupDeviceResources();
    void CleanupRenderingResources();
};
