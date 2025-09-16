#pragma once
#include "ViewModelInstanceProperty.g.h"

namespace winrt::WinRive::implementation
{
    struct ViewModelInstanceProperty : ViewModelInstancePropertyT<ViewModelInstanceProperty>
    {
        ViewModelInstanceProperty() = default;
        ViewModelInstanceProperty(hstring const& name, int32_t index, winrt::WinRive::ViewModelInstance const& parentInstance);

        // Basic info
        hstring Name();
        winrt::WinRive::ViewModelPropertyType Type();
        int32_t Index();

        // Value access - use IInspectable to support different types
        Windows::Foundation::IInspectable Value();
        void Value(Windows::Foundation::IInspectable const& value);

        // Type-safe accessors
        hstring StringValue();
        void StringValue(hstring const& value);
        double NumberValue();
        void NumberValue(double value);
        bool BooleanValue();
        void BooleanValue(bool value);
        uint32_t ColorValue();
        void ColorValue(uint32_t value);
        int32_t EnumValue();
        void EnumValue(int32_t value);

        // Trigger support
        void Fire();

        // Validation
        bool IsValid();

        // Events
        winrt::event_token ValueChanged(Windows::Foundation::TypedEventHandler<winrt::WinRive::ViewModelInstanceProperty, Windows::Foundation::IInspectable> const& handler);
        void ValueChanged(winrt::event_token const& token) noexcept;

        // Internal methods
        void SetType(winrt::WinRive::ViewModelPropertyType type);
        winrt::WinRive::ViewModelInstance GetParentInstance() const;

    private:
        hstring m_name;
        int32_t m_index{ -1 };
        winrt::WinRive::ViewModelPropertyType m_type{ winrt::WinRive::ViewModelPropertyType::String };
        winrt::weak_ref<winrt::WinRive::ViewModelInstance> m_parentInstance;

        // Events
        winrt::event<Windows::Foundation::TypedEventHandler<winrt::WinRive::ViewModelInstanceProperty, Windows::Foundation::IInspectable>> m_valueChangedEvent;

        void FireValueChangedEvent(Windows::Foundation::IInspectable const& newValue);
        bool ValidateParent() const;
    };
}

namespace winrt::WinRive::factory_implementation
{
    struct ViewModelInstanceProperty : ViewModelInstancePropertyT<ViewModelInstanceProperty, implementation::ViewModelInstanceProperty>
    {
    };
}
