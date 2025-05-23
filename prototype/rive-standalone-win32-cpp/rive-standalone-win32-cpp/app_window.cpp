#include "pch.h"
#include "app_window.h"

void AppWindow::OnPointerDown(int x, int y) {
    auto scale = GetDpiScale();
    std::cout << "Mouse click at: (" << x << ", " << y << ") - DPI: "
        << current_dpi_ << " (" << static_cast<int>(scale * 100) << "% scale)\n";
}

void AppWindow::OnDpiChanged(int dpi) {
    current_dpi_ = dpi;
    std::cout << "DPI changed to: " << dpi << "\n";
}

void AppWindow::OnResize(int width, int height) {
    std::cout << "Window resized to: " << width << "x" << height << "\n";
}
