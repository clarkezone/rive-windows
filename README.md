# Rive-windows
Project to integrate the [Rive](https://rive.app/) runtime into Windows UI libraries.  Rough goal is to create a Rive `ContentIsland` using system compositor visual hosting, `CompositionSwapchain` for Windows Compositor rendering integration and the Rive DX11 backend in order to be able to easily host Rive content inside of classic win32, UWP frameworkless, UWP XAML, WinUI3, ReactNative targets with alpha transparency, z-order interleaving (no airspace between frameworks), transform and input support.

# references
1. Rive Windows DX11 backend: https://github.com/rive-app/rive-runtime/blob/main/renderer/src/d3d/render_context_d3d_impl.cpp
2. Rive Windows sample: https://github.com/rive-app/rive-runtime/blob/main/renderer/path_fiddle/fiddle_context_d3d.cpp
3. CompositionSwapchain: https://learn.microsoft.com/en-us/windows/win32/comp_swapchain/comp-swapchain
4. Windows UI Islands: how to consume: https://github.com/microsoft/WindowsAppSDK-Samples/tree/main/Samples/Islands

# Direct hosting of Rive content in UWP
- Start with SwapChain and DX11 backend built directly into C++ UWP app
- Move Rive impl into WinRT component, host within a C# 9 host app
- Move to CompositionSwapchain

# Island-based hosting of Rive content in initial host (ideally UWP)

# Island-based hosting in more host types
- Win32
- WinUI3
- React Native

# Next step
1. Build all sample projects

