#!/usr/bin/env pwsh

# Build Release Package Script for WinRive
# This script builds the WinRive components in Release configuration and packages them into a NuGet package

param(
    [string]$Configuration = "Release",
    [string]$Platform = "x64",
    [switch]$AllPlatforms,
    [switch]$PackageOnly
)

Write-Host "WinRive Release Package Builder" -ForegroundColor Green
Write-Host "==============================" -ForegroundColor Green

$ErrorActionPreference = "Stop"
$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path

# Set working directory
Set-Location $projectRoot

if (-not $PackageOnly) {
    Write-Host "Building WinRive components..." -ForegroundColor Yellow
    
    if ($AllPlatforms) {
        $platforms = @("x64", "x86", "ARM64")
    } else {
        $platforms = @($Platform)
    }
    
    foreach ($plat in $platforms) {
        Write-Host "Building for platform: $plat" -ForegroundColor Cyan
        
        # Build the native WinRT component
        Write-Host "  Building WinRive native component..."
        msbuild WinRive\WinRive.vcxproj /p:Configuration=$Configuration /p:Platform=$plat /p:BuildProjectReferences=false
        
        if ($LASTEXITCODE -ne 0) {
            Write-Error "Failed to build WinRive native component for $plat"
            exit $LASTEXITCODE
        }
    }
    
    # Build the C# projection (use x64 platform for the primary build)
    Write-Host "Building WinRive C# projection..." -ForegroundColor Cyan
    msbuild WinRiveProjection\WinRiveProjection.csproj /p:Configuration=$Configuration /p:Platform=x64 /p:BuildProjectReferences=false
    
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Failed to build WinRive C# projection"
        exit $LASTEXITCODE
    }
}

# Validate that required files exist before packaging
Write-Host "Validating release artifacts..." -ForegroundColor Yellow

$requiredFiles = @(
    "x64\$Configuration\WinRive\WinRive.winmd",
    "x64\$Configuration\WinRive\WinRive.dll",
    "WinRiveProjection\bin\x64\$Configuration\net9.0-windows10.0.26100.0\WinRiveProjection.dll",
    "WinRiveProjection\bin\x64\$Configuration\net9.0-windows10.0.26100.0\WinRive.dll"
)

if ($AllPlatforms) {
    $requiredFiles += @(
        "x86\$Configuration\WinRive\WinRive.dll",
        "ARM64\$Configuration\WinRive\WinRive.dll"
    )
}

$missingFiles = @()
foreach ($file in $requiredFiles) {
    if (-not (Test-Path $file)) {
        $missingFiles += $file
    }
}

if ($missingFiles.Count -gt 0) {
    Write-Host "Missing required files for packaging:" -ForegroundColor Red
    foreach ($file in $missingFiles) {
        Write-Host "  - $file" -ForegroundColor Red
    }
    Write-Error "Cannot package without all required files. Please build the missing components first."
    exit 1
}

# Create NuGet package
Write-Host "Creating NuGet package..." -ForegroundColor Yellow

# Check for local nuget.exe first, then PATH, then download
$localNuget = Join-Path $projectRoot "nuget.exe"
if (Test-Path $localNuget) {
    $nugetPath = $localNuget
    Write-Host "Using local nuget.exe" -ForegroundColor Green
} else {
    $nugetPath = Get-Command nuget.exe -ErrorAction SilentlyContinue
    if (-not $nugetPath) {
        Write-Host "NuGet.exe not found in PATH. Attempting to download..." -ForegroundColor Yellow
        $nugetUrl = "https://dist.nuget.org/win-x86-commandline/latest/nuget.exe"
        $nugetExe = Join-Path $env:TEMP "nuget.exe"
        
        try {
            Invoke-WebRequest -Uri $nugetUrl -OutFile $nugetExe
            $nugetPath = $nugetExe
        } catch {
            Write-Error "Failed to download nuget.exe. Please install NuGet CLI tools."
            exit 1
        }
    } else {
        $nugetPath = $nugetPath.Source
    }
}

# Pack the NuGet package
& $nugetPath pack WinRive.nuspec -OutputDirectory nupkg

if ($LASTEXITCODE -ne 0) {
    Write-Error "Failed to create NuGet package"
    exit $LASTEXITCODE
}

Write-Host "Package created successfully!" -ForegroundColor Green
Write-Host "Output directory: nupkg\" -ForegroundColor Green

# List created packages
Get-ChildItem nupkg\*.nupkg | Sort-Object LastWriteTime -Descending | Select-Object -First 5 | ForEach-Object {
    Write-Host "  $($_.Name) ($(Get-Date $_.LastWriteTime -Format 'yyyy-MM-dd HH:mm'))" -ForegroundColor Gray
}

Write-Host "Done!" -ForegroundColor Green
