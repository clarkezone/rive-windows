#include "pch.h"
#include "rive_window.h"
#include <math.h>
#include <fstream>
#include <wrl/client.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

RiveWindow::RiveWindow()
{
}

RiveWindow::~RiveWindow()
{
    StopRenderThread();
    CleanupRenderingResources();
    CleanupDeviceResources();
}

void RiveWindow::WindowCreated() {
    std::cout << "RiveWindow::WindowCreated() called\n";
    
    try {
        m_controller = CreateDispatcherQueueCont();
        std::cout << "Created dispatcher queue controller\n";
        
        // Get initial window size
        RECT rect;
        GetClientRect(window_handle_, &rect);
        m_windowWidth = rect.right - rect.left;
        m_windowHeight = rect.bottom - rect.top;
        std::cout << "Initial window size: " << m_windowWidth << "x" << m_windowHeight << "\n";
        
        // Initialize DirectX resources
        CreateDeviceResources();
        std::cout << "DirectX resources created successfully\n";
        
        PrepareVisuals();
        std::cout << "Visuals prepared\n";
        
        CreateCompositionSurface();
        std::cout << "Composition surface created\n";
        
        // Initialize Rive context
        CreateRiveContext();
        std::cout << "Rive context initialized\n";
        
        StartRenderThread();
        std::cout << "Render thread started\n";
    }
    catch (winrt::hresult_error const& ex) {
        std::wcout << L"Failed to initialize DirectX: " << ex.message().c_str() << L" (0x" << std::hex << ex.code() << L")\n";
    }
}

void RiveWindow::OnPointerDown(int x, int y) {
    Win32Window::OnPointerDown(x, y);
    // TODO: Handle Rive input events
}

void RiveWindow::OnDpiChanged(int dpi) {
    Win32Window::OnDpiChanged(dpi);
    // Handle DPI change for Rive content if needed
}

void RiveWindow::OnResize(int width, int height) {
    Win32Window::OnResize(width, height);
    
    std::lock_guard<std::mutex> lock(m_deviceMutex);
    
    if (width > 0 && height > 0 && (width != m_windowWidth || height != m_windowHeight)) {
        m_windowWidth = width;
        m_windowHeight = height;
        
        // Resize the swap chain
        if (m_swapChain) {
            // Clear render target
            m_backBuffer = nullptr;
            
            HRESULT hr = m_swapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
            if (SUCCEEDED(hr)) {
                CreateRenderTarget();
            }
        }
    }
}

// Forward declaration - function is defined in dx_window.cpp
DesktopWindowTarget CreateDesktopWindowTarget(Compositor const& compositor, HWND window);

winrt::Windows::System::DispatcherQueueController RiveWindow::CreateDispatcherQueueCont()
{
    namespace abi = ABI::Windows::System;

    DispatcherQueueOptions options
    {
        sizeof(DispatcherQueueOptions),
        DQTYPE_THREAD_CURRENT,
        DQTAT_COM_STA
    };

    winrt::Windows::System::DispatcherQueueController controller{ nullptr };
    check_hresult(CreateDispatcherQueueController(options, reinterpret_cast<abi::IDispatcherQueueController**>(put_abi(controller))));
    return controller;
}

void RiveWindow::PrepareVisuals()
{
    m_compositor = Compositor();
    m_target = CreateDesktopWindowTarget(m_compositor, window_handle_);
    m_root = m_compositor.CreateContainerVisual();
    m_root.RelativeSizeAdjustment({ 1.0f, 1.0f });
    m_target.Root(m_root);
}

void RiveWindow::CreateCompositionSurface()
{
    if (!m_swapChain || !m_compositor) return;

    // Create a sprite visual for the DirectX content
    m_dxVisual = m_compositor.CreateSpriteVisual();
    m_dxVisual.RelativeSizeAdjustment({ 1.0f, 1.0f });
    
    // Use ICompositorInterop to create composition surface for swap chain
    auto compositorInterop = m_compositor.as<ABI::Windows::UI::Composition::ICompositorInterop>();
    
    winrt::com_ptr<ABI::Windows::UI::Composition::ICompositionSurface> compositionSurface;
    check_hresult(compositorInterop->CreateCompositionSurfaceForSwapChain(
        m_swapChain.get(),
        compositionSurface.put()
    ));
    
    // Create surface brush from the composition surface
    auto surface = compositionSurface.as<winrt::Windows::UI::Composition::ICompositionSurface>();
    auto surfaceBrush = m_compositor.CreateSurfaceBrush(surface);
    m_dxVisual.Brush(surfaceBrush);
    
    // Add to root
    m_root.Children().InsertAtTop(m_dxVisual);
}

