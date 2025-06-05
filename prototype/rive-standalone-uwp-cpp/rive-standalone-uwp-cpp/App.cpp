#include "pch.h"

using namespace winrt;

using namespace Windows;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;

struct App : implements<App, IFrameworkViewSource, IFrameworkView>
{
    Compositor m_compositor{ nullptr };
    CompositionTarget m_target{ nullptr };
    ContainerVisual m_root{ nullptr };
    SpriteVisual m_rectangleVisual{ nullptr };

    IFrameworkView CreateView()
    {
        return *this;
    }

    void Initialize(CoreApplicationView const &)
    {
    }

    void Load(hstring const&)
    {
    }

    void Uninitialize()
    {
    }

    void Run()
    {
        CoreWindow window = CoreWindow::GetForCurrentThread();
        window.Activate();

        CoreDispatcher dispatcher = window.Dispatcher();
        dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
    }

    void SetWindow(CoreWindow const & window)
    {
        // Initialize Composition
        PrepareVisuals();

        // Handle window size changes
        window.SizeChanged({ this, &App::OnSizeChanged });
    }

    void PrepareVisuals()
    {
        // Create compositor
        m_compositor = Compositor();

        // Create target for current view - this is the UWP equivalent of CreateDesktopWindowTarget
        m_target = m_compositor.CreateTargetForCurrentView();

        // Create root container visual
        m_root = m_compositor.CreateContainerVisual();
        m_root.RelativeSizeAdjustment({ 1.0f, 1.0f });

        // Set root as the target
        m_target.Root(m_root);

        // Create a simple colored rectangle
        CreateRectangleVisual();
    }

    void CreateRectangleVisual()
    {
        // Create a sprite visual for the rectangle
        m_rectangleVisual = m_compositor.CreateSpriteVisual();

        // Set size to 200x200 pixels
        m_rectangleVisual.Size({ 200.0f, 200.0f });

        // Position it in the center (will be updated when window size is known)
        m_rectangleVisual.Offset({ 100.0f, 100.0f, 0.0f });

        // Create a solid color brush - using a nice blue color
        auto colorBrush = m_compositor.CreateColorBrush({ 0xFF, 0x41, 0x69, 0xE1 }); // Royal Blue
        m_rectangleVisual.Brush(colorBrush);

        // Add to the root visual
        m_root.Children().InsertAtTop(m_rectangleVisual);
    }

    void OnSizeChanged(CoreWindow const& sender, WindowSizeChangedEventArgs const& args)
    {
        if (m_rectangleVisual)
        {
            // Center the rectangle in the window
            float windowWidth = args.Size().Width;
            float windowHeight = args.Size().Height;
            
            float rectWidth = 200.0f;
            float rectHeight = 200.0f;
            
            float centerX = (windowWidth - rectWidth) / 2.0f;
            float centerY = (windowHeight - rectHeight) / 2.0f;
            
            m_rectangleVisual.Offset({ centerX, centerY, 0.0f });
        }
    }
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    CoreApplication::Run(make<App>());
}
