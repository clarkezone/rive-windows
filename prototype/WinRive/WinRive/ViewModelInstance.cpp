#include "pch.h"
#include "ViewModelInstance.h"
#include "ViewModelInstance.g.cpp"
#include "ViewModelInstanceProperty.h"

namespace winrt::WinRive::implementation
{
    ViewModelInstance::ViewModelInstance(winrt::WinRive::ViewModel const& viewModel)
        : m_viewModel(viewModel)
    {
    }

    winrt::WinRive::ViewModel ViewModelInstance::ViewModel()
    {
        return m_viewModel;
    }

    Windows::Foundation::Collections::IVectorView<winrt::WinRive::ViewModelInstanceProperty> ViewModelInstance::GetProperties()
    {
        if (!m_propertiesCached)
        {
            CacheProperties();
        }
        return winrt::single_threaded_vector<winrt::WinRive::ViewModelInstanceProperty>(std::move(m_properties)).GetView();
    }

    winrt::WinRive::ViewModelInstanceProperty ViewModelInstance::GetProperty(hstring const& name)
    {
        if (!m_propertiesCached)
        {
            CacheProperties();
        }

        for (const auto& prop : m_properties)
        {
            if (prop.Name() == name)
            {
                return prop;
            }
        }

        return nullptr; // Not found
    }

    winrt::WinRive::ViewModelInstanceProperty ViewModelInstance::GetPropertyAt(int32_t index)
    {
        if (!m_propertiesCached)
        {
            CacheProperties();
        }

        if (index >= 0 && index < static_cast<int32_t>(m_properties.size()))
        {
            return m_properties[index];
        }

        return nullptr; // Invalid index
    }

    int32_t ViewModelInstance::GetPropertyCount()
    {
        if (!m_propertiesCached)
        {
            CacheProperties();
        }
        return static_cast<int32_t>(m_properties.size());
    }

    bool ViewModelInstance::SetStringProperty(hstring const& name, hstring const& value)
    {
#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
        if (!m_nativeInstance) {
            return false;
        }
        
        auto* nativeInstance = static_cast<rive::ViewModelInstance*>(m_nativeInstance);
        std::string propName = winrt::to_string(name);
        std::string propValue = winrt::to_string(value);
        
        // Use the same pattern as RiveRenderer::SetViewModelStringProperty
        auto property = nativeInstance->propertyValue(propName);
        if (property) {
            auto stringProperty = static_cast<rive::ViewModelInstanceString*>(property);
            if (stringProperty) {
                stringProperty->propertyValue(propValue);
                
                // Fire property changed event
                auto prop = GetProperty(name);
                if (prop) {
                    m_propertyChangedEvent(*this, prop);
                }
                return true;
            }
        }
#else
        // Unused parameters
        (void)name;
        (void)value;
#endif
        return false;
    }

    bool ViewModelInstance::SetNumberProperty(hstring const& name, double value)
    {
#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
        if (!m_nativeInstance) {
            return false;
        }
        
        auto* nativeInstance = static_cast<rive::ViewModelInstance*>(m_nativeInstance);
        std::string propName = winrt::to_string(name);
        
        auto property = nativeInstance->propertyValue(propName);
        if (property) {
            auto numberProperty = static_cast<rive::ViewModelInstanceNumber*>(property);
            if (numberProperty) {
                numberProperty->propertyValue(static_cast<float>(value));
                
                // Fire property changed event
                auto prop = GetProperty(name);
                if (prop) {
                    m_propertyChangedEvent(*this, prop);
                }
                return true;
            }
        }
#else
        // Unused parameters
        (void)name;
        (void)value;
#endif
        return false;
    }

    bool ViewModelInstance::SetBooleanProperty(hstring const& name, bool value)
    {
#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
        if (!m_nativeInstance) {
            return false;
        }
        
        auto* nativeInstance = static_cast<rive::ViewModelInstance*>(m_nativeInstance);
        std::string propName = winrt::to_string(name);
        
        auto property = nativeInstance->propertyValue(propName);
        if (property) {
            auto boolProperty = static_cast<rive::ViewModelInstanceBoolean*>(property);
            if (boolProperty) {
                boolProperty->propertyValue(value);
                
                // Fire property changed event
                auto prop = GetProperty(name);
                if (prop) {
                    m_propertyChangedEvent(*this, prop);
                }
                return true;
            }
        }
#else
        // Unused parameters
        (void)name;
        (void)value;
#endif
        return false;
    }

