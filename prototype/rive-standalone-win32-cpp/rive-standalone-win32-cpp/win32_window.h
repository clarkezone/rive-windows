#pragma once
#include "pch.h"

class Win32Window {
public:
    struct Point {
        int x{};
        int y{};
    };

    struct Size {
        int width{};
        int height{};
    };

    Win32Window();
    virtual ~Win32Window();

    // Prevent copying
    Win32Window(const Win32Window&) = delete;
    Win32Window& operator=(const Win32Window&) = delete;

    // Create and show window
    bool Create(std::wstring_view title, const Point& origin, const Size& size);
    void Show();

    // Window properties
    HWND GetHandle() const { return window_handle_; }
    float GetDpiScale() const;

    // Destroy window
    void Destroy();

protected:
    // Subclasses can override these handlers
    virtual void OnPointerDown(int x, int y);
    virtual void OnDpiChanged(int dpi);
    virtual void OnResize(int width, int height);
	virtual void WindowCreated();
    HWND window_handle_ = nullptr;

private:
    static constexpr std::wstring_view kWindowClassName = L"Win32WindowClass";

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    LRESULT MessageHandler(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

    bool RegisterWindowClass();
    void UnregisterWindowClass();
    void UpdateWindowSize(const RECT& rect);

    HINSTANCE instance_ = nullptr;
    std::wstring window_class_name_;
    int current_dpi_ = 96;
    int current_width_ = 0;
    int current_height_ = 0;
    bool class_registered_ = false;
};
