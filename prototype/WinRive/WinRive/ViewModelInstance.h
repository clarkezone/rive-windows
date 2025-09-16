#pragma once
#include "ViewModelInstance.g.h"
#include "ViewModel.h"

#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
#include "rive/viewmodel/viewmodel_instance.hpp"
#endif

namespace winrt::WinRive::implementation
{
    struct ViewModelInstance : ViewModelInstanceT<ViewModelInstance>
    {
        ViewModelInstance() = default;
        ViewModelInstance(winrt::WinRive::ViewModel const& viewModel);

        // Associated view model
        winrt::WinRive::ViewModel ViewModel();

        // Property access
        Windows::Foundation::Collections::IVectorView<winrt::WinRive::ViewModelInstanceProperty> GetProperties();
        winrt::WinRive::ViewModelInstanceProperty GetProperty(hstring const& name);
        winrt::WinRive::ViewModelInstanceProperty GetPropertyAt(int32_t index);
        int32_t GetPropertyCount();

        // Bulk operations
        bool SetStringProperty(hstring const& name, hstring const& value);
        bool SetNumberProperty(hstring const& name, double value);
        bool SetBooleanProperty(hstring const& name, bool value);
        bool SetColorProperty(hstring const& name, uint32_t color);
        bool SetEnumProperty(hstring const& name, int32_t value);
        bool FireTrigger(hstring const& name);

        // Validation
        bool IsValid() const;

        // Events
        winrt::event_token PropertyChanged(Windows::Foundation::TypedEventHandler<winrt::WinRive::ViewModelInstance, winrt::WinRive::ViewModelInstanceProperty> const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;

        // Internal methods
        void* GetNativeInstance() const;
        void InvalidatePropertyCache();

    private:
        winrt::WinRive::ViewModel m_viewModel{ nullptr };

#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
        void* m_nativeInstance{ nullptr };
#endif

        // Property cache
        mutable std::vector<winrt::WinRive::ViewModelInstanceProperty> m_properties;
        mutable bool m_propertiesCached{ false };

        // Events
        winrt::event<Windows::Foundation::TypedEventHandler<winrt::WinRive::ViewModelInstance, winrt::WinRive::ViewModelInstanceProperty>> m_propertyChangedEvent;

        void CacheProperties() const;
        winrt::WinRive::ViewModelInstanceProperty CreatePropertyWrapper(int32_t index, hstring const& name) const;
    };
}

namespace winrt::WinRive::factory_implementation
{
    struct ViewModelInstance : ViewModelInstanceT<ViewModelInstance, implementation::ViewModelInstance>
    {
    };
}
