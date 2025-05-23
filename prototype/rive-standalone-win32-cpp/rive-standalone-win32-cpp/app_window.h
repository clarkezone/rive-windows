#pragma once
#include "pch.h"
#include "win32_window.h"

class AppWindow : public Win32Window {
protected:
    void OnPointerDown(int x, int y) override;
    void OnDpiChanged(int dpi) override;
    void OnResize(int width, int height) override;

private:
    int current_dpi_ = 96;  // Track DPI for use in OnPointerDown
    friend class Win32Window;
};