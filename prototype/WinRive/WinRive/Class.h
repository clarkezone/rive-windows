#pragma once

#include "Class.g.h"

namespace winrt::WinRive::implementation
{
    struct Class : ClassT<Class>
    {
        int32_t value;
        Class() = default;

        int32_t MyProperty();
        void MyProperty(int32_t value);
    };
}

namespace winrt::WinRive::factory_implementation
{
    struct Class : ClassT<Class, implementation::Class>
    {
    };
}
