#include "pch.h"
#include "Class.h"
#include "Class.g.cpp"

namespace winrt::WinRive::implementation
{
    int32_t Class::MyProperty()
    {
        return value;
    }

    void Class::MyProperty(int32_t theval)
    {
        value = theval;
    }
}
