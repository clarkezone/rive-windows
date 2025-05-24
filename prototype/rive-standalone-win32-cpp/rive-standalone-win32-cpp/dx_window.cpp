#include "pch.h"
#include "dx_window.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

DXWindow::DXWindow()
{
}

DXWindow::~DXWindow()
{
    StopRenderThread();
    CleanupRenderingResources();
    CleanupDeviceResources();
}

void DXWindow::WindowCreated() {
    std::cout << "DXWindow::WindowCreated() called\n";
    
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
        
        StartRenderThread();
        std::cout << "Render thread started\n";
    }
    catch (winrt::hresult_error const& ex) {
        std::wcout << L"Failed to initialize DirectX: " << ex.message().c_str() << L" (0x" << std::hex << ex.code() << L")\n";
    }
}

void DXWindow::OnPointerDown(int x, int y) {
    Win32Window::OnPointerDown(x, y);
}

void DXWindow::OnDpiChanged(int dpi) {
    Win32Window::OnDpiChanged(dpi);
    // Handle DPI change for DirectX resources if needed
}

void DXWindow::OnResize(int width, int height) {
    Win32Window::OnResize(width, height);
    
    std::lock_guard<std::mutex> lock(m_deviceMutex);
    
    if (width > 0 && height > 0 && (width != m_windowWidth || height != m_windowHeight)) {
        m_windowWidth = width;
        m_windowHeight = height;
        
        // Resize the swap chain
        if (m_swapChain) {
            m_d2dContext->SetTarget(nullptr);
            m_d2dTargetBitmap = nullptr;
            m_backBuffer = nullptr;
            
            HRESULT hr = m_swapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
            if (SUCCEEDED(hr)) {
                CreateRenderTarget();
            }
        }
    }
}

Windows::System::DispatcherQueueController DXWindow::CreateDispatcherQueueCont()
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

void DXWindow::PrepareVisuals()
{
    m_compositor = Compositor();
    m_target = CreateDesktopWindowTarget(m_compositor, window_handle_);
    m_root = m_compositor.CreateContainerVisual();
    m_root.RelativeSizeAdjustment({ 1.0f, 1.0f });
    m_target.Root(m_root);
}

void DXWindow::CreateCompositionSurface()
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

void DXWindow::CreateDeviceResources()
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

    // Create D2D factory
    check_hresult(D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        IID_PPV_ARGS(m_d2dFactory.put())
    ));

    // Create D2D device
    winrt::com_ptr<::IDXGIDevice> dxgiDevice;
    check_hresult(m_d3dDevice->QueryInterface(IID_PPV_ARGS(dxgiDevice.put())));
    check_hresult(m_d2dFactory->CreateDevice(dxgiDevice.get(), m_d2dDevice.put()));
    check_hresult(m_d2dDevice->CreateDeviceContext(
        D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
        m_d2dContext.put()
    ));

    // Create DirectWrite factory
    check_hresult(DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(m_dwriteFactory.put())
    ));

    // Create text format
    check_hresult(m_dwriteFactory->CreateTextFormat(
        L"Segoe UI",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        32.0f,
        L"en-us",
        m_textFormat.put()
    ));

    // Create swap chain
    CreateSwapChain();

    // Create render target
    CreateRenderTarget();
}

void DXWindow::CreateSwapChain()
{
    winrt::com_ptr<::IDXGIDevice1> dxgiDevice;
    check_hresult(m_d3dDevice->QueryInterface(IID_PPV_ARGS(dxgiDevice.put())));

    winrt::com_ptr<::IDXGIAdapter> adapter;
    check_hresult(dxgiDevice->GetAdapter(adapter.put()));

    winrt::com_ptr<::IDXGIFactory2> factory;
    check_hresult(adapter->GetParent(IID_PPV_ARGS(factory.put())));

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

    check_hresult(factory->CreateSwapChainForComposition(
        m_d3dDevice.get(),
        &swapChainDesc,
        nullptr,
        m_swapChain.put()
    ));
}

