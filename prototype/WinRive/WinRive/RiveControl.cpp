#include "pch.h"
#include "RiveControl.h"
#include "RiveControl.g.cpp"

namespace winrt::WinRive::implementation
{
    RiveControl::RiveControl()
    {
        m_riveRenderer = std::make_unique<RiveRenderer>();
        m_hostingMode = winrt::WinRive::HostingMode::UWP_CoreWindow;
    }

    RiveControl::~RiveControl()
    {
        Shutdown();
    }

    bool RiveControl::Initialize(winrt::Windows::UI::Composition::Compositor const& compositor, int32_t width, int32_t height)
    {
        m_hostingMode = winrt::WinRive::HostingMode::WinUI3_Compositor;
        return InitializeCommon(compositor, width, height);
    }

    bool RiveControl::InitializeWithCoreWindow(winrt::Windows::UI::Composition::Compositor const& compositor,
                                              winrt::Windows::UI::Core::CoreWindow const& window,
                                              int32_t width, int32_t height)
    {
        // Legacy method - redirect to new UWP method for backward compatibility
        return InitializeForUWP(compositor, window, width, height);
    }

    bool RiveControl::InitializeForUWP(winrt::Windows::UI::Composition::Compositor const& compositor,
                                       winrt::Windows::UI::Core::CoreWindow const& window,
                                       int32_t width, int32_t height)
    {
        m_hostingMode = winrt::WinRive::HostingMode::UWP_CoreWindow;
        
        if (!InitializeCommon(compositor, width, height))
        {
            return false;
        }

        // Store CoreWindow for backward compatibility
        m_coreWindow = window;
        if (m_coreWindow)
        {
            m_pointerMovedToken = m_coreWindow.PointerMoved({ this, &RiveControl::OnPointerMoved });
            m_pointerPressedToken = m_coreWindow.PointerPressed({ this, &RiveControl::OnPointerPressed });
            m_pointerReleasedToken = m_coreWindow.PointerReleased({ this, &RiveControl::OnPointerReleased });
            
            OutputDebugStringW(L"RiveControl: Initialized for UWP with CoreWindow\n");
        }

        return true;
    }

    bool RiveControl::InitializeForWinUI3(winrt::Windows::UI::Composition::Compositor const& compositor,
                                          int32_t width, int32_t height)
    {
        m_hostingMode = winrt::WinRive::HostingMode::WinUI3_Compositor;
        
        if (!InitializeCommon(compositor, width, height))
        {
            return false;
        }

        OutputDebugStringW(L"RiveControl: Initialized for WinUI3\n");
        return true;
    }

    bool RiveControl::InitializeForWin32(winrt::Windows::UI::Composition::Compositor const& compositor,
                                         uint64_t hwnd,
                                         int32_t width, int32_t height)
    {
        m_hostingMode = winrt::WinRive::HostingMode::Win32_HWND;
        
        if (!InitializeCommon(compositor, width, height))
        {
            return false;
        }

        // Store HWND for Win32 hosting
        m_hwnd = reinterpret_cast<HWND>(hwnd);

        OutputDebugStringW(L"RiveControl: Initialized for Win32\n");
        return true;
    }

    winrt::WinRive::HostingMode RiveControl::GetHostingMode()
    {
        return m_hostingMode;
    }

