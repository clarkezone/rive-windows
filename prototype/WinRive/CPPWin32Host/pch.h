#pragma once

// Prevent Windows macros from conflicting with C++ standard library
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// Windows headers
#include <windows.h>
#include <windowsx.h>
#include <DispatcherQueue.h>

// WinRT headers
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <windows.ui.composition.interop.h>

// WinRive component
#include <winrt/WinRive.h>

// C++ Standard Library headers
#include <iostream>
#include <memory>

// External image base for module handle
extern "C" IMAGE_DOS_HEADER __ImageBase;
