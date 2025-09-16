#include "pch.h"
#include "ViewModelInstanceProperty.h"
#include "ViewModelInstanceProperty.g.cpp"
#include "ViewModelInstance.h"

namespace winrt::WinRive::implementation
{
    ViewModelInstanceProperty::ViewModelInstanceProperty(hstring const& name, int32_t index, winrt::WinRive::ViewModelInstance const& parentInstance)
        : m_name(name), m_index(index), m_parentInstance(parentInstance)
    {
        // Determine type from parent view model
        if (auto parent = m_parentInstance.get())
        {
            if (auto viewModel = parent.ViewModel())
            {
                auto propInfo = viewModel.GetPropertyByName(name);
                if (!propInfo.Name.empty())
                {
                    m_type = propInfo.Type;
                }
            }
        }
    }

    hstring ViewModelInstanceProperty::Name()
    {
        return m_name;
    }

    winrt::WinRive::ViewModelPropertyType ViewModelInstanceProperty::Type()
    {
        return m_type;
    }

    int32_t ViewModelInstanceProperty::Index()
    {
        return m_index;
    }

    Windows::Foundation::IInspectable ViewModelInstanceProperty::Value()
    {
        if (!ValidateParent()) return nullptr;

        auto parent = m_parentInstance.get();
        if (!parent) return nullptr;

        // Get the current value based on type
        switch (m_type)
        {
        case winrt::WinRive::ViewModelPropertyType::String:
            return winrt::box_value(StringValue());
        case winrt::WinRive::ViewModelPropertyType::Number:
            return winrt::box_value(NumberValue());
        case winrt::WinRive::ViewModelPropertyType::Boolean:
            return winrt::box_value(BooleanValue());
        case winrt::WinRive::ViewModelPropertyType::Color:
            return winrt::box_value(ColorValue());
        case winrt::WinRive::ViewModelPropertyType::Enum:
            return winrt::box_value(EnumValue());
        case winrt::WinRive::ViewModelPropertyType::Trigger:
            // Triggers don't have values
            return nullptr;
        default:
            return nullptr;
        }
    }

    void ViewModelInstanceProperty::Value(Windows::Foundation::IInspectable const& value)
    {
        if (!ValidateParent() || !value) return;

        auto parent = m_parentInstance.get();
        if (!parent) return;

        // Set the value based on type
        switch (m_type)
        {
        case winrt::WinRive::ViewModelPropertyType::String:
            if (auto stringVal = value.try_as<hstring>())
            {
                StringValue(*stringVal);
            }
            break;
        case winrt::WinRive::ViewModelPropertyType::Number:
            if (auto doubleVal = value.try_as<double>())
            {
                NumberValue(*doubleVal);
            }
            break;
        case winrt::WinRive::ViewModelPropertyType::Boolean:
            if (auto boolVal = value.try_as<bool>())
            {
                BooleanValue(*boolVal);
            }
            break;
        case winrt::WinRive::ViewModelPropertyType::Color:
            if (auto colorVal = value.try_as<uint32_t>())
            {
                ColorValue(*colorVal);
            }
            break;
        case winrt::WinRive::ViewModelPropertyType::Enum:
            if (auto enumVal = value.try_as<int32_t>())
            {
                EnumValue(*enumVal);
            }
            break;
        case winrt::WinRive::ViewModelPropertyType::Trigger:
            // Triggers are fired, not set
            Fire();
            break;
        }
    }

    hstring ViewModelInstanceProperty::StringValue()
    {
        if (!ValidateParent()) return L"";

        auto parent = m_parentInstance.get();
        if (!parent) return L"";

#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
        // TODO: Implement when rive viewmodel API is available
        // For now, return placeholder value
#endif

        return L""; // Default value
    }

    void ViewModelInstanceProperty::StringValue(hstring const& value)
    {
        if (!ValidateParent()) return;

        auto parent = m_parentInstance.get();
        if (!parent) return;

        if (parent.SetStringProperty(m_name, value))
        {
            FireValueChangedEvent(winrt::box_value(value));
        }
    }

