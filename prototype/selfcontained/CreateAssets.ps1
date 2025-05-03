# PowerShell script to create placeholder PNG files for UWP assets
# This script uses System.Drawing to create simple colored rectangles as placeholder images

Add-Type -AssemblyName System.Drawing

# Function to create a simple colored rectangle image
function Create-PlaceholderImage {
    param (
        [string]$FilePath,
        [int]$Width,
        [int]$Height,
        [System.Drawing.Color]$Color
    )
    
    $bitmap = New-Object System.Drawing.Bitmap($Width, $Height)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    
    # Fill the entire bitmap with the specified color
    $brush = New-Object System.Drawing.SolidBrush($Color)
    $graphics.FillRectangle($brush, 0, 0, $Width, $Height)
    
    # Add some text to identify the image
    $font = New-Object System.Drawing.Font("Arial", [math]::Min($Width, $Height) / 10)
    $textBrush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::White)
    $stringFormat = New-Object System.Drawing.StringFormat
    $stringFormat.Alignment = [System.Drawing.StringAlignment]::Center
    $stringFormat.LineAlignment = [System.Drawing.StringAlignment]::Center
    
    $text = "$Width x $Height"
    $rect = New-Object System.Drawing.RectangleF(0, 0, $Width, $Height)
    $graphics.DrawString($text, $font, $textBrush, $rect, $stringFormat)
    
    # Save the bitmap as PNG
    $bitmap.Save($FilePath, [System.Drawing.Imaging.ImageFormat]::Png)
    
    # Clean up
    $graphics.Dispose()
    $bitmap.Dispose()
    $brush.Dispose()
    $textBrush.Dispose()
    $font.Dispose()
    
    Write-Host "Created $FilePath"
}

# Create the Assets directory if it doesn't exist
$assetsDir = ".\Assets"
if (-not (Test-Path $assetsDir)) {
    New-Item -ItemType Directory -Path $assetsDir | Out-Null
}

# Create the required UWP assets
Create-PlaceholderImage -FilePath "$assetsDir\Square44x44Logo.png" -Width 44 -Height 44 -Color ([System.Drawing.Color]::Blue)
Create-PlaceholderImage -FilePath "$assetsDir\Square150x150Logo.png" -Width 150 -Height 150 -Color ([System.Drawing.Color]::Green)
Create-PlaceholderImage -FilePath "$assetsDir\Wide310x150Logo.png" -Width 310 -Height 150 -Color ([System.Drawing.Color]::Purple)
Create-PlaceholderImage -FilePath "$assetsDir\StoreLogo.png" -Width 50 -Height 50 -Color ([System.Drawing.Color]::Orange)
Create-PlaceholderImage -FilePath "$assetsDir\SplashScreen.png" -Width 620 -Height 300 -Color ([System.Drawing.Color]::Teal)

Write-Host "All placeholder assets created successfully."
