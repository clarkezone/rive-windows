#include "rive_renderer.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

RiveRenderer::RiveRenderer()
{
#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
    // Initialize transform matrix to identity
    m_artboardTransform = rive::Mat2D();
#endif
    m_transformValid = false;
    m_lastPointerDown = false;
}

RiveRenderer::~RiveRenderer()
{
    StopRenderThread();
    CleanupRenderingResources();
    CleanupDeviceResources();
}

bool RiveRenderer::Initialize(const winrt::Windows::UI::Composition::Compositor& compositor, int width, int height)
{
    try {
        m_compositor = compositor;
        m_renderWidth = width;
        m_renderHeight = height;
        
        // Initialize DirectX resources
        CreateDeviceResources();
        std::cout << "DirectX resources created successfully\n";
        
        CreateCompositionSurface();
        std::cout << "Composition surface created\n";
        
        // Initialize Rive context
        CreateRiveContext();
        std::cout << "Rive context initialized\n";
        
        return true;
    }
    catch (winrt::hresult_error const& ex) {
        std::wcout << L"Failed to initialize RiveRenderer: " << ex.message().c_str() << L" (0x" << std::hex << ex.code() << L")\n";
        return false;
    }
}

void RiveRenderer::Shutdown()
{
    StopRenderThread();
    CleanupRenderingResources();
    CleanupDeviceResources();
    m_riveVisual = nullptr;
    m_compositor = nullptr;
}

void RiveRenderer::SetSize(int width, int height)
{
    std::lock_guard<std::mutex> lock(m_deviceMutex);
    
    if (width > 0 && height > 0 && (width != m_renderWidth || height != m_renderHeight)) {
        m_renderWidth = width;
        m_renderHeight = height;
        
        // Invalidate artboard alignment transform
        m_transformValid = false;
        
        // Resize the swap chain
        if (m_swapChain) {
            m_backBuffer = nullptr;
            
            HRESULT hr = m_swapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
            if (SUCCEEDED(hr)) {
                CreateRiveContext();
            }
        }
        
        // Update visual size
        if (m_riveVisual) {
            m_riveVisual.Size({ static_cast<float>(width), static_cast<float>(height) });
        }
    }
}

bool RiveRenderer::LoadRiveFile(const std::string& filePath)
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

void RiveRenderer::CreateCompositionSurface()
{
    if (!m_swapChain || !m_compositor) return;

    // Create a sprite visual for the DirectX content
    m_riveVisual = m_compositor.CreateSpriteVisual();
    m_riveVisual.Size({ static_cast<float>(m_renderWidth), static_cast<float>(m_renderHeight) });
    
    // Use ICompositorInterop to create composition surface for swap chain
    auto compositorInterop = m_compositor.as<ABI::Windows::UI::Composition::ICompositorInterop>();
    
    winrt::com_ptr<ABI::Windows::UI::Composition::ICompositionSurface> compositionSurface;
    winrt::check_hresult(compositorInterop->CreateCompositionSurfaceForSwapChain(
        m_swapChain.get(),
        compositionSurface.put()
    ));
    
    // Create surface brush from the composition surface
    auto surface = compositionSurface.as<winrt::Windows::UI::Composition::ICompositionSurface>();
    auto surfaceBrush = m_compositor.CreateSurfaceBrush(surface);
    m_riveVisual.Brush(surfaceBrush);
}

