#pragma once

#include <functional>
#include <chrono>
#include <memory>

namespace winrt::WinRive::implementation
{
    // Input event structure for unified input handling
    struct InputEvent {
        enum Type { Move, Press, Release };
        Type type;
        float x, y;  // Relative to control bounds (0,0 to width,height)
        std::chrono::steady_clock::time_point timestamp;
    };

    // Abstract base class for input providers
    class IInputProvider
    {
    public:
        virtual ~IInputProvider() = default;
        
        // Initialize the input provider
        virtual bool Initialize() = 0;
        
        // Cleanup resources
        virtual void Shutdown() = 0;
        
        // Set the bounds for coordinate transformation
        virtual void SetBounds(int width, int height) = 0;
        
        // Event callback type
        using InputEventCallback = std::function<void(const InputEvent&)>;
        
        // Set the callback for input events
        virtual void SetInputEventCallback(InputEventCallback callback) = 0;
    };

    // Null input provider for scenarios without input handling
    class NullInputProvider : public IInputProvider
    {
    public:
        bool Initialize() override { return true; }
        void Shutdown() override {}
        void SetBounds(int width, int height) override {}
        void SetInputEventCallback(InputEventCallback callback) override {}
    };
}
