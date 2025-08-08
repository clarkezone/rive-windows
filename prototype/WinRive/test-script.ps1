# Test script to verify WinRive NuGet package installation
Write-Host "Testing WinRive NuGet package installation..." -ForegroundColor Green

# Create a temporary test project directory
$testDir = "temp-test-project"
if (Test-Path $testDir) {
    Remove-Item -Recurse -Force $testDir
}
New-Item -ItemType Directory -Path $testDir | Out-Null

# Change to test directory
Push-Location $testDir

try {
    Write-Host "Creating test .NET project..." -ForegroundColor Yellow
    
    # Create a simple .NET 9 project
    dotnet new console --framework net9.0 --name TestWinRive
    
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to create .NET project"
    }
    
    Set-Location TestWinRive
    
    # Modify the project file to target Windows
    Write-Host "Modifying project to target Windows..." -ForegroundColor Yellow
    $csprojContent = @"
<Project Sdk="Microsoft.NET.Sdk">

  <PropertyGroup>
    <OutputType>Exe</OutputType>
    <TargetFramework>net9.0-windows10.0.26100.0</TargetFramework>
    <Nullable>enable</Nullable>
    <UseWindowsForms>true</UseWindowsForms>
  </PropertyGroup>

</Project>
"@
    $csprojContent | Out-File -FilePath "TestWinRive.csproj" -Encoding UTF8
    
    Write-Host "Adding WinRive package from local source..." -ForegroundColor Yellow
    
    # Add our local NuGet package
    $packagePath = "../../nupkg/WinRive.1.0.0.nupkg"
    dotnet add package WinRive --source "../../nupkg" --version 1.0.0
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "SUCCESS: WinRive package installed successfully!" -ForegroundColor Green
        
        # Try to build the project
        Write-Host "Attempting to build project with WinRive..." -ForegroundColor Yellow
        dotnet build
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "SUCCESS: Project built successfully with WinRive package!" -ForegroundColor Green
        } else {
            Write-Host "WARNING: Project build failed, but package installation succeeded" -ForegroundColor Yellow
        }
    } else {
        Write-Host "FAILED: Could not install WinRive package" -ForegroundColor Red
    }
    
} catch {
    Write-Host "ERROR: $($_.Exception.Message)" -ForegroundColor Red
} finally {
    # Clean up
    Pop-Location
    if (Test-Path $testDir) {
        Remove-Item -Recurse -Force $testDir
    }
    Write-Host "Test completed." -ForegroundColor Green
}
