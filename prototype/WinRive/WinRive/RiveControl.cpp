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
            return m_riveRenderer->Initialize(compositor, width, height);
        }
        return false;
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
        if (m_riveRenderer)
        {
            m_riveRenderer->SetSize(width, height);
        }
    }

    void RiveControl::Shutdown()
    {
        if (m_riveRenderer)
        {
            m_riveRenderer->StopRenderThread();
            m_riveRenderer->Shutdown();
            m_riveRenderer.reset();
        }
    }
}
