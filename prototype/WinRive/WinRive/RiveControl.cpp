#include "pch.h"
#include "RiveControl.h"
#include "RiveControl.g.cpp"

namespace winrt::WinRive::implementation
{
    RiveControl::RiveControl() : m_riveRenderer(std::make_unique<RiveRenderer>())
    {
    }

    RiveControl::~RiveControl()
    {
        Shutdown();
    }

    bool RiveControl::Initialize(winrt::Windows::UI::Composition::Compositor const& compositor, int32_t width, int32_t height)
    {
        if (m_riveRenderer)
        {
            m_width = width;
            m_height = height;
            return m_riveRenderer->Initialize(compositor, width, height);
        }
        return false;
    }

    bool RiveControl::InitializeWithCoreWindow(winrt::Windows::UI::Composition::Compositor const& compositor,
                                              winrt::Windows::UI::Core::CoreWindow const& window,
                                              int32_t width, int32_t height)
    {
        if (!Initialize(compositor, width, height))
        {
            return false;
        }

        // Store the CoreWindow reference
        m_coreWindow = window;

        // Register for pointer events
        if (m_coreWindow)
        {
            m_pointerMovedToken = m_coreWindow.PointerMoved({ this, &RiveControl::OnPointerMoved });
            m_pointerPressedToken = m_coreWindow.PointerPressed({ this, &RiveControl::OnPointerPressed });
            m_pointerReleasedToken = m_coreWindow.PointerReleased({ this, &RiveControl::OnPointerReleased });
            
            OutputDebugStringW(L"RiveControl: Registered for CoreWindow pointer events\n");
        }

        return true;
    }

    winrt::Windows::UI::Composition::Visual RiveControl::GetVisual()
    {
        if (m_riveRenderer)
        {
            return m_riveRenderer->GetVisual();
        }
        return nullptr;
    }

    bool RiveControl::LoadRiveFile(hstring const& filePath)
    {
        if (m_riveRenderer)
        {
            return m_riveRenderer->LoadRiveFile(winrt::to_string(filePath));
        }
        return false;
    }

    bool RiveControl::LoadRiveFileFromPackage(hstring const& relativePath)
    {
        try
        {
            auto package = winrt::Windows::ApplicationModel::Package::Current();
            auto installFolder = package.InstalledLocation();
            auto fullPath = installFolder.Path() + L"\\Assets\\RiveAssets\\" + relativePath;
            return LoadRiveFile(fullPath);
        }
        catch (winrt::hresult_error const& ex)
        {
            OutputDebugStringW((L"Failed to load Rive file from package: " + ex.message() + L"\n").c_str());
            return false;
        }
    }

    void RiveControl::StartRenderLoop()
    {
        if (m_riveRenderer)
        {
            m_riveRenderer->StartRenderThread();
        }
    }

    void RiveControl::StopRenderLoop()
    {
        if (m_riveRenderer)
        {
            m_riveRenderer->StopRenderThread();
        }
    }

    void RiveControl::PauseRendering()
    {
        if (m_riveRenderer)
        {
            m_riveRenderer->PauseRendering();
        }
    }

    void RiveControl::ResumeRendering()
    {
        if (m_riveRenderer)
        {
            m_riveRenderer->ResumeRendering();
        }
    }

    void RiveControl::SetSize(int32_t width, int32_t height)
    {
        m_width = width;
        m_height = height;
        if (m_riveRenderer)
        {
            m_riveRenderer->SetSize(width, height);
        }
    }

    void RiveControl::Shutdown()
    {
        // Unregister event handlers
        if (m_coreWindow)
        {
            m_coreWindow.PointerMoved(m_pointerMovedToken);
            m_coreWindow.PointerPressed(m_pointerPressedToken);
            m_coreWindow.PointerReleased(m_pointerReleasedToken);
            m_coreWindow = nullptr;
        }

        if (m_riveRenderer)
        {
            m_riveRenderer->StopRenderThread();
            m_riveRenderer->Shutdown();
            m_riveRenderer.reset();
        }
    }

    void RiveControl::OnPointerMoved(winrt::Windows::UI::Core::CoreWindow const& sender,
                                   winrt::Windows::UI::Core::PointerEventArgs const& args)
    {
        auto point = args.CurrentPoint().Position();
        
        // Get the visual to calculate relative position
        auto visual = GetVisual();
        if (!visual)
            return;

        // For now, we'll use the raw coordinates
        // In a full implementation, we'd transform these to visual space
        if (IsPointInBounds(point))
        {
            wchar_t debugMsg[256];
            swprintf_s(debugMsg, L"RiveControl: Mouse moved to (%.1f, %.1f)\n", point.X, point.Y);
            OutputDebugStringW(debugMsg);

            // TODO: Forward to RiveRenderer for interaction handling
            // m_riveRenderer->HandlePointerMove(point.X, point.Y);
        }
    }

    void RiveControl::OnPointerPressed(winrt::Windows::UI::Core::CoreWindow const& sender,
                                     winrt::Windows::UI::Core::PointerEventArgs const& args)
    {
        auto point = args.CurrentPoint().Position();
        
        if (IsPointInBounds(point))
        {
            wchar_t debugMsg[256];
            swprintf_s(debugMsg, L"RiveControl: Mouse pressed at (%.1f, %.1f)\n", point.X, point.Y);
            OutputDebugStringW(debugMsg);

            // TODO: Forward to RiveRenderer
            // m_riveRenderer->HandlePointerPress(point.X, point.Y);
        }
    }

    void RiveControl::OnPointerReleased(winrt::Windows::UI::Core::CoreWindow const& sender,
                                       winrt::Windows::UI::Core::PointerEventArgs const& args)
    {
        auto point = args.CurrentPoint().Position();
        
        if (IsPointInBounds(point))
        {
            wchar_t debugMsg[256];
            swprintf_s(debugMsg, L"RiveControl: Mouse released at (%.1f, %.1f)\n", point.X, point.Y);
            OutputDebugStringW(debugMsg);

            // TODO: Forward to RiveRenderer
            // m_riveRenderer->HandlePointerRelease(point.X, point.Y);
        }
    }

    bool RiveControl::IsPointInBounds(winrt::Windows::Foundation::Point const& point)
    {
        // Simple bounds check - in a real implementation, we'd need to consider
        // the visual's position relative to the window
        return point.X >= 0 && point.X <= m_width &&
               point.Y >= 0 && point.Y <= m_height;
    }
}
