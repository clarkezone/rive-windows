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

        // Direct input methods for host applications to call
        void QueuePointerMove(float x, float y);
        void QueuePointerPress(float x, float y);
        void QueuePointerRelease(float x, float y);

    private:
        // The Rive renderer instance
        std::unique_ptr<RiveRenderer> m_riveRenderer;
        
        // Visual bounds
        int32_t m_width{ 0 };
        int32_t m_height{ 0 };
    };
}

namespace winrt::WinRive::factory_implementation
{
    struct RiveControl : RiveControlT<RiveControl, implementation::RiveControl>
    {
    };
}
