# WinRive NuGet Package Implementation Plan

## Overview

This document outlines the implementation plan for packaging the WinRive and WinRiveProjection projects into a distributable NuGet package while preserving the existing development workflow.

## Project Analysis

### Current Structure
- **WinRive** (C++/WinRT Component): Native WinRT component that generates WinRive.winmd and WinRive.dll
- **WinRiveProjection** (C# Projection): .NET 9 project that generates C# projections from WinRive.winmd using CSWinRT
- **Host Projects**: CSWinUI3XAMLHost, CSXamlHost, CPPWin32Host - use direct project references for development

### Key Requirements
1. Maintain existing development workflow (project references)
2. Integrate NuGet packaging into normal build without interference
3. Bundle all Rive runtime dependencies in the package
4. Target external developers for public distribution

## Implementation Plan

### Phase 1: Create NuGet Package Structure ✅ **COMPLETED**

#### Task 1.1: Create NuSpec File ✅ **COMPLETED**
- [x] Create `WinRive.nuspec` in root directory
- [x] Define package metadata (id, version, authors, description)
- [x] Specify dependencies (Microsoft.Windows.CsWinRT, Microsoft.Windows.SDK.BuildTools)
- [x] Map file locations for multi-platform support
- [x] Include MSBuild integration files

**File Structure:**
```
WinRive.nupkg/
├── lib/
│   ├── net9.0-windows10.0.26100.0/
│   │   └── WinRiveProjection.dll
│   └── winrt/
│       └── WinRive.winmd
├── runtimes/
│   ├── win-x86/native/
│   ├── win-x64/native/
│   └── win-arm64/native/
│       └── [WinRive.dll + Rive runtime DLLs]
└── build/
    ├── WinRive.props
    └── WinRive.targets
```

#### Task 1.2: Create MSBuild Integration Files ✅ **COMPLETED**
- [x] Create `build/WinRive.props` - Define common properties for consumers
- [x] Create `build/WinRive.targets` - Handle platform-specific native library deployment
- [x] Ensure automatic deployment of correct native libraries per platform

### Phase 2: Modify Project Files ✅ **COMPLETED**

#### Task 2.1: Update WinRive.vcxproj ✅ **COMPLETED**
- [x] Add conditional NuGet packaging properties using `BuildingForNuGet` flag
- [x] Add package metadata (PackageId, PackageVersion, Authors, Description)
- [x] Create post-build target to stage Rive runtime libraries
- [x] Ensure multi-platform build support (x86, x64, ARM64)
- [x] Verify existing build behavior remains unchanged

**Key Changes:**
```xml
<PropertyGroup Condition="'$(BuildingForNuGet)' == 'true'">
  <GeneratePackageOnBuild>true</GeneratePackageOnBuild>
  <PackageId>WinRive</PackageId>
  <PackageVersion>1.0.0</PackageVersion>
</PropertyGroup>
```

#### Task 2.2: Update WinRiveProjection.csproj ✅ **COMPLETED**
- [x] Add conditional NuGet packaging properties
- [x] Configure symbol package generation (IncludeSymbols, SymbolPackageFormat)
- [x] Ensure WinRT metadata inclusion in package
- [x] Maintain existing project reference to WinRive.vcxproj

#### Task 2.3: Handle Rive Runtime Dependencies ✅ **COMPLETED**
- [x] Identify all required Rive runtime libraries from hardcoded paths
- [x] Create staging mechanism to copy these libraries during NuGet build
- [x] Ensure platform-specific libraries are correctly mapped
- [x] Test that bundled libraries work independently of development environment

### Phase 3: Build System Integration

#### Task 3.1: Create Package Generation Script
- [ ] Create `pack-nuget.ps1` PowerShell script
- [ ] Build all configurations (Release x86, x64, ARM64) with `BuildingForNuGet=true`
- [ ] Execute nuget pack command with proper output directory
- [ ] Add error handling and validation

#### Task 3.2: Verify Non-Disruptive Integration
- [ ] Test that default builds work exactly as before
- [ ] Verify host projects (CSWinUI3XAMLHost, etc.) continue to build and run normally
- [ ] Confirm debugging experience remains unchanged
- [ ] Validate project references continue to work in development

### Phase 4: Testing and Validation

#### Task 4.1: Package Generation Testing
- [ ] Execute package generation script successfully
- [ ] Verify generated package contains all required files
- [ ] Check platform-specific native libraries are correctly placed
- [ ] Validate package metadata and dependencies

#### Task 4.2: Package Consumption Testing
- [ ] Create test project that consumes the generated NuGet package
- [ ] Verify C# projection APIs are available and functional
- [ ] Test on all supported platforms (x86, x64, ARM64)
- [ ] Confirm native libraries deploy correctly to output directory

#### Task 4.3: Compatibility Testing
- [ ] Build and run existing host projects to ensure no regression
- [ ] Test both Debug and Release configurations
- [ ] Verify hot-reload and debugging still work in host projects
- [ ] Test on clean development environment

### Phase 5: Documentation and Distribution

#### Task 5.1: Create Package Documentation
- [ ] Write package README with usage examples
- [ ] Document API surface for external developers
- [ ] Create sample project demonstrating package usage
- [ ] Add troubleshooting guide

#### Task 5.2: Prepare for Distribution
- [ ] Configure package versioning strategy
- [ ] Set up package signing (if required)
- [ ] Prepare release notes template
- [ ] Configure CI/CD integration for automated packaging

## Implementation Checklist

### Prerequisites
- [ ] Visual Studio 2022 with C++/WinRT workload
- [ ] .NET 9 SDK installed
- [ ] NuGet CLI tools available
- [ ] Access to Rive runtime libraries

### Core Implementation
- [x] Create WinRive.nuspec file
- [x] Create build/WinRive.props file  
- [x] Create build/WinRive.targets file
- [x] Update WinRive.vcxproj with conditional packaging properties
- [x] Update WinRiveProjection.csproj with conditional packaging properties
- [ ] Create pack-nuget.ps1 script
- [ ] Test package generation process

### Validation
- [x] Verify existing development workflow unaffected
- [x] Test host projects build and run normally
- [ ] Generate NuGet package successfully
- [ ] Test package consumption in new project
- [ ] Validate multi-platform support
- [ ] Confirm all Rive dependencies included

### Final Steps
- [ ] Create package documentation
- [ ] Test on clean environment
- [ ] Prepare for public distribution
- [ ] Update project README with packaging information

## Success Criteria

1. **Zero Development Impact**: Existing host projects continue to build, run, and debug exactly as before
2. **Successful Package Generation**: NuGet package builds successfully with all platforms and dependencies
3. **External Consumption**: External developers can install package and use WinRive APIs immediately
4. **Self-Contained**: Package includes all necessary Rive runtime dependencies
5. **Multi-Platform**: Package works correctly on x86, x64, and ARM64 platforms

## Risk Mitigation

- **Backup Strategy**: All changes use conditional compilation, allowing easy rollback
- **Testing Strategy**: Comprehensive testing ensures no regression in existing functionality
- **Incremental Approach**: Implementation can be done incrementally with validation at each step
- **Documentation**: Clear documentation enables team members to understand and maintain the packaging system

## Timeline Estimate

- **Phase 1**: 1-2 days (Create package structure and files)
- **Phase 2**: 2-3 days (Modify project files and test)
- **Phase 3**: 1 day (Build system integration)
- **Phase 4**: 2-3 days (Testing and validation)
- **Phase 5**: 1-2 days (Documentation and preparation)

**Total Estimated Time**: 7-11 days

This plan ensures a smooth implementation that preserves the existing development experience while adding professional NuGet packaging capabilities for external distribution.
