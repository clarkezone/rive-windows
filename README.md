# rive-windows
Project to integrate Rive into Windows UI libraries.  Rough goal is to create a Rive contentisland using system compositor visual hosting, CompositionSwapchain for efficiency and the Rive DX11 backend in order to be able to easily host Rive content inside of classic win32, UWP frameworkless, UWP XAML, WinUI3 targets with full z-order, transform and input support

# references
1. Rive Windows DX11 backend: https://github.com/rive-app/rive-runtime/blob/main/renderer/src/d3d/render_context_d3d_impl.cpp
2. Rive Windows sample: https://github.com/rive-app/rive-runtime/blob/main/renderer/path_fiddle/fiddle_context_d3d.cpp
3. CompositionSwapchain: https://learn.microsoft.com/en-us/windows/win32/comp_swapchain/comp-swapchain
4. Windows UI Islands: how to consume: https://github.com/microsoft/WindowsAppSDK-Samples/tree/main/Samples/Islands

# rough staging
1. Build all sample projects
