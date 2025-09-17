# WinRive - Rive Animation Runtime for Windows

WinRive provides a native Windows Runtime component for integrating Rive animations into Windows applications. This package includes both the native WinRT component and C# projections for .NET 9 applications targeting Windows 10.0.26100.0 and later.

## Features

- **Native WinRT Component**: High-performance Rive animation rendering using native Windows Runtime APIs
- **C# Projections**: Easy integration with .NET applications using automatically generated C# projections
- **Multi-platform Support**: Works on x86, x64, and ARM64 platforms
- **Bundled Dependencies**: All Rive runtime dependencies are included in the package
- **Application Support**: Compatible with WinUI 3, UWP, and Win32 applications
- **Automatic Deployment**: Native libraries are automatically deployed to your application output directory

## Installation

Install the WinRive NuGet package using the Package Manager Console:

```powershell
Install-Package WinRive
```

Or using the .NET CLI:

```bash
dotnet add package WinRive
```

## Quick Start

### C# / .NET Usage

```csharp
using WinRive;

// Create a Rive control
var riveControl = new RiveControl();

// Load a Rive animation file
await riveControl.LoadRiveFileAsync("path/to/your/animation.riv");

// Add to your UI
myPanel.Children.Add(riveControl);
```

### C++/WinRT Usage

```cpp
#include <winrt/WinRive.h>
using namespace winrt::WinRive;

// Create a Rive control
auto riveControl = RiveControl();

// Load a Rive animation file
co_await riveControl.LoadRiveFileAsync(L"path/to/your/animation.riv");

// Add to your UI
myPanel().Children().Append(riveControl);
```

## Supported Platforms

- **Windows 10.0.26100.0** and later
- **x86** (Win32) architecture
- **x64** architecture  
- **ARM64** architecture

## Supported Application Types

- **WinUI 3** applications
- **UWP** (Universal Windows Platform) applications
- **Win32** applications using Windows Runtime

## Package Contents

This NuGet package includes:

- `WinRive.dll` - Native WinRT component
- `WinRive.winmd` - Windows Runtime metadata
- `WinRiveProjection.dll` - C# projections for .NET 9
- Rive runtime dependencies (bundled automatically)
- MSBuild integration files for automatic deployment

## Configuration

The package automatically deploys native libraries to your application's output directory. You can customize this behavior by setting MSBuild properties:

```xml
<PropertyGroup>
  <!-- Disable automatic native library deployment -->
  <WinRiveDeployNativeLibraries>false</WinRiveDeployNativeLibraries>
  
  <!-- Display detailed package information during build -->
  <WinRiveDisplayInfo>true</WinRiveDisplayInfo>
</PropertyGroup>
```

## Requirements

- **Visual Studio 2022** with C++/WinRT workload (for C++ development)
- **.NET 9 SDK** (for C# development)
- **Windows 10 SDK 10.0.26100.0** or later

## Troubleshooting

### Native Libraries Not Found

If you encounter issues with missing native libraries:

1. Ensure your project targets the correct platform (x86, x64, or ARM64)
2. Verify that `WinRiveDeployNativeLibraries` is set to `true` (default)
3. Check that the package was restored correctly: `dotnet restore`

### Platform Mismatch

Ensure your application's target platform matches one of the supported platforms. The package will show a build error for unsupported platforms.

### WinRT Activation Issues

For Win32 applications, ensure your application manifest includes the appropriate WinRT activatable class registrations.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contributing

This is part of the WinRive project. For contributions and issues, please visit the [GitHub repository](https://github.com/clarkezone/rive-windows).