    double ViewModelInstanceProperty::NumberValue()
    {
        if (!ValidateParent()) return 0.0;

        auto parent = m_parentInstance.get();
        if (!parent) return 0.0;

#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
        // TODO: Implement when rive viewmodel API is available
        // For now, return placeholder value
#endif

        return 0.0; // Default value
    }

    void ViewModelInstanceProperty::NumberValue(double value)
    {
        if (!ValidateParent()) return;

        auto parent = m_parentInstance.get();
        if (!parent) return;

        if (parent.SetNumberProperty(m_name, value))
        {
            FireValueChangedEvent(winrt::box_value(value));
        }
    }

    bool ViewModelInstanceProperty::BooleanValue()
    {
        if (!ValidateParent()) return false;

        auto parent = m_parentInstance.get();
        if (!parent) return false;

#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
        // TODO: Implement when rive viewmodel API is available
        // For now, return placeholder value
#endif

        return false; // Default value
    }

    void ViewModelInstanceProperty::BooleanValue(bool value)
    {
        if (!ValidateParent()) return;

        auto parent = m_parentInstance.get();
        if (!parent) return;

        if (parent.SetBooleanProperty(m_name, value))
        {
            FireValueChangedEvent(winrt::box_value(value));
        }
    }

    uint32_t ViewModelInstanceProperty::ColorValue()
    {
        if (!ValidateParent()) return 0;

        auto parent = m_parentInstance.get();
        if (!parent) return 0;

#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
        // TODO: Implement when rive viewmodel API is available
        // For now, return placeholder value
#endif

        return 0; // Default value
    }

    void ViewModelInstanceProperty::ColorValue(uint32_t value)
    {
        if (!ValidateParent()) return;

        auto parent = m_parentInstance.get();
        if (!parent) return;

        if (parent.SetColorProperty(m_name, value))
        {
            FireValueChangedEvent(winrt::box_value(value));
        }
    }

    int32_t ViewModelInstanceProperty::EnumValue()
    {
        if (!ValidateParent()) return 0;

        auto parent = m_parentInstance.get();
        if (!parent) return 0;

#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
        // TODO: Implement when rive viewmodel API is available
        // For now, return placeholder value
#endif

        return 0; // Default value
    }

    void ViewModelInstanceProperty::EnumValue(int32_t value)
    {
        if (!ValidateParent()) return;

        auto parent = m_parentInstance.get();
        if (!parent) return;

        if (parent.SetEnumProperty(m_name, value))
        {
            FireValueChangedEvent(winrt::box_value(value));
        }
    }

    void ViewModelInstanceProperty::Fire()
    {
        if (!ValidateParent()) return;

        auto parent = m_parentInstance.get();
        if (!parent) return;

        if (parent.FireTrigger(m_name))
        {
            FireValueChangedEvent(nullptr); // Triggers don't have values
        }
    }

    bool ViewModelInstanceProperty::IsValid()
    {
        return ValidateParent();
    }

    winrt::event_token ViewModelInstanceProperty::ValueChanged(Windows::Foundation::TypedEventHandler<winrt::WinRive::ViewModelInstanceProperty, Windows::Foundation::IInspectable> const& handler)
    {
        return m_valueChangedEvent.add(handler);
    }

    void ViewModelInstanceProperty::ValueChanged(winrt::event_token const& token) noexcept
    {
        m_valueChangedEvent.remove(token);
    }

    void ViewModelInstanceProperty::SetType(winrt::WinRive::ViewModelPropertyType type)
    {
        m_type = type;
    }

    winrt::WinRive::ViewModelInstance ViewModelInstanceProperty::GetParentInstance() const
    {
        return m_parentInstance.get();
    }

    void ViewModelInstanceProperty::FireValueChangedEvent(Windows::Foundation::IInspectable const& newValue)
    {
        m_valueChangedEvent(*this, newValue);
    }

    bool ViewModelInstanceProperty::ValidateParent() const
    {
        auto parent = m_parentInstance.get();
        return parent && parent.IsValid();
    }
}
