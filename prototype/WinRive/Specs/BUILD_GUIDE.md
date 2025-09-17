# WinRive Build Guide

This guide explains how to build WinRive for release distribution and create NuGet packages.

## Prerequisites

- Visual Studio 2022 with Windows 10/11 SDK
- .NET 9.0 SDK
- MSBuild (included with Visual Studio)
- PowerShell 5.1 or PowerShell Core

## Building for Release

### Option 1: Using the Build Script (Recommended)

The project includes a PowerShell script that automates the entire build and packaging process:

```powershell
# Build for x64 only and create package
.\build-release-package.ps1

# Build for all platforms (x64, x86, ARM64) and create package
.\build-release-package.ps1 -AllPlatforms

# Only create package (assumes components are already built)
.\build-release-package.ps1 -PackageOnly

# Build for specific platform
.\build-release-package.ps1 -Platform x86
```

### Option 2: Manual Build Process

#### Step 1: Build Native WinRT Component

```powershell
# Build for x64
msbuild WinRive\WinRive.vcxproj /p:Configuration=Release /p:Platform=x64

# Build for x86
msbuild WinRive\WinRive.vcxproj /p:Configuration=Release /p:Platform=x86

# Build for ARM64
msbuild WinRive\WinRive.vcxproj /p:Configuration=Release /p:Platform=ARM64
```

#### Step 2: Build C# Projection

```powershell
# Use MSBuild with explicit platform to avoid C++ dependency issues
msbuild WinRiveProjection\WinRiveProjection.csproj /p:Configuration=Release /p:Platform=x64 /p:BuildProjectReferences=false
```

#### Step 3: Create NuGet Package

```powershell
# Download nuget.exe if not available
# Create package
nuget pack WinRive.nuspec -OutputDirectory nupkg
```

Or use dotnet CLI:

```powershell
# For projects that support it
dotnet pack WinRiveProjection\WinRiveProjection.csproj -c Release -o nupkg
```

## Release Configuration Changes

The following changes have been made to support release builds:

### Updated WinRive.nuspec

- Changed all Debug paths to Release paths
- Added support for multiple platforms (x86, x64, ARM64)
- Updated native library paths for all supported platforms

### Build Artifacts Structure

After a successful release build, the following structure is expected:

```
WinRive/
├── x64/Release/WinRive/
│   ├── WinRive.winmd     # WinRT metadata
│   └── WinRive.dll       # x64 native library
├── x86/Release/WinRive/
│   └── WinRive.dll       # x86 native library
├── ARM64/Release/WinRive/
│   └── WinRive.dll       # ARM64 native library
├── WinRiveProjection/bin/Release/net9.0-windows10.0.26100.0/
│   ├── WinRiveProjection.dll  # C# projection assembly
│   └── WinRive.dll            # Generated WinRT interop assembly
└── nupkg/
    └── WinRive.1.0.6.nupkg    # Final NuGet package
```

## Platform Support

The release configuration supports the following platforms:

- **x64**: Primary development and testing platform
- **x86**: For compatibility with 32-bit applications
- **ARM64**: For ARM64 devices (Surface Pro X, etc.)

## MSBuild Integration

The NuGet package includes MSBuild targets that automatically:

1. Reference the correct platform-specific native libraries
2. Copy native DLLs to output directory for non-UWP apps
3. Reference WinRT metadata for UWP apps
4. Reference C# projection assemblies for .NET apps

## Troubleshooting

### Missing Build Artifacts

If the build script reports missing files:

1. Ensure Visual Studio is properly installed with Windows SDK
2. Check that the correct platform toolset is installed
3. Verify that .NET 9.0 SDK is installed
4. Run the build commands manually to see detailed error messages

### NuGet Packaging Issues

If packaging fails:

1. Ensure all referenced files in WinRive.nuspec exist
2. Check that paths in the nuspec file match actual build output locations
3. Verify that nuget.exe is available or install NuGet CLI tools

### C# Projection Build Issues

If you encounter the error:
```
error MSB4278: The imported file "$(VCTargetsPath)\Microsoft.Cpp.Default.props" does not exist and appears to be part of a Visual Studio component
```

**Solution**: Use MSBuild instead of `dotnet build` for the C# projection with explicit platform:
```powershell
# Instead of: dotnet build WinRiveProjection\WinRiveProjection.csproj -c Release
# Use: 
msbuild WinRiveProjection\WinRiveProjection.csproj /p:Configuration=Release /p:Platform=x64 /p:BuildProjectReferences=false
```

This occurs because the C# projection has a project reference to the C++ WinRT component, and `dotnet build` doesn't handle mixed C++/C# solutions well.

### WinMD File Path Issues

If you encounter the error:
```
Path 'C:\...\WinRive\AnyCPU\Release\Merged\WinRive.winmd' is not a file or directory
```

**Solution**: Ensure you specify the correct platform when building the C# projection. The C# projection expects the WinMD file at a platform-specific path. Always build the C# projection with an explicit platform (e.g., `/p:Platform=x64`) that matches one of the native builds.

### Platform-Specific Build Issues

For ARM64 builds:
- Ensure ARM64 build tools are installed in Visual Studio
- May require Windows 11 SDK for best ARM64 support

For x86 builds:
- Ensure 32-bit toolchains are installed
- Some dependencies may need 32-bit versions

## Version Management

To update the package version:

1. Update the `<version>` element in WinRive.nuspec
2. Update version in any related project files
3. Update the build script if version references are hardcoded
4. Update release notes in the nuspec file

## Distribution

The generated NuGet package can be:

1. Published to nuget.org for public distribution
2. Published to private NuGet feeds for internal use
3. Distributed as local packages for testing

Example of local installation:
```powershell
nuget install WinRive -Source "path\to\nupkg\directory"
