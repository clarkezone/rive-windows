#include "pch.h"
#include "../../shared/rive_renderer.h"

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
    std::unique_ptr<RiveRenderer> m_riveRenderer;

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
        if (m_riveRenderer) {
            m_riveRenderer->StopRenderThread();
            m_riveRenderer->Shutdown();
            m_riveRenderer.reset();
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

        // Initialize RiveRenderer
        InitializeRiveRenderer();
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

    void InitializeRiveRenderer()
    {
        try {
            // Create RiveRenderer instance
            m_riveRenderer = std::make_unique<RiveRenderer>();
            
            // Get initial window size (use default if not available yet)
            CoreWindow window = CoreWindow::GetForCurrentThread();
            auto bounds = window.Bounds();
            int width = static_cast<int>(bounds.Width);
            int height = static_cast<int>(bounds.Height);
            
            // Use reasonable defaults if window size is not available
            if (width <= 0) width = 800;
            if (height <= 0) height = 600;
            
            // Initialize the renderer
            if (m_riveRenderer->Initialize(m_compositor, width, height)) {
                // Get the visual from the renderer and add it to our root
                auto riveVisual = m_riveRenderer->GetVisual();
                if (riveVisual) {
                    // Position the Rive visual next to the colored rectangle
                    riveVisual.Offset({ 300.0f, 50.0f, 0.0f });
                    
                    m_root.Children().InsertAtTop(riveVisual);
                    
                    // Load the packaged Rive file
                    auto package = winrt::Windows::ApplicationModel::Package::Current();
                    auto installFolder = package.InstalledLocation();
                    auto rivePath = installFolder.Path() + L"\\meeting_ui.riv";
                    std::string rivePathStr = winrt::to_string(rivePath);
                    
                    if (m_riveRenderer->LoadRiveFile(rivePathStr)) {
                        std::cout << "Successfully loaded packaged Rive file: " << rivePathStr << "\n";
                    } else {
                        std::cout << "Failed to load packaged Rive file: " << rivePathStr << "\n";
                    }
                    
                    // Start the renderer's animation/tick
                    m_riveRenderer->StartRenderThread();
                    
                    std::cout << "RiveRenderer initialized and started animation\n";
                }
            } else {
                std::cout << "Failed to initialize RiveRenderer\n";
            }
        }
        catch (winrt::hresult_error const& ex) {
            std::wcout << L"Failed to create RiveRenderer: " << ex.message().c_str() << L"\n";
        }
    }

    void OnSizeChanged(CoreWindow const& sender, WindowSizeChangedEventArgs const& args)
    {
        if (m_riveRenderer) {
            // Update the renderer size
            int width = static_cast<int>(args.Size().Width);
            int height = static_cast<int>(args.Size().Height);
            m_riveRenderer->SetSize(width, height);
        }
    }
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    CoreApplication::Run(make<App>());
}
