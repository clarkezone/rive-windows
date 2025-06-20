#pragma once

#include "RiveControl.g.h"

namespace winrt::WinRive::implementation
{
    struct RiveControl : RiveControlT<RiveControl>
    {
        RiveControl();
        ~RiveControl();

        // Initialize the Rive renderer with a composition visual
        bool Initialize(winrt::Windows::UI::Composition::Compositor const& compositor, int32_t width, int32_t height);
        
        // Get the visual that can be added to the composition tree
        winrt::Windows::UI::Composition::Visual GetVisual();
        
        // Load a Rive file from a path
        bool LoadRiveFile(hstring const& filePath);
        
        // Load a Rive file from a package
        bool LoadRiveFileFromPackage(hstring const& relativePath);
        
        // Control the rendering
        void StartRenderLoop();
        void StopRenderLoop();
        void PauseRendering();
        void ResumeRendering();
        
        // Update the size of the renderer
        void SetSize(int32_t width, int32_t height);
        
        // Clean up resources
        void Shutdown();

    private:
        // The Rive renderer instance
        std::unique_ptr<RiveRenderer> m_riveRenderer;
    };
}

namespace winrt::WinRive::factory_implementation
{
    struct RiveControl : RiveControlT<RiveControl, implementation::RiveControl>
    {
    };
}
