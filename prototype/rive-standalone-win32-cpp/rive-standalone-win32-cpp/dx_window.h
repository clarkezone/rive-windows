#pragma once
#include "pch.h"
#include "win32_window.h"
using namespace winrt;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Composition::Desktop;

class DXWindow : public Win32Window {
private:
    DesktopWindowTarget m_target{ nullptr };
    Windows::System::DispatcherQueueController m_controller{ nullptr };

protected:

    DXWindow();
    virtual ~DXWindow();

    virtual void OnPointerDown(int x, int y) override;
    virtual void OnDpiChanged(int dpi) override;
    virtual void OnResize(int width, int height) override;
    virtual void WindowCreated() override;
    Windows::System::DispatcherQueueController CreateDispatcherQueueCont();
    void PrepareVisuals();
    void AddVisual(VisualCollection const& visuals, float x, float y);
};