#pragma once

// Prevent Windows macros from conflicting with C++ standard library
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// Windows headers
#include <windows.h>

// WinRT headers
#include <winrt/base.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Composition.h>
#include <windows.ui.composition.interop.h>

// DirectX headers
#include <d3d11_1.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

// C++ Standard Library headers
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <fstream>
#include <vector>
#include <iostream>

// Rive headers (only include if available)
#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
#include "rive/renderer/rive_renderer.hpp"
#include "rive/renderer/d3d11/render_context_d3d_impl.hpp"
#include "rive/renderer/d3d11/d3d11.hpp"
#include "rive/artboard.hpp"
#include "rive/file.hpp"
#include "rive/animation/linear_animation_instance.hpp"
#include "rive/animation/state_machine_instance.hpp"
#include "rive/static_scene.hpp"
#endif

class RiveRenderer {
private:
    // Composition API
    winrt::Windows::UI::Composition::Compositor m_compositor{ nullptr };
    winrt::Windows::UI::Composition::SpriteVisual m_riveVisual{ nullptr };
    
    // DirectX 11 resources
    winrt::com_ptr<::ID3D11Device> m_d3dDevice;
    winrt::com_ptr<::ID3D11DeviceContext1> m_d3dContext;
    winrt::com_ptr<::IDXGISwapChain1> m_swapChain;
    winrt::com_ptr<::ID3D11Texture2D> m_backBuffer;
    winrt::com_ptr<::IDXGIFactory2> m_dxgiFactory;
    
#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
    // Rive rendering resources
    Microsoft::WRL::ComPtr<::ID3D11Device> m_riveGpu;
    Microsoft::WRL::ComPtr<::ID3D11DeviceContext> m_riveGpuContext;
    std::unique_ptr<rive::gpu::RenderContext> m_riveRenderContext;
    rive::rcp<rive::gpu::RenderTargetD3D> m_riveRenderTarget;
    std::unique_ptr<rive::Renderer> m_riveRenderer;
    
    // Rive content
    std::unique_ptr<rive::File> m_riveFile;
    std::unique_ptr<rive::Artboard> m_artboard;
    std::unique_ptr<rive::Scene> m_scene;
    rive::rcp<rive::ViewModelInstance> m_viewModelInstance;
#endif
    
    // Rive file data
    std::vector<uint8_t> m_riveFileData;
    std::string m_riveFilePath;
    
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
    RiveRenderer();
    virtual ~RiveRenderer();
    
    // Prevent copying
    RiveRenderer(const RiveRenderer&) = delete;
    RiveRenderer& operator=(const RiveRenderer&) = delete;

    // Initialization
    bool Initialize(const winrt::Windows::UI::Composition::Compositor& compositor, int width = 800, int height = 600);
    void Shutdown();
    
    // Visual management
    winrt::Windows::UI::Composition::SpriteVisual GetVisual() const { return m_riveVisual; }
    void SetSize(int width, int height);
    
    // Content management
    bool LoadRiveFile(const std::string& filePath);
    
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
    
    // Resource cleanup
    void CleanupDeviceResources();
    void CleanupRenderingResources();
};