void RiveWindow::CreateDeviceResources()
{
    // Create D3D11 device
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    winrt::com_ptr<::ID3D11Device> device;
    winrt::com_ptr<::ID3D11DeviceContext> context;
    
    check_hresult(D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        device.put(),
        nullptr,
        context.put()
    ));

    // Get D3D11.1 interfaces
    check_hresult(device->QueryInterface(IID_PPV_ARGS(m_d3dDevice.put())));
    check_hresult(context->QueryInterface(IID_PPV_ARGS(m_d3dContext.put())));

    // Create DXGI factory
    winrt::com_ptr<::IDXGIDevice1> dxgiDevice;
    check_hresult(m_d3dDevice->QueryInterface(IID_PPV_ARGS(dxgiDevice.put())));

    winrt::com_ptr<::IDXGIAdapter> adapter;
    check_hresult(dxgiDevice->GetAdapter(adapter.put()));

    check_hresult(adapter->GetParent(IID_PPV_ARGS(m_dxgiFactory.put())));

    // Create swap chain
    CreateSwapChain();

    // Create render target
    CreateRenderTarget();
}

void RiveWindow::CreateSwapChain()
{
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = m_windowWidth;
    swapChainDesc.Height = m_windowHeight;
    swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;

    check_hresult(m_dxgiFactory->CreateSwapChainForComposition(
        m_d3dDevice.get(),
        &swapChainDesc,
        nullptr,
        m_swapChain.put()
    ));
}

void RiveWindow::CreateRenderTarget()
{
    if (!m_swapChain) return;

    check_hresult(m_swapChain->GetBuffer(0, IID_PPV_ARGS(m_backBuffer.put())));
    
    // TODO: Create Rive render target when Rive dependencies are available
}

void RiveWindow::CreateRiveContext()
{
    auto d3dContextOptions = rive::gpu::D3DContextOptions{};
    m_riveRenderContext = rive::gpu::RenderContextD3DImpl::MakeContext(
        m_d3dDevice.get(),
        m_d3dContext.get(),
        d3dContextOptions
    );
    
    if (m_riveRenderContext) {
        auto renderContextImpl = m_riveRenderContext->static_impl_cast<rive::gpu::RenderContextD3DImpl>();
        m_riveRenderTarget = renderContextImpl->makeRenderTarget(m_windowWidth, m_windowHeight);
        m_riveRenderer = std::make_unique<rive::RiveRenderer>(m_riveRenderContext.get());
    }
}

void RiveWindow::CreateRiveContent()
{
    if (!m_riveFileData.empty() && m_riveRenderContext) {
        m_riveFile = rive::File::import(m_riveFileData, m_riveRenderContext.get());
        if (m_riveFile) {
            m_artboard = m_riveFile->artboardDefault();
            if (m_artboard) {
                // Create static scene - try without parameters first
                m_scene = std::make_unique<rive::StaticScene>();
                
                m_viewModelInstance = m_riveFile->createViewModelInstance(m_artboard.get());
                if (m_viewModelInstance) {
                    m_artboard->bindViewModelInstance(m_viewModelInstance);
                    if (m_scene) {
                        m_scene->bindViewModelInstance(m_viewModelInstance);
                    }
                }
            }
        }
    }
}

void RiveWindow::RecreateDeviceResources()
{
    CleanupRenderingResources();
    CleanupDeviceResources();
    
    CreateDeviceResources();
    CreateCompositionSurface();
    CreateRiveContext();
}

void RiveWindow::StartRenderThread()
{
    m_shouldRender = true;
    m_isPaused = false;
    m_renderThread = std::thread(&RiveWindow::RenderLoop, this);
}

void RiveWindow::StopRenderThread()
{
    m_shouldRender = false;
    if (m_renderThread.joinable()) {
        m_renderThread.join();
    }
}

