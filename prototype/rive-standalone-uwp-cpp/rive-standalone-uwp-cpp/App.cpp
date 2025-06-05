#include "pch.h"
#include "../../shared/dx_renderer.h"

using namespace winrt;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::UI;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::Composition;

struct App : implements<App, IFrameworkViewSource, IFrameworkView>
{
    Compositor m_compositor{ nullptr };
    CompositionTarget m_target{ nullptr };
    ContainerVisual m_root{ nullptr };
    SpriteVisual m_coloredRectangle{ nullptr };
    std::unique_ptr<DXRenderer> m_dxRenderer;

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
        // Clean up renderer
        if (m_dxRenderer) {
            m_dxRenderer->StopRenderThread();
            m_dxRenderer->Shutdown();
            m_dxRenderer.reset();
        }
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

        // Create a simple colored rectangle visual
        CreateColoredRectangle();

        // Initialize DXRenderer
        InitializeDXRenderer();
    }

    void CreateColoredRectangle()
    {
        // Create a sprite visual for a simple colored rectangle
        m_coloredRectangle = m_compositor.CreateSpriteVisual();
        
        // Set size and position
        m_coloredRectangle.Size({ 200.0f, 150.0f });
        m_coloredRectangle.Offset({ 50.0f, 50.0f, 0.0f });
        
        // Create a colored brush (bright blue)
        auto colorBrush = m_compositor.CreateColorBrush(Colors::DeepSkyBlue());
        m_coloredRectangle.Brush(colorBrush);
        
        // Add to the visual tree
        m_root.Children().InsertAtTop(m_coloredRectangle);
        
        std::cout << "Created simple colored rectangle visual\n";
    }

    void InitializeDXRenderer()
    {
        try {
            // Create DXRenderer instance
            m_dxRenderer = std::make_unique<DXRenderer>();
            
            // Get initial window size (use default if not available yet)
            CoreWindow window = CoreWindow::GetForCurrentThread();
            auto bounds = window.Bounds();
            int width = static_cast<int>(bounds.Width);
            int height = static_cast<int>(bounds.Height);
            
            // Use reasonable defaults if window size is not available
            if (width <= 0) width = 800;
            if (height <= 0) height = 600;
            
            // Initialize the renderer
            if (m_dxRenderer->Initialize(m_compositor, width, height)) {
                // Get the visual from the renderer and add it to our root
                auto dxVisual = m_dxRenderer->GetVisual();
                if (dxVisual) {
                    // Position the DX visual next to the colored rectangle
                    dxVisual.Offset({ 300.0f, 50.0f, 0.0f });
                    
                    m_root.Children().InsertAtTop(dxVisual);
                    
                    // Start the renderer's animation/tick
                    m_dxRenderer->StartRenderThread();
                    
                    std::cout << "DXRenderer initialized and started animation\n";
                }
            } else {
                std::cout << "Failed to initialize DXRenderer\n";
            }
        }
        catch (winrt::hresult_error const& ex) {
            std::wcout << L"Failed to create DXRenderer: " << ex.message().c_str() << L"\n";
        }
    }

    void OnSizeChanged(CoreWindow const& sender, WindowSizeChangedEventArgs const& args)
    {
        if (m_dxRenderer) {
            // Update the renderer size
            int width = static_cast<int>(args.Size().Width);
            int height = static_cast<int>(args.Size().Height);
            m_dxRenderer->SetSize(width, height);
        }
    }
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    CoreApplication::Run(make<App>());
}