void DXWindow::CreateRenderTarget()
{
    if (!m_swapChain) return;

    check_hresult(m_swapChain->GetBuffer(0, IID_PPV_ARGS(m_backBuffer.put())));

    winrt::com_ptr<::IDXGISurface> dxgiBackBuffer;
    check_hresult(m_backBuffer->QueryInterface(IID_PPV_ARGS(dxgiBackBuffer.put())));

    D2D1_BITMAP_PROPERTIES1 bitmapProperties = {};
    bitmapProperties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
    bitmapProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
    bitmapProperties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;

    check_hresult(m_d2dContext->CreateBitmapFromDxgiSurface(
        dxgiBackBuffer.get(),
        &bitmapProperties,
        m_d2dTargetBitmap.put()
    ));

    m_d2dContext->SetTarget(m_d2dTargetBitmap.get());

    // Create brushes
    check_hresult(m_d2dContext->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::White),
        m_clockBrush.put()
    ));

    check_hresult(m_d2dContext->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::DarkBlue),
        m_backgroundBrush.put()
    ));

    check_hresult(m_d2dContext->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::Yellow),
        m_textBrush.put()
    ));
}

void DXWindow::RecreateDeviceResources()
{
    CleanupRenderingResources();
    CleanupDeviceResources();
    
    CreateDeviceResources();
    CreateCompositionSurface();
}

void DXWindow::StartRenderThread()
{
    m_shouldRender = true;
    m_isPaused = false;
    m_renderThread = std::thread(&DXWindow::RenderLoop, this);
}

void DXWindow::StopRenderThread()
{
    m_shouldRender = false;
    if (m_renderThread.joinable()) {
        m_renderThread.join();
    }
}

void DXWindow::RenderLoop()
{
    while (m_shouldRender) {
        if (!m_isPaused && !m_deviceLost) {
            std::lock_guard<std::mutex> lock(m_deviceMutex);
            
            if (CheckDeviceLost()) {
                HandleDeviceLost();
                continue;
            }
            
            RenderClock();
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }
}

void DXWindow::RenderClock()
{
    if (!m_d2dContext || !m_d2dTargetBitmap) return;

    m_d2dContext->BeginDraw();
    m_d2dContext->Clear(D2D1::ColorF(D2D1::ColorF::DarkBlue, 1.0f));

    float centerX = static_cast<float>(m_windowWidth) / 2.0f;
    float centerY = static_cast<float>(m_windowHeight) / 2.0f;
    float radius = (centerX < centerY ? centerX : centerY) * 0.8f;

    // Draw clock face
    DrawClockFace(centerX, centerY, radius);

    // Get current time
    auto now = std::chrono::system_clock::now();
    auto currentTime = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    struct tm timeinfo;
    localtime_s(&timeinfo, &currentTime);

    float hours = static_cast<float>(timeinfo.tm_hour % 12);
    float minutes = static_cast<float>(timeinfo.tm_min);
    float seconds = static_cast<float>(timeinfo.tm_sec) + static_cast<float>(ms.count()) / 1000.0f;

    // Calculate angles (12 o'clock is 0 degrees)
    float hourAngle = (hours + minutes / 60.0f) * 30.0f - 90.0f; // 30 degrees per hour
    float minuteAngle = (minutes + seconds / 60.0f) * 6.0f - 90.0f; // 6 degrees per minute
    float secondAngle = seconds * 6.0f - 90.0f; // 6 degrees per second

    // Convert to radians
    hourAngle = hourAngle * (float)M_PI / 180.0f;
    minuteAngle = minuteAngle * (float)M_PI / 180.0f;
    secondAngle = secondAngle * (float)M_PI / 180.0f;

    // Draw hands
    DrawClockHand(centerX, centerY, hourAngle, radius * 0.5f, 6.0f);
    DrawClockHand(centerX, centerY, minuteAngle, radius * 0.7f, 4.0f);
    DrawClockHand(centerX, centerY, secondAngle, radius * 0.9f, 2.0f);

    // Draw center dot
    m_d2dContext->FillEllipse(
        D2D1::Ellipse(D2D1::Point2F(centerX, centerY), 8.0f, 8.0f),
        m_clockBrush.get()
    );

    // Draw digital time
    DrawDigitalTime(centerX, centerY + radius + 50);

    winrt::hresult hr = m_d2dContext->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        m_deviceLost = true;
    } else if (SUCCEEDED(hr)) {
        m_swapChain->Present(1, 0);
    }
}