void RiveWindow::RenderLoop()
{
    while (m_shouldRender) {
        if (!m_isPaused && !m_deviceLost) {
            std::lock_guard<std::mutex> lock(m_deviceMutex);
            
            if (CheckDeviceLost()) {
                HandleDeviceLost();
                continue;
            }
            
            RenderRive();
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }
}

void RiveWindow::RenderRive()
{
    if (!m_d3dContext || !m_swapChain) return;

    if (m_riveRenderer && m_riveRenderTarget && m_scene) {
        // Get fresh backbuffer from swap chain (following path_fiddle pattern)
        Microsoft::WRL::ComPtr<ID3D11Texture2D> backbuffer;
        check_hresult(m_swapChain->GetBuffer(0, IID_PPV_ARGS(backbuffer.ReleaseAndGetAddressOf())));
        
        // Set render target texture
        m_riveRenderTarget->setTargetTexture(backbuffer);
        
        // Begin frame
        m_riveRenderContext->beginFrame({
            .renderTargetWidth = static_cast<uint32_t>(m_windowWidth),
            .renderTargetHeight = static_cast<uint32_t>(m_windowHeight),
            .clearColor = 0xff404040,
            .msaaSampleCount = 0
        });
        
        // Advance animation
        m_scene->advanceAndApply(1.0f / 60.0f);
        
        // Calculate transform to fit content
        rive::Mat2D transform = rive::computeAlignment(
            rive::Fit::contain,
            rive::Alignment::center,
            rive::AABB(0, 0, m_windowWidth, m_windowHeight),
            m_artboard->bounds()
        );
        
        // Render
        m_riveRenderer->save();
        m_riveRenderer->transform(transform);
        m_scene->draw(m_riveRenderer.get());
        m_riveRenderer->restore();
        
        // Flush and present
        m_riveRenderContext->flush({.renderTarget = m_riveRenderTarget.get()});
        m_riveRenderTarget->setTargetTexture(nullptr);
    }
    else {
        // Fallback: clear to test color if no Rive content
        winrt::com_ptr<::ID3D11Texture2D> backbuffer;
        check_hresult(m_swapChain->GetBuffer(0, IID_PPV_ARGS(backbuffer.put())));
        
        winrt::com_ptr<::ID3D11RenderTargetView> renderTargetView;
        check_hresult(m_d3dDevice->CreateRenderTargetView(
            backbuffer.get(),
            nullptr,
            renderTargetView.put()
        ));

        float clearColor[4] = { 0.2f, 0.2f, 0.4f, 1.0f }; // Dark blue
        m_d3dContext->ClearRenderTargetView(renderTargetView.get(), clearColor);
    }

    // Present the frame
    m_swapChain->Present(1, 0);
}

bool RiveWindow::LoadRiveFile(const std::string& filePath)
{
    try {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            std::cout << "Failed to open Rive file: " << filePath << std::endl;
            return false;
        }
        
        // Read file data
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        m_riveFileData.resize(fileSize);
        file.read(reinterpret_cast<char*>(m_riveFileData.data()), fileSize);
        file.close();
        
        m_riveFilePath = filePath;
        
        std::cout << "Loaded Rive file: " << filePath << " (" << fileSize << " bytes)" << std::endl;
        
        // Create Rive content
        CreateRiveContent();
        
        return true;
    }
    catch (const std::exception& e) {
        std::cout << "Error loading Rive file: " << e.what() << std::endl;
        return false;
    }
}

bool RiveWindow::CheckDeviceLost()
{
    if (!m_d3dDevice) return true;
    
    HRESULT hr = m_d3dDevice->GetDeviceRemovedReason();
    return FAILED(hr);
}

void RiveWindow::HandleDeviceLost()
{
    m_deviceLost = true;
    try {
        RecreateDeviceResources();
        m_deviceLost = false;
    }
    catch (winrt::hresult_error const&) {
        // Device recreation failed, stay in device lost state
    }
}

void RiveWindow::PauseRendering()
{
    m_isPaused = true;
}

void RiveWindow::ResumeRendering()
{
    m_isPaused = false;
}

void RiveWindow::CleanupDeviceResources()
{
    m_backBuffer = nullptr;
    m_swapChain = nullptr;
    m_dxgiFactory = nullptr;
    m_d3dContext = nullptr;
    m_d3dDevice = nullptr;
}

void RiveWindow::CleanupRenderingResources()
{
    m_riveRenderer = nullptr;
    m_riveRenderTarget = nullptr;
    m_riveRenderContext = nullptr;
    m_viewModelInstance = nullptr;
    m_scene = nullptr;
    m_artboard = nullptr;
    m_riveFile = nullptr;
    m_riveFileData.clear();
    m_riveFilePath.clear();
}
