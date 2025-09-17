#pragma once

// Prevent Windows macros from conflicting with C++ standard library
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// Windows headers
#include <windows.h>

// WinRT headers
#include <winrt/base.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Composition.h>
#include <windows.ui.composition.interop.h>

// DirectX headers
#include <d3d11_1.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

// C++ Standard Library headers
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <fstream>
#include <vector>
#include <iostream>
#include <queue>

// Rive headers (only include if available)
#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
#include "rive/renderer/rive_renderer.hpp"
#include "rive/renderer/d3d11/render_context_d3d_impl.hpp"
#include "rive/renderer/d3d11/d3d11.hpp"
#include "rive/artboard.hpp"
#include "rive/file.hpp"
#include "rive/animation/linear_animation_instance.hpp"
#include "rive/animation/state_machine_instance.hpp"
#include "rive/static_scene.hpp"

#include "rive/viewmodel/viewmodel.hpp"
#include "rive/viewmodel/viewmodel_instance.hpp"
#include "rive/viewmodel/viewmodel_instance_value.hpp"
#include "rive/viewmodel/viewmodel_instance_string.hpp"
#include "rive/viewmodel/viewmodel_instance_number.hpp"
#include "rive/viewmodel/viewmodel_instance_boolean.hpp"
#include "rive/viewmodel/viewmodel_instance_color.hpp"
#include "rive/viewmodel/viewmodel_instance_enum.hpp"
#include "rive/viewmodel/viewmodel_instance_trigger.hpp"
#endif

// Input event structure for thread-safe input handling
struct MouseInputEvent {
    enum Type { Move, Press, Release };
    Type type;
    float x, y;  // Relative to RiveRenderer bounds (0,0 to width,height)
    std::chrono::steady_clock::time_point timestamp;
};

class RiveRenderer {
private:
    // Composition API
    winrt::Windows::UI::Composition::Compositor m_compositor{ nullptr };
    winrt::Windows::UI::Composition::SpriteVisual m_riveVisual{ nullptr };
    
    // DirectX 11 resources
    winrt::com_ptr<::ID3D11Device> m_d3dDevice;
    winrt::com_ptr<::ID3D11DeviceContext1> m_d3dContext;
    winrt::com_ptr<::IDXGISwapChain1> m_swapChain;
    winrt::com_ptr<::ID3D11Texture2D> m_backBuffer;
    winrt::com_ptr<::IDXGIFactory2> m_dxgiFactory;
    
#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
    // Rive rendering resources
    Microsoft::WRL::ComPtr<::ID3D11Device> m_riveGpu;
    Microsoft::WRL::ComPtr<::ID3D11DeviceContext> m_riveGpuContext;
    std::unique_ptr<rive::gpu::RenderContext> m_riveRenderContext;
    rive::rcp<rive::gpu::RenderTargetD3D> m_riveRenderTarget;
    std::unique_ptr<rive::Renderer> m_riveRenderer;
    
    // Rive content
    rive::rcp<rive::File> m_riveFile;
    //std::unique_ptr<rive::File> m_riveFile;
    std::unique_ptr<rive::ArtboardInstance> m_artboard;
    std::unique_ptr<rive::Scene> m_scene;
    rive::rcp<rive::ViewModelInstance> m_viewModelInstance;
    
    // State machine management
    std::vector<std::unique_ptr<rive::StateMachineInstance>> m_stateMachines;
    rive::StateMachineInstance* m_activeStateMachine = nullptr;
    std::vector<rive::rcp<rive::ViewModelInstance>> m_viewModelInstances;
    int m_activeStateMachineIndex = -1;
    int m_defaultStateMachineIndex = -1;
    bool m_stateMachineActive = false;
#endif
    
    // Rive file data
    std::vector<uint8_t> m_riveFileData;
    std::string m_riveFilePath;
    
    // Threading
    std::thread m_renderThread;
    std::atomic<bool> m_shouldRender{ true };
    std::atomic<bool> m_isPaused{ false };
    std::mutex m_deviceMutex;
    
    // Rendering state
    int m_renderWidth = 800;
    int m_renderHeight = 600;
    bool m_deviceLost = false;
    
    // Input event queue system
    std::queue<MouseInputEvent> m_inputQueue;
    std::mutex m_inputQueueMutex;
    
    // Coordinate transformation & alignment
#if defined(WITH_RIVE_TEXT) && defined(RIVE_HEADERS_AVAILABLE)
    rive::Mat2D m_artboardTransform;  // Stores the alignment transformation
#endif
    bool m_transformValid = false;
    bool m_lastPointerDown = false;  // Track pointer state

public:
    RiveRenderer();
    virtual ~RiveRenderer();
    