void DXWindow::DrawClockFace(float centerX, float centerY, float radius)
{
    // Draw outer circle
    m_d2dContext->DrawEllipse(
        D2D1::Ellipse(D2D1::Point2F(centerX, centerY), radius, radius),
        m_clockBrush.get(),
        3.0f
    );

    // Draw hour markers
    for (int i = 0; i < 12; i++) {
        float angle = i * 30.0f * (float)M_PI / 180.0f;
        float innerRadius = radius * 0.9f;
        float outerRadius = radius * 0.95f;
        
        float x1 = centerX + cosf(angle) * innerRadius;
        float y1 = centerY + sinf(angle) * innerRadius;
        float x2 = centerX + cosf(angle) * outerRadius;
        float y2 = centerY + sinf(angle) * outerRadius;
        
        m_d2dContext->DrawLine(
            D2D1::Point2F(x1, y1),
            D2D1::Point2F(x2, y2),
            m_clockBrush.get(),
            3.0f
        );
    }

    // Draw minute markers
    for (int i = 0; i < 60; i++) {
        if (i % 5 != 0) { // Skip hour markers
            float angle = i * 6.0f * (float)M_PI / 180.0f;
            float innerRadius = radius * 0.95f;
            float outerRadius = radius * 0.98f;
            
            float x1 = centerX + cosf(angle) * innerRadius;
            float y1 = centerY + sinf(angle) * innerRadius;
            float x2 = centerX + cosf(angle) * outerRadius;
            float y2 = centerY + sinf(angle) * outerRadius;
            
            m_d2dContext->DrawLine(
                D2D1::Point2F(x1, y1),
                D2D1::Point2F(x2, y2),
                m_clockBrush.get(),
                1.0f
            );
        }
    }
}

void DXWindow::DrawClockHand(float centerX, float centerY, float angle, float length, float thickness)
{
    float endX = centerX + cosf(angle) * length;
    float endY = centerY + sinf(angle) * length;
    
    m_d2dContext->DrawLine(
        D2D1::Point2F(centerX, centerY),
        D2D1::Point2F(endX, endY),
        m_clockBrush.get(),
        thickness
    );
}

void DXWindow::DrawDigitalTime(float x, float y)
{
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    
    struct tm timeinfo;
    localtime_s(&timeinfo, &time_t_now);
    
    wchar_t timeString[32];
    swprintf_s(timeString, L"%02d:%02d:%02d", 
        timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    
    winrt::com_ptr<::IDWriteTextLayout> textLayout;
    winrt::hresult hr = m_dwriteFactory->CreateTextLayout(
        timeString,
        static_cast<UINT32>(wcslen(timeString)),
        m_textFormat.get(),
        200.0f,
        50.0f,
        textLayout.put()
    );
    
    if (SUCCEEDED(hr) && textLayout) {
        m_d2dContext->DrawTextLayout(
            D2D1::Point2F(x - 100, y),
            textLayout.get(),
            m_textBrush.get()
        );
    }
}

bool DXWindow::CheckDeviceLost()
{
    if (!m_d3dDevice) return true;
    
    HRESULT hr = m_d3dDevice->GetDeviceRemovedReason();
    return FAILED(hr);
}

void DXWindow::HandleDeviceLost()
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

void DXWindow::PauseRendering()
{
    m_isPaused = true;
}

void DXWindow::ResumeRendering()
{
    m_isPaused = false;
}

void DXWindow::CleanupDeviceResources()
{
    m_d2dTargetBitmap = nullptr;
    m_backBuffer = nullptr;
    m_swapChain = nullptr;
    m_d3dContext = nullptr;
    m_d3dDevice = nullptr;
}

void DXWindow::CleanupRenderingResources()
{
    m_textBrush = nullptr;
    m_backgroundBrush = nullptr;
    m_clockBrush = nullptr;
    m_textFormat = nullptr;
    m_dwriteFactory = nullptr;
    m_d2dContext = nullptr;
    m_d2dDevice = nullptr;
    m_d2dFactory = nullptr;
}