    bool ViewModelInstance::SetColorProperty(hstring const& name, uint32_t color)
    {
#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
        if (!m_nativeInstance) {
            return false;
        }
        
        auto* nativeInstance = static_cast<rive::ViewModelInstance*>(m_nativeInstance);
        std::string propName = winrt::to_string(name);
        
        auto property = nativeInstance->propertyValue(propName);
        if (property) {
            auto colorProperty = static_cast<rive::ViewModelInstanceColor*>(property);
            if (colorProperty) {
                colorProperty->propertyValue(color);
                
                // Fire property changed event
                auto prop = GetProperty(name);
                if (prop) {
                    m_propertyChangedEvent(*this, prop);
                }
                return true;
            }
        }
#else
        // Unused parameters
        (void)name;
        (void)color;
#endif
        return false;
    }

    bool ViewModelInstance::SetEnumProperty(hstring const& name, int32_t value)
    {
#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
        if (!m_nativeInstance) {
            return false;
        }
        
        auto* nativeInstance = static_cast<rive::ViewModelInstance*>(m_nativeInstance);
        std::string propName = winrt::to_string(name);
        
        auto property = nativeInstance->propertyValue(propName);
        if (property) {
            auto enumProperty = static_cast<rive::ViewModelInstanceEnum*>(property);
            if (enumProperty) {
                enumProperty->propertyValue(static_cast<uint32_t>(value));
                
                // Fire property changed event
                auto prop = GetProperty(name);
                if (prop) {
                    m_propertyChangedEvent(*this, prop);
                }
                return true;
            }
        }
#else
        // Unused parameters
        (void)name;
        (void)value;
#endif
        return false;
    }

    bool ViewModelInstance::FireTrigger(hstring const& name)
    {
#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
        if (!m_nativeInstance) {
            return false;
        }
        
        auto* nativeInstance = static_cast<rive::ViewModelInstance*>(m_nativeInstance);
        std::string propName = winrt::to_string(name);
        
        auto property = nativeInstance->propertyValue(propName);
        if (property) {
            auto trigger = static_cast<rive::ViewModelInstanceTrigger*>(property);
            if (trigger) {
                trigger->trigger();
                
                // Fire property changed event
                auto prop = GetProperty(name);
                if (prop) {
                    m_propertyChangedEvent(*this, prop);
                }
                return true;
            }
        }
#else
        // Unused parameters
        (void)name;
#endif
        return false;
    }

    bool ViewModelInstance::IsValid() const
    {
#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
        return m_nativeInstance != nullptr;
#else
        return m_viewModel != nullptr;
#endif
    }

    winrt::event_token ViewModelInstance::PropertyChanged(Windows::Foundation::TypedEventHandler<winrt::WinRive::ViewModelInstance, winrt::WinRive::ViewModelInstanceProperty> const& handler)
    {
        return m_propertyChangedEvent.add(handler);
    }

    void ViewModelInstance::PropertyChanged(winrt::event_token const& token) noexcept
    {
        m_propertyChangedEvent.remove(token);
    }

    void* ViewModelInstance::GetNativeInstance() const
    {
#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
        return m_nativeInstance;
#else
        return nullptr;
#endif
    }

    void ViewModelInstance::SetNativeInstance(void* nativeInstance)
    {
#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
        m_nativeInstance = nativeInstance;
        InvalidatePropertyCache(); // Force re-cache of properties when native instance changes
#else
        // Unused when rive headers not available
        (void)nativeInstance;
#endif
    }

    void ViewModelInstance::InvalidatePropertyCache()
    {
        m_propertiesCached = false;
        m_properties.clear();
    }

    void ViewModelInstance::CacheProperties() const
    {
        m_properties.clear();

        if (!IsValid() || !m_viewModel)
        {
            m_propertiesCached = true;
            return;
        }

        // Get properties from the view model definition
        auto viewModelProperties = m_viewModel.GetProperties();
        for (uint32_t i = 0; i < viewModelProperties.Size(); ++i)
        {
            auto propInfo = viewModelProperties.GetAt(i);
            auto propertyWrapper = CreatePropertyWrapper(propInfo.Index, propInfo.Name);
            if (propertyWrapper)
            {
                m_properties.push_back(propertyWrapper);
            }
        }

        m_propertiesCached = true;
    }

    winrt::WinRive::ViewModelInstanceProperty ViewModelInstance::CreatePropertyWrapper(int32_t index, hstring const& name) const
    {
        // Create a new ViewModelInstanceProperty that references this instance
        auto propertyImpl = winrt::make<implementation::ViewModelInstanceProperty>(
            name, 
            index, 
            *this
        );
        
        return propertyImpl.as<winrt::WinRive::ViewModelInstanceProperty>();
    }
}