    bool RiveControl::InitializeCommon(winrt::Windows::UI::Composition::Compositor const& compositor, int32_t width, int32_t height)
    {
        if (m_riveRenderer)
        {
            m_width = width;
            m_height = height;
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
        m_width = width;
        m_height = height;
        if (m_riveRenderer)
        {
            m_riveRenderer->SetSize(width, height);
        }
        
        // TODO: Update input provider bounds if available in future
    }

    void RiveControl::Shutdown()
    {
        CleanupInput();

        if (m_riveRenderer)
        {
            m_riveRenderer->StopRenderThread();
            m_riveRenderer->Shutdown();
            m_riveRenderer.reset();
        }
    }

    void RiveControl::CleanupInput()
    {
        // Cleanup legacy CoreWindow handlers
        if (m_coreWindow)
        {
            m_coreWindow.PointerMoved(m_pointerMovedToken);
            m_coreWindow.PointerPressed(m_pointerPressedToken);
            m_coreWindow.PointerReleased(m_pointerReleasedToken);
            m_coreWindow = nullptr;
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

        // Transform coordinates to renderer bounds (0,0 to width,height)
        if (IsPointInBounds(point))
        {
            // Convert CoreWindow coordinates to RiveRenderer bounds
            float rendererX = static_cast<float>(point.X);
            float rendererY = static_cast<float>(point.Y);
            
            // Forward to RiveRenderer input queue
            if (m_riveRenderer) {
                m_riveRenderer->QueuePointerMove(rendererX, rendererY);
            }
        }
    }

    void RiveControl::OnPointerPressed(winrt::Windows::UI::Core::CoreWindow const& sender,
                                     winrt::Windows::UI::Core::PointerEventArgs const& args)
    {
        auto point = args.CurrentPoint().Position();
        
        if (IsPointInBounds(point))
        {
            // Convert CoreWindow coordinates to RiveRenderer bounds
            float rendererX = static_cast<float>(point.X);
            float rendererY = static_cast<float>(point.Y);
            
            // Forward to RiveRenderer input queue
            if (m_riveRenderer) {
                m_riveRenderer->QueuePointerPress(rendererX, rendererY);
            }
        }
    }

    void RiveControl::OnPointerReleased(winrt::Windows::UI::Core::CoreWindow const& sender,
                                       winrt::Windows::UI::Core::PointerEventArgs const& args)
    {
        auto point = args.CurrentPoint().Position();
        
        if (IsPointInBounds(point))
        {
            // Convert CoreWindow coordinates to RiveRenderer bounds
            float rendererX = static_cast<float>(point.X);
            float rendererY = static_cast<float>(point.Y);
            
            // Forward to RiveRenderer input queue
            if (m_riveRenderer) {
                m_riveRenderer->QueuePointerRelease(rendererX, rendererY);
            }
        }
    }

    bool RiveControl::IsPointInBounds(winrt::Windows::Foundation::Point const& point)
    {
        // Simple bounds check - in a real implementation, we'd need to consider
        // the visual's position relative to the window
        return point.X >= 0 && point.X <= m_width &&
               point.Y >= 0 && point.Y <= m_height;
    }

    // State machine enumeration
    winrt::Windows::Foundation::Collections::IVectorView<winrt::WinRive::StateMachineInfo> RiveControl::GetStateMachines()
    {
        std::vector<winrt::WinRive::StateMachineInfo> result;
        
        if (m_riveRenderer)
        {
            auto stateMachines = m_riveRenderer->EnumerateStateMachines();
            for (const auto& sm : stateMachines)
            {
                WinRive::StateMachineInfo info;
                info.Name = winrt::to_hstring(sm.name);
                info.Index = sm.index;
                info.IsDefault = sm.isDefault;
                result.push_back(info);
            }
        }
        
        return winrt::single_threaded_vector<WinRive::StateMachineInfo>(std::move(result)).GetView();
    }

    winrt::WinRive::StateMachineInfo RiveControl::GetDefaultStateMachine()
    {
        WinRive::StateMachineInfo defaultInfo;
        defaultInfo.Name = L"";
        defaultInfo.Index = -1;
        defaultInfo.IsDefault = false;
        
        if (m_riveRenderer)
        {
            auto defaultSM = m_riveRenderer->GetDefaultStateMachine();
            defaultInfo.Name = winrt::to_hstring(defaultSM.name);
            defaultInfo.Index = defaultSM.index;
            defaultInfo.IsDefault = defaultSM.isDefault;
        }
        
        return defaultInfo;
    }

    int32_t RiveControl::GetStateMachineCount()
    {
        if (m_riveRenderer)
        {
            return m_riveRenderer->GetStateMachineCount();
        }
        return 0;
    }

    // State machine control
    bool RiveControl::SetActiveStateMachine(int32_t index)
    {
        if (m_riveRenderer)
        {
            return m_riveRenderer->SetActiveStateMachine(index);
        }
        return false;
    }

    bool RiveControl::SetActiveStateMachineByName(hstring const& name)
    {
        if (m_riveRenderer)
        {
            return m_riveRenderer->SetActiveStateMachineByName(winrt::to_string(name));
        }
        return false;
    }

    int32_t RiveControl::GetActiveStateMachineIndex()
    {
        if (m_riveRenderer)
        {
            return m_riveRenderer->GetActiveStateMachineIndex();
        }
        return -1;
    }

    // State machine playback control
    void RiveControl::PlayStateMachine()
    {
        if (m_riveRenderer)
        {
            m_riveRenderer->PlayStateMachine();
        }
    }

    void RiveControl::PauseStateMachine()
    {
        if (m_riveRenderer)
        {
            m_riveRenderer->PauseStateMachine();
        }
    }

    void RiveControl::ResetStateMachine()
    {
        if (m_riveRenderer)
        {
            m_riveRenderer->ResetStateMachine();
        }
    }

    bool RiveControl::IsStateMachineActive()
    {
        if (m_riveRenderer)
        {
            return m_riveRenderer->IsStateMachineActive();
        }
        return false;
    }

    // Input control
    winrt::Windows::Foundation::Collections::IVectorView<winrt::WinRive::StateMachineInput> RiveControl::GetStateMachineInputs()
    {
        std::vector<winrt::WinRive::StateMachineInput> result;
        
        if (m_riveRenderer)
        {
            auto inputs = m_riveRenderer->GetStateMachineInputs();
            for (const auto& input : inputs)
            {
                winrt::WinRive::StateMachineInput inputInfo;
                inputInfo.Name = winrt::to_hstring(input.name);
                inputInfo.Type = winrt::to_hstring(input.type);
                inputInfo.BooleanValue = input.booleanValue;
                inputInfo.NumberValue = input.numberValue;
                result.push_back(inputInfo);
            }
        }
        
        return winrt::single_threaded_vector(std::move(result)).GetView();
    }

    bool RiveControl::SetBooleanInput(hstring const& inputName, bool value)
    {
        if (m_riveRenderer)
        {
            return m_riveRenderer->SetBooleanInput(winrt::to_string(inputName), value);
        }
        return false;
    }

    bool RiveControl::SetNumberInput(hstring const& inputName, double value)
    {
        if (m_riveRenderer)
        {
            return m_riveRenderer->SetNumberInput(winrt::to_string(inputName), value);
        }
        return false;
    }

    bool RiveControl::FireTrigger(hstring const& inputName)
    {
        if (m_riveRenderer)
        {
            return m_riveRenderer->FireTrigger(winrt::to_string(inputName));
        }
        return false;
    }
}
