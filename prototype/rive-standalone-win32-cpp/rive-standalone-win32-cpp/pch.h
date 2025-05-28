#pragma once

// Prevent Windows macros from conflicting with C++ standard library
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

// Windows headers
#include <windows.h>
#include <windowsx.h>
#include <shellscalingapi.h>

#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <windows.ui.composition.interop.h>
#include <ShellScalingAPI.h>
#include <DispatcherQueue.h>
#include <winrt/base.h>

// DirectX headers
#include <d3d11_1.h>
#include <d2d1_1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <dxgi1_2.h>
#include <wincodec.h>
#include <DirectXMath.h>

// C++ Standard Library headers
#include <iostream>
#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <format>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

// Link required libraries
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shcore.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "windowscodecs.lib")
