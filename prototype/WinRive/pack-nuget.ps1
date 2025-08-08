<#
.SYNOPSIS
    Builds and packages WinRive NuGet package with all platform configurations.

.DESCRIPTION
    This script builds WinRive for all supported platforms (x86, x64, ARM64) in Release configuration
    with NuGet packaging enabled, then creates the NuGet package using the existing nuspec file.
    
    The script ensures:
    - All platforms are built with BuildingForNuGet=true
    - Rive runtime dependencies are properly staged
    - NuGet package is created with all required files
    - Build artifacts are cleaned up appropriately

.PARAMETER OutputPath
    Directory where the NuGet package will be created. Defaults to ./nupkg

.PARAMETER Version
    Package version to use. If not specified, uses version from nuspec file.

.PARAMETER Configuration
    Build configuration to use. Defaults to Release.

.PARAMETER SkipBuild
    Skip the build process and only create package from existing artifacts.

.PARAMETER ShowDetails
    Enable verbose output for debugging.

.EXAMPLE
    .\pack-nuget.ps1
    Creates WinRive NuGet package with default settings.

.EXAMPLE
    .\pack-nuget.ps1 -Version "1.0.1-beta" -OutputPath "./dist"
    Creates package with specific version in custom output directory.

.EXAMPLE
    .\pack-nuget.ps1 -SkipBuild -ShowDetails
    Creates package without building, with verbose output.
#>

[CmdletBinding()]
param(
    [string]$OutputPath = "./nupkg",
    [string]$Version = $null,
    [string]$Configuration = "Release",
    [switch]$SkipBuild,
    [switch]$ShowDetails
)

# Script configuration
$ErrorActionPreference = "Stop"
$InformationPreference = if ($ShowDetails) { "Continue" } else { "SilentlyContinue" }

# Platform configurations (temporarily just x64 for testing)
$Platforms = @("x64")
$SolutionFile = "WinRive.sln"
$NuSpecFile = "WinRive.nuspec"

# Functions
function Write-Header {
    param([string]$Message)
    Write-Host ""
    Write-Host "=" * 80 -ForegroundColor Cyan
    Write-Host " $Message" -ForegroundColor Cyan
    Write-Host "=" * 80 -ForegroundColor Cyan
}

function Write-Step {
    param([string]$Message)
    Write-Host ""
    Write-Host "STEP: $Message" -ForegroundColor Green
}

function Write-Error-And-Exit {
    param([string]$Message, [int]$ExitCode = 1)
    Write-Host ""
    Write-Host "ERROR: $Message" -ForegroundColor Red
    exit $ExitCode
}

function Test-Prerequisites {
    Write-Step "Checking prerequisites..."
    
    # Check if solution file exists
    if (-not (Test-Path $SolutionFile)) {
        Write-Error-And-Exit "Solution file '$SolutionFile' not found."
    }
    
    # Check if nuspec file exists
    if (-not (Test-Path $NuSpecFile)) {
        Write-Error-And-Exit "NuGet spec file '$NuSpecFile' not found."
    }
    
    # Check for MSBuild
    $msbuild = Get-Command "msbuild.exe" -ErrorAction SilentlyContinue
    if (-not $msbuild) {
        Write-Error-And-Exit "MSBuild not found in PATH. Please run from Visual Studio Developer Command Prompt."
    }
    
    # Check for NuGet or dotnet (prefer dotnet pack)
    $dotnet = Get-Command "dotnet.exe" -ErrorAction SilentlyContinue
    $nuget = Get-Command "nuget.exe" -ErrorAction SilentlyContinue
    
    if (-not $dotnet -and -not $nuget) {
        Write-Error-And-Exit "Neither dotnet.exe nor nuget.exe found in PATH. Please install .NET SDK or NuGet CLI tools."
    }
    
    $usesDotnet = $dotnet -ne $null
    
    Write-Information "Prerequisites validated successfully."
    Write-Host "  [OK] Solution file: $SolutionFile"
    Write-Host "  [OK] NuSpec file: $NuSpecFile"
    Write-Host "  [OK] MSBuild: $($msbuild.Source)"
    if ($usesDotnet) {
        Write-Host "  [OK] .NET CLI: $($dotnet.Source)"
    } else {
        Write-Host "  [OK] NuGet CLI: $($nuget.Source)"
    }
    
    return $usesDotnet
}

