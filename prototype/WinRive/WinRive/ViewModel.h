#pragma once
#include "ViewModel.g.h"

#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
#include "rive/file.hpp"
#include "rive/viewmodel/viewmodel.hpp"
#include "rive/viewmodel/viewmodel_property_string.hpp"
#endif

namespace winrt::WinRive::implementation
{
    struct ViewModel : ViewModelT<ViewModel>
    {
        ViewModel();
        ViewModel(hstring const& name, int32_t index, int32_t id);

        // Basic info properties
        hstring Name();
        int32_t Index();
        int32_t Id();

        // Property enumeration
        Windows::Foundation::Collections::IVectorView<WinRive::ViewModelPropertyInfo> GetProperties();
        int32_t GetPropertyCount();
        WinRive::ViewModelPropertyInfo GetPropertyAt(int32_t index);
        WinRive::ViewModelPropertyInfo GetPropertyByName(hstring const& name);

        // Internal methods
        void SetNativeViewModel(void* nativeViewModel);
        bool IsValid() const;

    private:
        hstring m_name;
        int32_t m_index{ -1 };
        int32_t m_id{ -1 };

#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
        void* m_nativeViewModel{ nullptr }; // Weak reference - owned by File
#endif

        // Cache for properties
        mutable std::vector<WinRive::ViewModelPropertyInfo> m_properties;
        mutable bool m_propertiesCached{ false };

        void CacheProperties() const;
        WinRive::ViewModelPropertyType MapNativePropertyType(int nativeType) const;
    };
}

namespace winrt::WinRive::factory_implementation
{
    struct ViewModel : ViewModelT<ViewModel, implementation::ViewModel>
    {
    };
}