    // Prevent copying
    RiveRenderer(const RiveRenderer&) = delete;
    RiveRenderer& operator=(const RiveRenderer&) = delete;

    // Initialization
    bool Initialize(const winrt::Windows::UI::Composition::Compositor& compositor, int width = 800, int height = 600);
    void Shutdown();
    
    // Visual management
    winrt::Windows::UI::Composition::SpriteVisual GetVisual() const { return m_riveVisual; }
    void SetSize(int width, int height);
    
    // Content management
    bool LoadRiveFile(const std::string& filePath);
    
    // Rendering control
    void StartRenderThread();
    void StopRenderThread();
    void PauseRendering();
    void ResumeRendering();
    
    // Input handling - coordinates should be relative to renderer bounds
    void QueuePointerMove(float x, float y);
    void QueuePointerPress(float x, float y);  
    void QueuePointerRelease(float x, float y);

    // State machine management
    struct StateMachineInfo {
        std::string name;
        int index;
        bool isDefault;
    };
    
    struct StateMachineInputInfo {
        std::string name;
        std::string type;
        bool booleanValue;
        double numberValue;
    };
    
    std::vector<StateMachineInfo> EnumerateStateMachines();
    StateMachineInfo GetDefaultStateMachine();
    int GetStateMachineCount();
    bool SetActiveStateMachine(int index);
    bool SetActiveStateMachineByName(const std::string& name);
    int GetActiveStateMachineIndex();
    void PlayStateMachine();
    void PauseStateMachine();
    void ResetStateMachine();
    bool IsStateMachineActive();
    std::vector<StateMachineInputInfo> GetStateMachineInputs();
    bool SetBooleanInput(const std::string& name, bool value);
    bool SetNumberInput(const std::string& name, double value);
    bool FireTrigger(const std::string& name);

    // ViewModel management
    struct ViewModelInfo {
        std::string name;
        int index;
        int id;
        void* nativeViewModel = nullptr;
    };
    
    std::vector<ViewModelInfo> EnumerateViewModels();
    ViewModelInfo GetDefaultViewModel();
    int GetViewModelCount();
    
    // ViewModelInstance management
    void* CreateViewModelInstance(); // Returns rive::ViewModelInstance*
    void* CreateViewModelInstanceById(int viewModelId); // Returns rive::ViewModelInstance*
    void* CreateViewModelInstanceByName(const std::string& viewModelName); // Returns rive::ViewModelInstance*
    bool BindViewModelInstance(void* instance); // Takes rive::ViewModelInstance*
    void* GetBoundViewModelInstance(); // Returns rive::ViewModelInstance*
    
    // Property access on bound instance
    bool SetViewModelStringProperty(const std::string& propertyName, const std::string& value);
    bool SetViewModelNumberProperty(const std::string& propertyName, double value);
    bool SetViewModelBooleanProperty(const std::string& propertyName, bool value);
    bool SetViewModelColorProperty(const std::string& propertyName, uint32_t color);
    bool SetViewModelEnumProperty(const std::string& propertyName, int value);
    bool FireViewModelTrigger(const std::string& triggerName);
    
    // Property enumeration and access
    struct ViewModelPropertyInfo {
        std::string name;
        std::string type; // "String", "Number", "Boolean", "Color", "Enum", "Trigger"
        int index;
    };
    
    std::vector<ViewModelPropertyInfo> GetViewModelProperties(void* instance); // Takes rive::ViewModelInstance*
    void* GetViewModelProperty(void* instance, const std::string& propertyName); // Returns rive::ViewModelInstanceValue*
    void* GetViewModelPropertyAt(void* instance, int index); // Returns rive::ViewModelInstanceValue*

private:
    // Composition setup
    void CreateCompositionSurface();
    
    // DirectX initialization
    void CreateDeviceResources();
    void CreateSwapChain();
    void CreateRenderTarget();
    void RecreateDeviceResources();
    
    // Rive setup
    void CreateRiveContext();
    void CreateRiveContent();
    void ClearScene();
    void MakeScene();
    
    // Rendering
    void RenderLoop();
    void RenderRive();
    
    // Device management
    bool CheckDeviceLost();
    void HandleDeviceLost();
    
    // Resource cleanup
    void CleanupDeviceResources();
    void CleanupRenderingResources();
    
    // Input processing
    void ProcessInputQueue();
    void ForwardPointerEventToStateMachine(float x, float y, bool isDown);
    
    // Coordinate transformation
    bool TransformToArtboardSpace(float& x, float& y);
    void UpdateArtboardAlignment();  // Called when size changes
    
    // State machine initialization
    void EnumerateAndInitializeStateMachines();
};