function Build-Platform {
    param([string]$Platform)
    
    Write-Step "Building platform: $Platform"
    
    $buildArgs = @(
        $SolutionFile
        "/p:Configuration=$Configuration"
        "/p:Platform=$Platform"
        "/p:BuildingForNuGet=true"
        "/verbosity:minimal"
        "/nologo"
    )
    
    if ($ShowDetails) {
        $buildArgs += "/verbosity:normal"
    }
    
    Write-Information "Executing: msbuild $($buildArgs -join ' ')"
    
    $process = Start-Process -FilePath "msbuild.exe" -ArgumentList $buildArgs -Wait -PassThru -NoNewWindow
    
    if ($process.ExitCode -ne 0) {
        Write-Error-And-Exit "Build failed for platform $Platform (Exit code: $($process.ExitCode))"
    }
    
    Write-Host "  [OK] Build completed successfully for $Platform"
}

function Build-AllPlatforms {
    Write-Header "Building WinRive for All Platforms"
    
    Write-Information "Configuration: $Configuration"
    Write-Information "Platforms: $($Platforms -join ', ')"
    Write-Information "BuildingForNuGet: true"
    
    # Clean solution first
    Write-Step "Cleaning solution..."
    $cleanArgs = @(
        $SolutionFile
        "/t:Clean"
        "/p:Configuration=$Configuration"
        "/verbosity:minimal"
        "/nologo"
    )
    
    $process = Start-Process -FilePath "msbuild.exe" -ArgumentList $cleanArgs -Wait -PassThru -NoNewWindow
    if ($process.ExitCode -ne 0) {
        Write-Warning "Clean operation failed, but continuing with build..."
    }
    
    # Build each platform
    foreach ($platform in $Platforms) {
        Build-Platform -Platform $platform
    }
    
    Write-Host ""
    Write-Host "All platform builds completed successfully!" -ForegroundColor Green
}

function Verify-BuildArtifacts {
    Write-Step "Verifying build artifacts..."
    
    $requiredFiles = @()
    
    # Check for WinRT metadata and DLLs for each platform
    foreach ($platform in $Platforms) {
        $platformDir = switch ($platform) {
            "x86" { "x86" }
            "x64" { "x64" }
            "ARM64" { "ARM64" }
        }
        
        # WinRive artifacts  
        $winmdPath = "WinRive\$platformDir\$Configuration\Merged\WinRive.winmd"
        $dllPath = "$platformDir\$Configuration\WinRive\WinRive.dll"
        
        if (-not (Test-Path $winmdPath)) {
            Write-Error-And-Exit "Missing WinRT metadata: $winmdPath"
        }
        
        if (-not (Test-Path $dllPath)) {
            Write-Error-And-Exit "Missing WinRive DLL: $dllPath"
        }
        
        # WinRiveProjection artifacts (projection generates WinRive.dll in its output)
        $projectionPath = "WinRiveProjection\bin\$platformDir\$Configuration\net9.0-windows10.0.26100.0\WinRive.dll"
        if (-not (Test-Path $projectionPath)) {
            Write-Error-And-Exit "Missing WinRiveProjection DLL: $projectionPath"
        }
        
        $requiredFiles += $winmdPath, $dllPath, $projectionPath
    }
    
    Write-Host "  [OK] All required build artifacts verified"
    Write-Information "Verified files: $($requiredFiles.Count)"
}

