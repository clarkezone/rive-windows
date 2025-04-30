# references
1. Rive Windows DX11 backend: https://github.com/rive-app/rive-runtime/blob/main/renderer/src/d3d/render_context_d3d_impl.cpp
2. Rive Windows sample: https://github.com/rive-app/rive-runtime/blob/main/renderer/path_fiddle/fiddle_context_d3d.cpp
3. CompositionSwapchain: https://learn.microsoft.com/en-us/windows/win32/comp_swapchain/comp-swapchain
4. Windows UI Islands: how to consume: https://github.com/microsoft/WindowsAppSDK-Samples/tree/main/Samples/Islands

# Experiments

Build Rive Windows DX11 backend
Outcome: instructions for building DX11 backend and running samples
Plan:


# Prototypes

## Self contained UWP Rive
Outcome: TODO including which Rive experience
Plan:
Direct hosting of Rive content in UWP
- Start with SwapChain and DX11 backend built directly into C++ UWP app
- Move Rive impl into C++ WinRT component, host within a C# 9 host app
- Move to CompositionSwapchain

## CompositionSwapchain
Outcome:

## Island Source
Outcome:

## Island-based hosting in more host types
- Win32
- WinUI3
- React Native
- 

## Spectrum usage of Rive for rendering 
