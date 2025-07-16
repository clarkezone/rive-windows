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
        
        // Initialize the Rive renderer with CoreWindow for input handling
        bool InitializeWithCoreWindow(winrt::Windows::UI::Composition::Compositor const& compositor,
                                    winrt::Windows::UI::Core::CoreWindow const& window,
                                    int32_t width, int32_t height);
        
        // New initialization methods for different hosting scenarios
        bool InitializeForUWP(winrt::Windows::UI::Composition::Compositor const& compositor,
                              winrt::Windows::UI::Core::CoreWindow const& window,
                              int32_t width, int32_t height);
                              
        bool InitializeForWinUI3(winrt::Windows::UI::Composition::Compositor const& compositor,
                                 int32_t width, int32_t height);
                                 
        bool InitializeForWin32(winrt::Windows::UI::Composition::Compositor const& compositor,
                                uint64_t hwnd,
                                int32_t width, int32_t height);
        
        // Get the current hosting mode
        winrt::WinRive::HostingMode GetHostingMode();
        
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

        // State machine enumeration
        winrt::Windows::Foundation::Collections::IVectorView<winrt::WinRive::StateMachineInfo> GetStateMachines();
        winrt::WinRive::StateMachineInfo GetDefaultStateMachine();
        int32_t GetStateMachineCount();

        // State machine control
        bool SetActiveStateMachine(int32_t index);
        bool SetActiveStateMachineByName(hstring const& name);
        int32_t GetActiveStateMachineIndex();

        // State machine playback control
        void PlayStateMachine();
        void PauseStateMachine();
        void ResetStateMachine();
        bool IsStateMachineActive();

        // Input control
        winrt::Windows::Foundation::Collections::IVectorView<winrt::WinRive::StateMachineInput> GetStateMachineInputs();
        bool SetBooleanInput(hstring const& inputName, bool value);
        bool SetNumberInput(hstring const& inputName, double value);
        bool FireTrigger(hstring const& inputName);

    private:
        // The Rive renderer instance
        std::unique_ptr<RiveRenderer> m_riveRenderer;
        
        // Current hosting mode
        winrt::WinRive::HostingMode m_hostingMode{ winrt::WinRive::HostingMode::UWP_CoreWindow };
        
        // Visual bounds
        int32_t m_width{ 0 };
        int32_t m_height{ 0 };
        
        // Win32 hosting specific
        HWND m_hwnd{ nullptr };
        
        // Backward compatibility - CoreWindow support
        winrt::Windows::UI::Core::CoreWindow m_coreWindow{ nullptr };
        winrt::event_token m_pointerMovedToken;
        winrt::event_token m_pointerPressedToken;
        winrt::event_token m_pointerReleasedToken;
        
        // Placeholder for future input provider
        void* m_inputProvider{ nullptr };
        
        // Internal initialization
        bool InitializeCommon(winrt::Windows::UI::Composition::Compositor const& compositor, 
                             int32_t width, int32_t height);
        
        // Legacy event handlers for backward compatibility
        void OnPointerMoved(winrt::Windows::UI::Core::CoreWindow const& sender,
                           winrt::Windows::UI::Core::PointerEventArgs const& args);
        void OnPointerPressed(winrt::Windows::UI::Core::CoreWindow const& sender,
                             winrt::Windows::UI::Core::PointerEventArgs const& args);
        void OnPointerReleased(winrt::Windows::UI::Core::CoreWindow const& sender,
                              winrt::Windows::UI::Core::PointerEventArgs const& args);
        
        // Helper methods
        bool IsPointInBounds(winrt::Windows::Foundation::Point const& point);
        void CleanupInput();
    };
}

namespace winrt::WinRive::factory_implementation
{
    struct RiveControl : RiveControlT<RiveControl, implementation::RiveControl>
    {
    };
}