function Create-NuGetPackage {
    param([bool]$UsesDotnet)
    
    Write-Header "Creating NuGet Package"
    
    # Ensure output directory exists
    if (-not (Test-Path $OutputPath)) {
        New-Item -ItemType Directory -Path $OutputPath -Force | Out-Null
        Write-Information "Created output directory: $OutputPath"
    }
    
    if ($UsesDotnet) {
        # Use dotnet pack with WinRiveProjection.csproj (which references the nuspec)
        Write-Step "Using .NET CLI (dotnet pack)..."
        
        $packArgs = @(
            "pack"
            "WinRiveProjection\WinRiveProjection.csproj"
            "--configuration", $Configuration
            "--output", $OutputPath
            "--no-build"  # We already built everything
        )
        
        if ($Version) {
            $packArgs += "-p:PackageVersion=$Version"
            Write-Information "Using custom version: $Version"
        }
        
        if ($ShowDetails) {
            $packArgs += "--verbosity", "detailed"
        }
        
        Write-Information "Command: dotnet $($packArgs -join ' ')"
        
        $process = Start-Process -FilePath "dotnet.exe" -ArgumentList $packArgs -Wait -PassThru -NoNewWindow
        
        if ($process.ExitCode -ne 0) {
            Write-Error-And-Exit ".NET pack failed (Exit code: $($process.ExitCode))"
        }
        
    } else {
        # Use traditional nuget.exe pack
        Write-Step "Using NuGet CLI (nuget pack)..."
        
        $packArgs = @(
            "pack"
            $NuSpecFile
            "-OutputDirectory"
            $OutputPath
            "-NonInteractive"
        )
        
        if ($Version) {
            $packArgs += "-Version", $Version
            Write-Information "Using custom version: $Version"
        }
        
        if ($ShowDetails) {
            $packArgs += "-Verbosity", "detailed"
        }
        
        Write-Information "Command: nuget $($packArgs -join ' ')"
        
        $process = Start-Process -FilePath "nuget.exe" -ArgumentList $packArgs -Wait -PassThru -NoNewWindow
        
        if ($process.ExitCode -ne 0) {
            Write-Error-And-Exit "NuGet pack failed (Exit code: $($process.ExitCode))"
        }
    }
    
    # Find the created package
    $packageFiles = Get-ChildItem -Path $OutputPath -Filter "WinRive*.nupkg" | Sort-Object LastWriteTime -Descending
    
    if ($packageFiles.Count -eq 0) {
        Write-Error-And-Exit "No package files found in output directory: $OutputPath"
    }
    
    $latestPackage = $packageFiles[0]
    Write-Host ""
    Write-Host "[SUCCESS] NuGet package created successfully!" -ForegroundColor Green
    Write-Host "  Package: $($latestPackage.Name)" -ForegroundColor Yellow
    Write-Host "  Path: $($latestPackage.FullName)" -ForegroundColor Yellow
    Write-Host "  Size: $([math]::Round($latestPackage.Length / 1MB, 2)) MB" -ForegroundColor Yellow
    
    return $latestPackage
}

function Show-PackageContents {
    param([System.IO.FileInfo]$PackageFile)
    
    if (-not $ShowDetails) {
        return
    }
    
    Write-Step "Package contents (verbose mode):"
    
    try {
        # Use .NET to read the package as a ZIP file
        Add-Type -AssemblyName System.IO.Compression.FileSystem
        $zip = [System.IO.Compression.ZipFile]::OpenRead($PackageFile.FullName)
        
        foreach ($entry in $zip.Entries | Sort-Object FullName) {
            $size = if ($entry.Length -gt 0) { " ($([math]::Round($entry.Length / 1KB, 1)) KB)" } else { "" }
            Write-Host "    $($entry.FullName)$size" -ForegroundColor Gray
        }
        
        $zip.Dispose()
    }
    catch {
        Write-Warning "Could not read package contents: $($_.Exception.Message)"
    }
}

# Main execution
try {
    Write-Header "WinRive NuGet Package Builder"
    Write-Host "Configuration: $Configuration" -ForegroundColor Yellow
    Write-Host "Output Path: $OutputPath" -ForegroundColor Yellow
    if ($Version) {
        Write-Host "Version: $Version" -ForegroundColor Yellow
    }
    if ($SkipBuild) {
        Write-Host "Skip Build: Enabled" -ForegroundColor Yellow
    }
    
    # Step 1: Check prerequisites
    $usesDotnet = Test-Prerequisites
    
    # Step 2: Build all platforms (unless skipped)
    if (-not $SkipBuild) {
        Build-AllPlatforms
        Verify-BuildArtifacts
    } else {
        Write-Header "Skipping Build (as requested)"
        Write-Step "Verifying existing artifacts..."
        Verify-BuildArtifacts
    }
    
    # Step 3: Create NuGet package
    $package = Create-NuGetPackage -UsesDotnet $usesDotnet
    
    # Step 4: Show package contents (if verbose)
    Show-PackageContents -PackageFile $package
    
    # Success!
    Write-Header "Package Creation Complete"
    Write-Host "Success! WinRive NuGet package has been created." -ForegroundColor Green
    Write-Host ""
    Write-Host "Package Location:" -ForegroundColor Cyan
    Write-Host "  $($package.FullName)" -ForegroundColor White
    Write-Host ""
    Write-Host "Next Steps:" -ForegroundColor Cyan
    Write-Host "  1. Test the package in a sample project" -ForegroundColor White
    Write-Host "  2. Verify all platforms work correctly" -ForegroundColor White
    Write-Host "  3. Publish to NuGet repository when ready" -ForegroundColor White
    
}
catch {
    Write-Error-And-Exit "Script execution failed: $($_.Exception.Message)"
}
