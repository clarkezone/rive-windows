# Overview
This page tracks the list of experiments and prototypes that will inform the project.

# Experiments

## 0.1 Build Rive Windows DX11 backend
Outcome: instructions for building DX11 backend and running samples.

Status: DONE

## 0.2 Test path_fiddle with a couple of RIV files
Outcome: list of RIV with screenshots and characteristics (eg input, binding etc) [Fix markdown as part of this].

Status: In progress, partially complete

# Prototypes

## 0.1 Standalone Windows win32 Project <====== We are here
Outcome:
- standalone project added to this repo that can render Rive content in a Win32 window
- Loading / device initialization / Rendering with animation / windows resizing / no input

- [x] Status: COMPLETED

~Plan:~ 
~1. Create a project with empty window using modern techniques, no glfw~
~2. Rebuild path_fiddle stripping all non-windows code and leveraging native windowing~
~3. Figure out how to pick up the libs from the Rive build system~
~4. Ensure swapchain binding~
~5. Use visual hosting, not HWND binding~
~6. Compiles~
~7. Links~
~8. Runs and draws content~
~9. Window Resizing~
- [ ] Screenshot or video
- [ ] Verify on another machine (ensure riv path is correct)
- [ ] Test in release mode

## 0.2 WinRive component refactor
Outcome:
- WinRive core lib project
- Win32 Test project rewired to lib
- Add win32 test buttons to play, pause, stop, load different scenes

## 0.3 UWP host project
Outcome: 
- WinRive WinRT componet DLL wrapping WinRive lib
- Windows UWP test app launches and renders content using CoreWindow and WinRive component
- Loading / device initialization / Rendering with animation / windows resizing / no input

Status: Not started

---
# Everything from here is SPECULATIVE due to unknown unknowns.
## Remaining Rive features
- Mouse input
- Key input
- DPI handling
- Scripting
- TODO: enumerate rest of rive featureset
- TODO: is there a rive test file with all the things as a goal to get working?

## DirectX12

## CompositionSwapchain
Outcome:  Trianlge rendered using CompositionSwapchain
Plan:
- Naive approach
- Optimize non animating case to delete backbuffers
- Optimize resize to use progressive buffer sizing

## Island Source
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
