#pragma once

// Prevent Windows macros from conflicting with C++ standard library
#ifndef NOMINMAX
#define NOMINMAX
#endif

// Force inclusion of all Win32 APIs by undefining WINRT_LEAN_AND_MEAN temporarily
#pragma push_macro("WINRT_LEAN_AND_MEAN")
#undef WINRT_LEAN_AND_MEAN

// Standard WinRT headers
#include <unknwn.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>

// Restore the WINRT_LEAN_AND_MEAN setting
#pragma pop_macro("WINRT_LEAN_AND_MEAN")

// Additional WinRT headers needed for Rive renderer
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Input.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.ApplicationModel.h>

// Include the shared Rive renderer
#include "../../shared/rive_renderer.h"
