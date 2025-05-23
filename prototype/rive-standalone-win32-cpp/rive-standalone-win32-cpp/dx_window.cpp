#include "pch.h"
#include "dx_window.h"

DXWindow::DXWindow()
{
}

DXWindow::~DXWindow()
{
    // Clean up any DXWindow specific resources here if needed
}

void DXWindow::WindowCreated() {
    m_controller = CreateDispatcherQueueCont();
	PrepareVisuals();
}

void DXWindow::OnPointerDown(int x, int y) {
    // TODO: Implement DXWindow specific OnPointerDown
    Win32Window::OnPointerDown(x, y); // Call base class implementation if needed
}

void DXWindow::OnDpiChanged(int dpi) {
    // TODO: Implement DXWindow specific OnDpiChanged
    Win32Window::OnDpiChanged(dpi); // Call base class implementation if needed
}

void DXWindow::OnResize(int width, int height) {
    // TODO: Implement DXWindow specific OnResize
    Win32Window::OnResize(width, height); // Call base class implementation if needed
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
        Compositor compositor;
        m_target = CreateDesktopWindowTarget(compositor,window_handle_);
        auto root = compositor.CreateSpriteVisual();
        root.RelativeSizeAdjustment({ 1.0f, 1.0f });
        root.Brush(compositor.CreateColorBrush({ 0xFF, 0xEF, 0xE4 , 0xB0 }));
        m_target.Root(root);
        auto visuals = root.Children();

        AddVisual(visuals, 100.0f, 100.0f);
        AddVisual(visuals, 220.0f, 100.0f);
        AddVisual(visuals, 100.0f, 220.0f);
        AddVisual(visuals, 220.0f, 220.0f);
    }

    void DXWindow::AddVisual(VisualCollection const& visuals, float x, float y)
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