void RiveRenderer::CreateDeviceResources()
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
    
    winrt::check_hresult(D3D11CreateDevice(
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
    winrt::check_hresult(device->QueryInterface(IID_PPV_ARGS(m_d3dDevice.put())));
    winrt::check_hresult(context->QueryInterface(IID_PPV_ARGS(m_d3dContext.put())));

    // Create DXGI factory
    winrt::com_ptr<::IDXGIDevice1> dxgiDevice;
    winrt::check_hresult(m_d3dDevice->QueryInterface(IID_PPV_ARGS(dxgiDevice.put())));

    winrt::com_ptr<::IDXGIAdapter> adapter;
    winrt::check_hresult(dxgiDevice->GetAdapter(adapter.put()));

    winrt::check_hresult(adapter->GetParent(IID_PPV_ARGS(m_dxgiFactory.put())));

    // Create swap chain
    CreateSwapChain();
}

void RiveRenderer::CreateSwapChain()
{
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = m_renderWidth;
    swapChainDesc.Height = m_renderHeight;
    swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;

    winrt::check_hresult(m_dxgiFactory->CreateSwapChainForComposition(
        m_d3dDevice.get(),
        &swapChainDesc,
        nullptr,
        m_swapChain.put()
    ));
}

void RiveRenderer::CreateRenderTarget()
{
    // This will be implemented when Rive dependencies are properly configured
}

void RiveRenderer::CreateRiveContext()
{
#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
    auto d3dContextOptions = rive::gpu::D3DContextOptions{};

    m_riveGpu = m_d3dDevice.get();
    m_riveGpuContext = m_d3dContext.get();
    m_riveRenderContext = rive::gpu::RenderContextD3DImpl::MakeContext(m_riveGpu,
        m_riveGpuContext,
        d3dContextOptions);

    if (m_riveRenderContext) {
        auto renderContextImpl = m_riveRenderContext->static_impl_cast<rive::gpu::RenderContextD3DImpl>();
        m_riveRenderTarget = renderContextImpl->makeRenderTarget(m_renderWidth, m_renderHeight);
        m_riveRenderer = std::make_unique<rive::RiveRenderer>(m_riveRenderContext.get());
    }
#endif
}

void RiveRenderer::CreateRiveContent()
{
#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
    if (!m_riveFileData.empty() && m_riveRenderContext) {
        m_riveFile = rive::File::import(m_riveFileData, m_riveRenderContext.get());
        if (m_riveFile) {
            MakeScene();
        }
    }
#endif
}

void RiveRenderer::ClearScene()
{
#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
    m_artboard = nullptr;
    m_scene = nullptr; 
    m_viewModelInstance = nullptr;
#endif
}

void RiveRenderer::MakeScene()
{
#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
    ClearScene();
    
    // Following path_fiddle make_scenes pattern exactly
    auto artboard = m_riveFile->artboardDefault();
    std::unique_ptr<rive::Scene> scene;
    
    // Try animationAt(0) first (path_fiddle default pattern)
    scene = artboard->animationAt(0);
    
    if (scene == nullptr) {
        // This is a riv without any animations or state machines. Just draw the artboard.
        scene = std::make_unique<rive::StaticScene>(artboard.get());
    }

    m_viewModelInstance = m_riveFile->createViewModelInstance(artboard.get());
    artboard->bindViewModelInstance(m_viewModelInstance);
    if (m_viewModelInstance != nullptr) {
        scene->bindViewModelInstance(m_viewModelInstance);
    }

    // Set initial animation state
    scene->advanceAndApply(0.0f);
    
    // Store the artboard and scene (following path_fiddle pattern)
    m_artboard = std::move(artboard);
    m_scene = std::move(scene);
#endif
}

void RiveRenderer::RecreateDeviceResources()
{
    CleanupRenderingResources();
    CleanupDeviceResources();
    
    CreateDeviceResources();
    CreateCompositionSurface();
    CreateRiveContext();
}

void RiveRenderer::StartRenderThread()
{
    m_shouldRender = true;
    m_isPaused = false;
    m_renderThread = std::thread(&RiveRenderer::RenderLoop, this);
}

void RiveRenderer::StopRenderThread()
{
    m_shouldRender = false;
    if (m_renderThread.joinable()) {
        m_renderThread.join();
    }
}

void RiveRenderer::RenderLoop()
{
    while (m_shouldRender) {
        if (!m_isPaused && !m_deviceLost) {
            std::lock_guard<std::mutex> lock(m_deviceMutex);
            
            if (CheckDeviceLost()) {
                HandleDeviceLost();
                continue;
            }
            
            // Process input events at start of render frame
            ProcessInputQueue();
            
            RenderRive();
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }
}

void RiveRenderer::RenderRive()
{
    if (!m_d3dContext || !m_swapChain) return;

#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
    if (m_riveRenderer && m_riveRenderTarget && m_scene) {
        // Get fresh backbuffer from swap chain (following path_fiddle pattern)
        Microsoft::WRL::ComPtr<ID3D11Texture2D> backbuffer;
        winrt::check_hresult(m_swapChain->GetBuffer(0, IID_PPV_ARGS(backbuffer.ReleaseAndGetAddressOf())));
        
        // Set render target texture
        m_riveRenderTarget->setTargetTexture(backbuffer);
        
        // Begin frame
        m_riveRenderContext->beginFrame({
            .renderTargetWidth = static_cast<uint32_t>(m_renderWidth),
            .renderTargetHeight = static_cast<uint32_t>(m_renderHeight),
            .clearColor = 0xff404040,
            .msaaSampleCount = 0
        });
        
        // Advance animation
        m_scene->advanceAndApply(1.0f / 60.0f);
        
        // Calculate transform to fit content
        rive::Mat2D transform = rive::computeAlignment(
            rive::Fit::contain,
            rive::Alignment::center,
            rive::AABB(0, 0, m_renderWidth, m_renderHeight),
            m_artboard->bounds()
        );
        
        // Render
        m_riveRenderer->save();
        m_riveRenderer->transform(transform);
        m_scene->draw(m_riveRenderer.get());
        m_riveRenderer->restore();
        
        // Flush and present
        auto result = m_riveRenderTarget.get();
        m_riveRenderContext->flush({.renderTarget = result});
        m_riveRenderTarget->setTargetTexture(nullptr);
    }
    else
#endif
    {
        // Fallback: clear to test color if no Rive content
        winrt::com_ptr<::ID3D11Texture2D> backbuffer;
        winrt::check_hresult(m_swapChain->GetBuffer(0, IID_PPV_ARGS(backbuffer.put())));
        
        winrt::com_ptr<::ID3D11RenderTargetView> renderTargetView;
        winrt::check_hresult(m_d3dDevice->CreateRenderTargetView(
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

bool RiveRenderer::CheckDeviceLost()
{
    if (!m_d3dDevice) return true;
    
    HRESULT hr = m_d3dDevice->GetDeviceRemovedReason();
    return FAILED(hr);
}

void RiveRenderer::HandleDeviceLost()
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

void RiveRenderer::PauseRendering()
{
    m_isPaused = true;
}

void RiveRenderer::ResumeRendering()
{
    m_isPaused = false;
}

// Input handling - coordinates should be relative to renderer bounds
void RiveRenderer::QueuePointerMove(float x, float y)
{
    std::lock_guard<std::mutex> lock(m_inputQueueMutex);
    MouseInputEvent event;
    event.type = MouseInputEvent::Move;
    event.x = x;
    event.y = y;
    event.timestamp = std::chrono::steady_clock::now();
    m_inputQueue.push(event);
}

void RiveRenderer::QueuePointerPress(float x, float y)
{
    std::lock_guard<std::mutex> lock(m_inputQueueMutex);
    MouseInputEvent event;
    event.type = MouseInputEvent::Press;
    event.x = x;
    event.y = y;
    event.timestamp = std::chrono::steady_clock::now();
    m_inputQueue.push(event);
}

void RiveRenderer::QueuePointerRelease(float x, float y)
{
    std::lock_guard<std::mutex> lock(m_inputQueueMutex);
    MouseInputEvent event;
    event.type = MouseInputEvent::Release;
    event.x = x;
    event.y = y;
    event.timestamp = std::chrono::steady_clock::now();
    m_inputQueue.push(event);
}

// Input processing
void RiveRenderer::ProcessInputQueue()
{
    std::lock_guard<std::mutex> lock(m_inputQueueMutex);
    
    while (!m_inputQueue.empty()) {
        MouseInputEvent event = m_inputQueue.front();
        m_inputQueue.pop();
        
        // Transform coordinates to artboard space
        float artboardX = event.x;
        float artboardY = event.y;
        
        if (TransformToArtboardSpace(artboardX, artboardY)) {
            // Forward to state machine if available
            bool isDown = (event.type == MouseInputEvent::Press) ? true : 
                         (event.type == MouseInputEvent::Release) ? false : m_lastPointerDown;
            
            ForwardPointerEventToStateMachine(artboardX, artboardY, isDown);
            
            // Update pointer state tracking
            if (event.type == MouseInputEvent::Press) {
                m_lastPointerDown = true;
            } else if (event.type == MouseInputEvent::Release) {
                m_lastPointerDown = false;
            }
        }
    }
}

void RiveRenderer::ForwardPointerEventToStateMachine(float x, float y, bool isDown)
{
#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
    if (m_scene) {
        // Check if current scene is a state machine
        auto stateMachine = dynamic_cast<rive::StateMachineInstance*>(m_scene.get());
        if (stateMachine) {
            // Forward pointer event to state machine
            stateMachine->pointerMove(rive::Vec2D(x, y));
            if (isDown) {
                stateMachine->pointerDown(rive::Vec2D(x, y));
            } else {
                stateMachine->pointerUp(rive::Vec2D(x, y));
            }
        }
    }
#endif
}

// Coordinate transformation
bool RiveRenderer::TransformToArtboardSpace(float& x, float& y)
{
#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
    // Only attempt transformation if we have a valid artboard
    if (!m_artboard) {
        return false;
    }
    
    // Update alignment if needed
    if (!m_transformValid) {
        UpdateArtboardAlignment();
    }
    
    // Only proceed if we have a valid transform
    if (m_transformValid) {
        // Apply inverse transform to convert from renderer space to artboard space
        rive::Mat2D inverseTransform = m_artboardTransform.invertOrIdentity();
        rive::Vec2D point = inverseTransform * rive::Vec2D(x, y);
        x = point.x;
        y = point.y;
        return true;
    }
#endif
    
    return false;
}

void RiveRenderer::UpdateArtboardAlignment()
{
#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
    if (m_artboard) {
        // Calculate transform to fit artboard within renderer bounds
        m_artboardTransform = rive::computeAlignment(
            rive::Fit::contain,
            rive::Alignment::center,
            rive::AABB(0, 0, static_cast<float>(m_renderWidth), static_cast<float>(m_renderHeight)),
            m_artboard->bounds()
        );
        m_transformValid = true;
    } else {
        m_transformValid = false;
    }
#else
    m_transformValid = false;
#endif
}

void RiveRenderer::CleanupDeviceResources()
{
    m_backBuffer = nullptr;
    m_swapChain = nullptr;
    m_dxgiFactory = nullptr;
    m_d3dContext = nullptr;
    m_d3dDevice = nullptr;
}

void RiveRenderer::CleanupRenderingResources()
{
#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
    m_riveRenderer = nullptr;
    m_riveRenderTarget = nullptr;
    m_riveRenderContext = nullptr;
    m_viewModelInstance = nullptr;
    m_scene = nullptr;
    m_artboard = nullptr;
    m_riveFile = nullptr;
#endif
    m_riveFileData.clear();
    m_riveFilePath.clear();
}
