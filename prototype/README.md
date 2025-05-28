# Overview
This page tracks the list of experiments and prototypes that will inform the project.

# Experiments

## 0.1 Build Rive Windows DX11 backend
Outcome: instructions for building DX11 backend and running samples.
Status: DONE

## 0.2 Test path_fiddle with a couple of RIV files
Outcome: list of RIV with screenshots and characteristics (eg input, binding etc) [Fix markdown as part of this].

Status: In progress, partially complete

## 0.3 Test DX12 backend
Outcome: everything still works  
Status: Defered until later

## 0.4 Smoke-test for banned APIs by matching linker settings with Flutter UWP engine project
Outcome: list of potential problems
Status: not started

# Prototypes

## 0.1 Standalone Windows Project
Outcome: standalone project added to this repo
Status: Step 7 in progress
Plan: 
1. Create a project with empty window using modern techniques, no glfw
2. Rebuild path_fiddle stripping all non-windows code and leveraging native windowing
3. Figure out how to pick up the libs from the Rive build system
4. Ensure swapchain binding
5. Use visual hosting, not HWND binding
6. Compiles
7. Links
8. Runs and draws content
9. Ensure feature parity (wire up all mouse / keyboard input)


## 0.2 Standalone UWP Project
Outcome: a Windows UWP version launches (likely with WACK violations)
Status: Not started
Plan:
1. Build minimal c++ UWP using CoreWindow / CoreApplicationView etc
2. Port rive_window code from Prototype 0.1 on top of UWP windowing and input API's
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
