# Overview
This page tracks the list of experiments and prototypes that will inform the project.

# Experiments

## 0.1 Build Rive Windows DX11 backend
Outcome: instructions for building DX11 backend and running samples
Status: DONE

## 0.2 Test path_fiddle with a couple of RIV files
Outcome: list of RIV with screenshots and characteristics (eg input, binding etc) [Fix markdown as part of this]

## 0.3 Test DX12 backend
Outcome: everything still works

## 0.4 Smoke-test for banned APIs by matching linker settings with Flutter UWP engine project
Outcome: list of potential problems

# Prototypes

## 0.1 Standalone Windows Project
Plan: 
- Create a project with empty window using modern techniques, no glfw
- Rebuild path_fiddle stripping all non-windows code and leveraging native windowing
- Figure out how to pick up the libs from the Rive build system
- Ensure swapchain binding
- Use visual hosting, not HWND binding
Outcome: standalone project added to this repo

## 0.2 Minimal Rive DLL
Outcome: a full-trust Windows dll that doesnt import user32.dll or gdi32.dll with minimal exports to support helloworld rive using swapchain as currency for output

## 0.3 Run WACK on DLL
Outcome: Accurate list of components and banned APIs

## 0.4 Minimal Rive DLL appcontainer compliant
Outcome: dll passes WAC or list of banned APIs.
1. Run WACK on all components
2. ForEach component
3. ForEach banned API call sight, add a failfast until compiles and WACK passes
4. Start app and replace failfast with bandaid to enable launch to complete

---
# Everything from here is SPECULATIVE due to unknown unknowns.

## 1. Self contained UWP Rive
Outcome: SwapChain and Rive DX11 backend built directly into C++ UWP app
TODO including which Rive experience
Plan:
- Start with empty frameworkless UWP in C++
- Rendering
- Input
- Rive API integration
- Click
- Mousemove


## 2. WinRT component housing Rive Rendering
Outcome: Move Rive impl into C++ WinRT component, host within a C# 10 host app
Plan:
- Build empty WinRT component with C# frameworkless host
- Triange rendered with win2d
- Bring Rive impl across
- API across WinRT boundary for Rive interactivity
- Resizing
- Host in XAML

## 3. CompositionSwapchain
Outcome:  Trianlge rendered using CompositionSwapchain
Plan:
- Naive approach
- Optimize non animating case to delete backbuffers
- Optimize resize to use progressive buffer sizing

## 4. Island Source
Outcome: C++ island content source that can render a square into a swapchain using D2D, input, focus, transform all work, hosted in WinUI3 host
Plan:
- Build island
- Host in all supported island host frameworks

## 5. Island-based hosting in more host types
- Win32
- WinUI3
- React Native
- what else?

## Spectrum Analizer usage of Rive for rendering 
## Port WinRT component to .NET 10

# references
1. Rive Windows DX11 backend: https://github.com/rive-app/rive-runtime/blob/main/renderer/src/d3d11/render_context_d3d_impl.cpp
2. Rive Windows DX12 backend: [https://github.com/rive-app/rive-runtime/blob/main/renderer/src/d3d11/render_context_d3d_impl.cpp](https://github.com/rive-app/rive-runtime/blob/main/renderer/src/d3d12/render_context_d3d12_impl.cpp)
3. Rive Windows sample: https://github.com/rive-app/rive-runtime/blob/main/renderer/path_fiddle/fiddle_context_d3d.cpp
4. CompositionSwapchain: https://learn.microsoft.com/en-us/windows/win32/comp_swapchain/comp-swapchain
5. Windows UI Islands: how to consume: https://github.com/microsoft/WindowsAppSDK-Samples/tree/main/Samples/Islands
