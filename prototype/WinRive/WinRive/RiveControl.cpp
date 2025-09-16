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
        
        if (m_riveRenderer)
        {
            auto viewModels = m_riveRenderer->EnumerateViewModels();
            for (const auto& vm : viewModels)
            {
                winrt::WinRive::ViewModelInfo info;
                info.Name = winrt::to_hstring(vm.name);
                info.Index = vm.index;
                info.Id = vm.id;
                result.push_back(info);
            }
        }
        
        return winrt::single_threaded_vector<winrt::WinRive::ViewModelInfo>(std::move(result)).GetView();
    }

    winrt::WinRive::ViewModel RiveControl::GetViewModelByName(hstring const& name)
    {
        if (!m_riveRenderer)
        {
            return nullptr;
        }
        
        auto viewModels = m_riveRenderer->EnumerateViewModels();
        std::string nameStr = winrt::to_string(name);
        
        for (const auto& vm : viewModels)
        {
            if (vm.name == nameStr)
            {
                // Create ViewModel wrapper
                auto viewModelImpl = winrt::make<implementation::ViewModel>(
                    winrt::to_hstring(vm.name),
                    vm.index,
                    vm.id
                );
                return viewModelImpl.as<winrt::WinRive::ViewModel>();
            }
        }
        
        return nullptr;
    }

    winrt::WinRive::ViewModel RiveControl::GetViewModelAt(int32_t index)
    {
        if (!m_riveRenderer)
        {
            return nullptr;
        }
        
        auto viewModels = m_riveRenderer->EnumerateViewModels();
        
        for (const auto& vm : viewModels)
        {
            if (vm.index == index)
            {
                // Create ViewModel wrapper
                auto viewModelImpl = winrt::make<implementation::ViewModel>(
                    winrt::to_hstring(vm.name),
                    vm.index,
                    vm.id
                );
                return viewModelImpl.as<winrt::WinRive::ViewModel>();
            }
        }
        
        return nullptr;
    }

    int32_t RiveControl::GetViewModelCount()
    {
        if (m_riveRenderer)
        {
            return m_riveRenderer->GetViewModelCount();
        }
        return 0;
    }

    winrt::WinRive::ViewModel RiveControl::GetDefaultViewModel()
    {
        if (!m_riveRenderer)
        {
            return nullptr;
        }
        
        auto defaultVM = m_riveRenderer->GetDefaultViewModel();
        if (defaultVM.index >= 0)
        {
            // Create ViewModel wrapper
            auto viewModelImpl = winrt::make<implementation::ViewModel>(
                winrt::to_hstring(defaultVM.name),
                defaultVM.index,
                defaultVM.id
            );
            return viewModelImpl.as<winrt::WinRive::ViewModel>();
        }
        
        return nullptr;
    }

    // ViewModelInstance management
    winrt::WinRive::ViewModelInstance RiveControl::CreateViewModelInstance()
    {
        if (!m_riveRenderer)
        {
            return nullptr;
        }
        
        // Create instance using default ViewModel (first one or artboard's ViewModel)
        void* nativeInstance = m_riveRenderer->CreateViewModelInstance();
        if (nativeInstance)
        {
            // Get the default ViewModel to associate with the instance
            auto defaultVM = GetDefaultViewModel();
            if (defaultVM)
            {
                auto instanceImpl = winrt::make<implementation::ViewModelInstance>(defaultVM);
                
                // Set the native instance pointer
                instanceImpl.as<implementation::ViewModelInstance>()->SetNativeInstance(nativeInstance);
                
                return instanceImpl.as<winrt::WinRive::ViewModelInstance>();
            }
        }
        
        return nullptr;
    }

    winrt::WinRive::ViewModelInstance RiveControl::CreateViewModelInstanceById(int32_t viewModelId)
    {
        if (!m_riveRenderer)
        {
            return nullptr;
        }
        
        void* nativeInstance = m_riveRenderer->CreateViewModelInstanceById(viewModelId);
        if (nativeInstance)
        {
            // Find the ViewModel with this ID
            auto viewModels = m_riveRenderer->EnumerateViewModels();
            for (const auto& vm : viewModels)
            {
                if (vm.id == viewModelId)
                {
                    auto viewModelWrapper = winrt::make<implementation::ViewModel>(
                        winrt::to_hstring(vm.name),
                        vm.index,
                        vm.id
                    );
                    
                    auto instanceImpl = winrt::make<implementation::ViewModelInstance>(
                        viewModelWrapper.as<winrt::WinRive::ViewModel>()
                    );
                    
                    // Set the native instance pointer
                    instanceImpl.as<implementation::ViewModelInstance>()->SetNativeInstance(nativeInstance);
                    
                    return instanceImpl.as<winrt::WinRive::ViewModelInstance>();
                }
            }
        }
        
        return nullptr;
    }

    winrt::WinRive::ViewModelInstance RiveControl::CreateViewModelInstanceByName(hstring const& viewModelName)
    {
        if (!m_riveRenderer)
        {
            return nullptr;
        }
        
        std::string nameStr = winrt::to_string(viewModelName);
        void* nativeInstance = m_riveRenderer->CreateViewModelInstanceByName(nameStr);
        if (nativeInstance)
        {
            // Find the ViewModel with this name
            auto viewModels = m_riveRenderer->EnumerateViewModels();
            for (const auto& vm : viewModels)
            {
                if (vm.name == nameStr)
                {
                    auto viewModelWrapper = winrt::make<implementation::ViewModel>(
                        winrt::to_hstring(vm.name),
                        vm.index,
                        vm.id
                    );
                    
                    auto instanceImpl = winrt::make<implementation::ViewModelInstance>(
                        viewModelWrapper.as<winrt::WinRive::ViewModel>()
                    );
                    
                    // Set the native instance pointer
                    instanceImpl.as<implementation::ViewModelInstance>()->SetNativeInstance(nativeInstance);
                    
                    return instanceImpl.as<winrt::WinRive::ViewModelInstance>();
                }
            }
        }
        
        return nullptr;
    }

    bool RiveControl::BindViewModelInstance(winrt::WinRive::ViewModelInstance const& instance)
    {
        if (!m_riveRenderer || !instance)
        {
            return false;
        }
        
        // Get the native instance pointer from the WinRT wrapper
        auto instanceImpl = instance.as<implementation::ViewModelInstance>();
        void* nativeInstance = instanceImpl->GetNativeInstance();
        
        if (nativeInstance)
        {
            bool success = m_riveRenderer->BindViewModelInstance(nativeInstance);
            if (success)
            {
                m_boundViewModelInstance = instance;
                
                // Fire the bound event
                m_viewModelInstanceBoundEvent(*this, instance);
                
                return true;
            }
        }
        
        return false;
    }

    winrt::WinRive::ViewModelInstance RiveControl::GetBoundViewModelInstance()
    {
        return m_boundViewModelInstance;
    }

    // Direct property access (convenience methods)
    bool RiveControl::SetViewModelStringProperty(hstring const& propertyName, hstring const& value)
    {
        if (m_riveRenderer)
        {
            std::string propName = winrt::to_string(propertyName);
            std::string propValue = winrt::to_string(value);
            bool success = m_riveRenderer->SetViewModelStringProperty(propName, propValue);
            
            if (success && m_boundViewModelInstance)
            {
                // Fire property changed event if we have a bound instance
                auto property = m_boundViewModelInstance.GetProperty(propertyName);
                if (property)
                {
                    m_viewModelPropertyChangedEvent(*this, property);
                }
            }
            
            return success;
        }
        return false;
    }

    bool RiveControl::SetViewModelNumberProperty(hstring const& propertyName, double value)
    {
        if (m_riveRenderer)
        {
            std::string propName = winrt::to_string(propertyName);
            bool success = m_riveRenderer->SetViewModelNumberProperty(propName, value);
            
            if (success && m_boundViewModelInstance)
            {
                // Fire property changed event if we have a bound instance
                auto property = m_boundViewModelInstance.GetProperty(propertyName);
                if (property)
                {
                    m_viewModelPropertyChangedEvent(*this, property);
                }
            }
            
            return success;
        }
        return false;
    }

    bool RiveControl::SetViewModelBooleanProperty(hstring const& propertyName, bool value)
    {
        if (m_riveRenderer)
        {
            std::string propName = winrt::to_string(propertyName);
            bool success = m_riveRenderer->SetViewModelBooleanProperty(propName, value);
            
            if (success && m_boundViewModelInstance)
            {
                // Fire property changed event if we have a bound instance
                auto property = m_boundViewModelInstance.GetProperty(propertyName);
                if (property)
                {
                    m_viewModelPropertyChangedEvent(*this, property);
                }
            }
            
            return success;
        }
        return false;
    }

    bool RiveControl::SetViewModelColorProperty(hstring const& propertyName, uint32_t color)
    {
        if (m_riveRenderer)
        {
            std::string propName = winrt::to_string(propertyName);
            bool success = m_riveRenderer->SetViewModelColorProperty(propName, color);
            
            if (success && m_boundViewModelInstance)
            {
                // Fire property changed event if we have a bound instance
                auto property = m_boundViewModelInstance.GetProperty(propertyName);
                if (property)
                {
                    m_viewModelPropertyChangedEvent(*this, property);
                }
            }
            
            return success;
        }
        return false;
    }

    bool RiveControl::SetViewModelEnumProperty(hstring const& propertyName, int32_t value)
    {
        if (m_riveRenderer)
        {
            std::string propName = winrt::to_string(propertyName);
            bool success = m_riveRenderer->SetViewModelEnumProperty(propName, value);
            
            if (success && m_boundViewModelInstance)
            {
                // Fire property changed event if we have a bound instance
                auto property = m_boundViewModelInstance.GetProperty(propertyName);
                if (property)
                {
                    m_viewModelPropertyChangedEvent(*this, property);
                }
            }
            
            return success;
        }
        return false;
    }

    bool RiveControl::FireViewModelTrigger(hstring const& triggerName)
    {
        if (m_riveRenderer)
        {
            std::string triggerNameStr = winrt::to_string(triggerName);
            bool success = m_riveRenderer->FireViewModelTrigger(triggerNameStr);
            
            if (success && m_boundViewModelInstance)
            {
                // Fire property changed event if we have a bound instance
                auto property = m_boundViewModelInstance.GetProperty(triggerName);
                if (property)
                {
                    m_viewModelPropertyChangedEvent(*this, property);
                }
            }
            
            return success;
        }
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
