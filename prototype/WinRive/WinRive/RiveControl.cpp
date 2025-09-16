#include "pch.h"
#include "RiveControl.h"
#include "RiveControl.g.cpp"
#include "ViewModelInstance.h"

namespace winrt::WinRive::implementation
{
    RiveControl::RiveControl()
    {
        m_riveRenderer = std::make_unique<RiveRenderer>();
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
        if (m_riveRenderer)
        {
            m_riveRenderer->StopRenderThread();
            m_riveRenderer->Shutdown();
            m_riveRenderer.reset();
        }
    }

    // Direct input methods for host applications to call
    void RiveControl::QueuePointerMove(float x, float y)
    {
        if (m_riveRenderer)
        {
            m_riveRenderer->QueuePointerMove(x, y);
        }
    }

    void RiveControl::QueuePointerPress(float x, float y)
    {
        if (m_riveRenderer)
        {
            m_riveRenderer->QueuePointerPress(x, y);
        }
    }

    void RiveControl::QueuePointerRelease(float x, float y)
    {
        if (m_riveRenderer)
        {
            m_riveRenderer->QueuePointerRelease(x, y);
        }
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

    // ViewModel support - matching IDL
    Windows::Foundation::Collections::IVectorView<winrt::WinRive::ViewModelInfo> RiveControl::GetViewModels()
    {
        std::vector<winrt::WinRive::ViewModelInfo> result;
        // TODO: Implement when rive viewmodel API is available
        return winrt::single_threaded_vector<winrt::WinRive::ViewModelInfo>(std::move(result)).GetView();
    }

    winrt::WinRive::ViewModel RiveControl::GetViewModelByName(hstring const& name)
    {
        (void)name;
        return nullptr;
    }

    winrt::WinRive::ViewModel RiveControl::GetViewModelAt(int32_t index)
    {
        (void)index;
        return nullptr;
    }

    int32_t RiveControl::GetViewModelCount()
    {
        return 0;
    }

    winrt::WinRive::ViewModel RiveControl::GetDefaultViewModel()
    {
        return nullptr;
    }

    // ViewModelInstance management
    winrt::WinRive::ViewModelInstance RiveControl::CreateViewModelInstance()
    {
        return winrt::make<implementation::ViewModelInstance>();
    }

    winrt::WinRive::ViewModelInstance RiveControl::CreateViewModelInstanceById(int32_t viewModelId)
    {
        (void)viewModelId;
        return nullptr;
    }

    winrt::WinRive::ViewModelInstance RiveControl::CreateViewModelInstanceByName(hstring const& viewModelName)
    {
        (void)viewModelName;
        return nullptr;
    }

    bool RiveControl::BindViewModelInstance(winrt::WinRive::ViewModelInstance const& instance)
    {
        (void)instance;
        return false;
    }

    winrt::WinRive::ViewModelInstance RiveControl::GetBoundViewModelInstance()
    {
        return nullptr;
    }

    // Direct property access (convenience methods)
    bool RiveControl::SetViewModelStringProperty(hstring const& propertyName, hstring const& value)
    {
        (void)propertyName;
        (void)value;
        return false;
    }

    bool RiveControl::SetViewModelNumberProperty(hstring const& propertyName, double value)
    {
        (void)propertyName;
        (void)value;
        return false;
    }

    bool RiveControl::SetViewModelBooleanProperty(hstring const& propertyName, bool value)
    {
        (void)propertyName;
        (void)value;
        return false;
    }

    bool RiveControl::SetViewModelColorProperty(hstring const& propertyName, uint32_t color)
    {
        (void)propertyName;
        (void)color;
        return false;
    }

    bool RiveControl::SetViewModelEnumProperty(hstring const& propertyName, int32_t value)
    {
        (void)propertyName;
        (void)value;
        return false;
    }

    bool RiveControl::FireViewModelTrigger(hstring const& triggerName)
    {
        (void)triggerName;
        return false;
    }

    // Events
    winrt::event_token RiveControl::ViewModelInstanceBound(Windows::Foundation::TypedEventHandler<winrt::WinRive::RiveControl, winrt::WinRive::ViewModelInstance> const& handler)
    {
        return m_viewModelInstanceBoundEvent.add(handler);
    }

    void RiveControl::ViewModelInstanceBound(winrt::event_token const& token) noexcept
    {
        m_viewModelInstanceBoundEvent.remove(token);
    }

    winrt::event_token RiveControl::ViewModelPropertyChanged(Windows::Foundation::TypedEventHandler<winrt::WinRive::RiveControl, winrt::WinRive::ViewModelInstanceProperty> const& handler)
    {
        return m_viewModelPropertyChangedEvent.add(handler);
    }

    void RiveControl::ViewModelPropertyChanged(winrt::event_token const& token) noexcept
    {
        m_viewModelPropertyChangedEvent.remove(token);
    }
}
