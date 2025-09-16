#include "pch.h"
#include "ViewModel.h"
#include "ViewModel.g.cpp"

namespace winrt::WinRive::implementation
{
    ViewModel::ViewModel()
    {
    }

    ViewModel::ViewModel(hstring const& name, int32_t index, int32_t id)
        : m_name(name), m_index(index), m_id(id)
    {
    }

    hstring ViewModel::Name()
    {
        return m_name;
    }

    int32_t ViewModel::Index()
    {
        return m_index;
    }

    int32_t ViewModel::Id()
    {
        return m_id;
    }

    Windows::Foundation::Collections::IVectorView<WinRive::ViewModelPropertyInfo> ViewModel::GetProperties()
    {
        if (!m_propertiesCached)
        {
            CacheProperties();
        }
        return winrt::single_threaded_vector<WinRive::ViewModelPropertyInfo>(std::move(m_properties)).GetView();
    }

    int32_t ViewModel::GetPropertyCount()
    {
        if (!m_propertiesCached)
        {
            CacheProperties();
        }
        return static_cast<int32_t>(m_properties.size());
    }

    WinRive::ViewModelPropertyInfo ViewModel::GetPropertyAt(int32_t index)
    {
        if (!m_propertiesCached)
        {
            CacheProperties();
        }

        if (index >= 0 && index < static_cast<int32_t>(m_properties.size()))
        {
            return m_properties[index];
        }

        // Return empty property info for invalid index
        WinRive::ViewModelPropertyInfo empty{};
        empty.Name = L"";
        empty.Type = WinRive::ViewModelPropertyType::String;
        empty.Index = -1;
        return empty;
    }

    WinRive::ViewModelPropertyInfo ViewModel::GetPropertyByName(hstring const& name)
    {
        if (!m_propertiesCached)
        {
            CacheProperties();
        }

        for (const auto& prop : m_properties)
        {
            if (prop.Name == name)
            {
                return prop;
            }
        }

        // Return empty property info for not found
        WinRive::ViewModelPropertyInfo empty{};
        empty.Name = L"";
        empty.Type = WinRive::ViewModelPropertyType::String;
        empty.Index = -1;
        return empty;
    }

    void ViewModel::SetNativeViewModel(void* nativeViewModel)
    {
#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
        m_nativeViewModel = nativeViewModel;
        m_propertiesCached = false; // Force re-cache of properties
#else
        // Unused when rive headers not available
        (void)nativeViewModel;
#endif
    }

    bool ViewModel::IsValid() const
    {
#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
        return m_nativeViewModel != nullptr;
#else
        return !m_name.empty() && m_index >= 0;
#endif
    }

    void ViewModel::CacheProperties() const
    {
        m_properties.clear();

#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
        if (m_nativeViewModel)
        {
            // TODO: Implement actual property enumeration when rive viewmodel API is available
            // For now, create placeholder properties
            // This would use something like m_nativeViewModel->properties() when available
        }
#endif

        // For now, create a placeholder property for testing
        WinRive::ViewModelPropertyInfo testProp{};
        testProp.Name = L"TestProperty";
        testProp.Type = WinRive::ViewModelPropertyType::String;
        testProp.Index = 0;
        m_properties.push_back(testProp);

        m_propertiesCached = true;
    }

    WinRive::ViewModelPropertyType ViewModel::MapNativePropertyType(int nativeType) const
    {
        // TODO: Implement actual mapping when rive viewmodel property types are available
        // This would map from rive::ViewModelPropertyType to WinRive::ViewModelPropertyType
        switch (nativeType)
        {
        case 0: return WinRive::ViewModelPropertyType::String;
        case 1: return WinRive::ViewModelPropertyType::Number;
        case 2: return WinRive::ViewModelPropertyType::Boolean;
        case 3: return WinRive::ViewModelPropertyType::Color;
        case 4: return WinRive::ViewModelPropertyType::Enum;
        case 5: return WinRive::ViewModelPropertyType::Trigger;
        default: return WinRive::ViewModelPropertyType::String;
        }
    }
}
