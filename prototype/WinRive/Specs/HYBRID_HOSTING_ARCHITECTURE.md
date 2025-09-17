# Hybrid Hosting Architecture for WinRive WinRT Component

## Overview

This implementation enables the WinRive WinRT component to work across multiple hosting scenarios:
- **UWP XAML** applications (existing functionality)
- **WinUI3 XAML** applications (enhanced support)
- **Win32** applications (new capability using alternative hosting mechanism)

## Architecture

### Core Components

1. **Input Abstraction Layer** (`InputProvider.h/cpp`)
   - `IInputProvider` interface for unified input handling
   - `CoreWindowInputProvider` for UWP scenarios
   - `Win32InputProvider` for Win32 scenarios
   - `NullInputProvider` for scenarios without input
   - `InputProviderFactory` for creating appropriate providers

2. **Enhanced RiveControl** (`RiveControl.idl/h/cpp`)
   - New `HostingMode` enum: `UWP_CoreWindow`, `WinUI3_Compositor`, `Win32_HWND`
   - Multiple initialization methods for different hosting scenarios
   - Backward compatibility with existing API

3. **Shared Renderer** (`rive_renderer.h/cpp`)
   - Unchanged - continues to provide core Rive rendering functionality
   - Works with Windows.UI.Composition APIs across all hosting modes

### New API Methods

```cpp
// Enhanced WinRT Component API
namespace WinRive
{
    enum HostingMode
    {
        UWP_CoreWindow,     // Traditional UWP with CoreWindow
        WinUI3_Compositor,  // WinUI3 with Compositor only
        Win32_HWND         // Win32 with HWND
    };

    runtimeclass RiveControl
    {
        // New initialization methods
        Boolean InitializeForUWP(Windows.UI.Composition.Compositor compositor, 
                                  Windows.UI.Core.CoreWindow window, 
                                  Int32 width, Int32 height);
                                  
        Boolean InitializeForWinUI3(Windows.UI.Composition.Compositor compositor,
                                     Int32 width, Int32 height);
                                     
        Boolean InitializeForWin32(Windows.UI.Composition.Compositor compositor,
                                    UInt64 hwnd,
                                    Int32 width, Int32 height);
        
        HostingMode GetHostingMode();
        
        // Existing methods remain unchanged for backward compatibility
    };
}
```

## Usage Scenarios

### 1. UWP XAML Applications

```cpp
// Existing code continues to work
_riveControl.InitializeWithCoreWindow(compositor, coreWindow, width, height);

// Or use new explicit method
_riveControl.InitializeForUWP(compositor, coreWindow, width, height);
```

### 2. WinUI3 XAML Applications

```cpp
// New WinUI3 support
var compositor = ElementCompositionPreview.GetElementVisual(container).Compositor;
_riveControl.InitializeForWinUI3(compositor, width, height);
```

### 3. Win32 Applications

```cpp
// Win32 hosting using composition
winrt::WinRive::RiveControl riveControl;
riveControl.InitializeForWin32(compositor, reinterpret_cast<uint64_t>(hwnd), width, height);

// Add to composition tree
auto riveVisual = riveControl.GetVisual();
containerVisual.Children().InsertAtTop(riveVisual);
```

## Input Handling

### Abstracted Input System

The new architecture abstracts input handling through the `IInputProvider` interface:

```cpp
class IInputProvider
{
public:
    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual void SetBounds(int width, int height) = 0;
    virtual void SetInputEventCallback(InputEventCallback callback) = 0;
};
```

### Input Event Flow

1. **UWP**: CoreWindow → CoreWindowInputProvider → Unified InputEvent → RiveRenderer
2. **WinUI3**: XAML Input → NullInputProvider (handled at XAML level)
3. **Win32**: Win32 Messages → Win32InputProvider → Unified InputEvent → RiveRenderer

## Key Benefits

### 1. Unified Codebase
- Single WinRT component works across all hosting scenarios
- Shared rendering logic and state management
- Consistent API surface across platforms

### 2. Backward Compatibility
- Existing UWP and WinUI3 code continues to work unchanged
- Legacy `InitializeWithCoreWindow` method redirects to new implementation

### 3. Flexible Input Handling
- Pluggable input providers support different input mechanisms
- Unified coordinate transformation and event processing
- Easy to extend for new input scenarios

### 4. Win32 Integration
- Enables pure Win32 applications to use WinRT component
- Leverages Windows.UI.Composition for modern rendering
- Supports both ElementCompositionPreview and DesktopWindowTarget

## Implementation Details

### Core Architecture Changes

1. **Input Abstraction**: Replaced direct CoreWindow dependency with pluggable input providers
2. **Hosting Mode Management**: Added mode detection and provider factory pattern
3. **Initialization Flow**: Centralized common initialization with mode-specific input setup
4. **Cleanup Logic**: Enhanced cleanup to handle different provider types

### Threading and Lifecycle

- **Dispatcher Queue**: Proper management across hosting scenarios
- **Composition Threading**: Consistent threading model using Windows.UI.Composition
- **Resource Cleanup**: Mode-aware cleanup of input providers and resources

### Memory Management

- **RAII Pattern**: Input providers use RAII for automatic cleanup
- **Smart Pointers**: std::unique_ptr for automatic memory management
- **WinRT References**: Proper WinRT object lifecycle management

## Example Integration

The provided `Win32RiveExample.cpp` demonstrates:

1. Creating a Win32 window with Windows.UI.Composition support
2. Using the enhanced WinRive component in Win32 mode
3. Handling input and rendering through the unified interface
4. Proper resource cleanup and lifecycle management

## Future Enhancements

### Potential Extensions

1. **Enhanced WinUI3 Input**: Direct WinUI3 input handling through InputActivationListener
2. **Multi-Touch Support**: Extended input providers for touch scenarios
3. **Custom Input Providers**: Framework for application-specific input handling
4. **Performance Optimizations**: Input batching and coordinate caching

### Migration Path

1. **Phase 1**: Use new APIs in new projects
2. **Phase 2**: Gradually migrate existing code to new explicit methods
3. **Phase 3**: Deprecate legacy methods while maintaining compatibility

## Conclusion

This hybrid hosting architecture successfully enables the WinRive WinRT component to work across UWP XAML, WinUI3 XAML, and Win32 scenarios while maintaining backward compatibility and providing a unified development experience.

The architecture demonstrates how to bridge the gap between different Windows application models while leveraging the power of Windows.UI.Composition for modern rendering capabilities.
