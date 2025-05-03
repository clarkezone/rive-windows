using System.Diagnostics.CodeAnalysis;
using Windows.ApplicationModel.Core;
using Windows.UI.Core;
using Windows.UI.Composition;
using Windows.UI;
using System.Numerics;

namespace Self_contained_Rive
{
    /// <summary>
    /// Represent a non-XAML UWP app, i.e. an app without XAML content that is hosted in a <see cref="CoreWindow"/> instance.
    /// </summary>
    public sealed partial class App : IFrameworkViewSource, IFrameworkView
    {
        /// <summary>
        /// The <see cref="CoreApplicationView"/> for the current app instance.
        /// </summary>
        private CoreApplicationView? _applicationView;

        /// <summary>
        /// The <see cref="CoreWindow"/> used to display the app content.
        /// </summary>
        private CoreWindow? _window;

        private Compositor? _compositor;
        private ContainerVisual? _root;
        private CompositionTarget? _compositionTarget;

        /// <summary>
        /// The entry point for the application.
        /// </summary>
        public static void Main()
        {
            CoreApplication.Run(new App());
        }

        /// <inheritdoc/>
        public IFrameworkView CreateView()
        {
            return this;
        }

        /// <inheritdoc/>
        [MemberNotNull(nameof(_applicationView))]
        public void Initialize(CoreApplicationView applicationView)
        {
            this._applicationView = applicationView;
        }

        /// <inheritdoc/>
        [MemberNotNull(nameof(_window))]
        public void SetWindow(CoreWindow window)
        {
            this._window = window;
            SetupComposition();
        }

        /// <inheritdoc/>
        public void Load(string entryPoint)
        {
        }

        /// <inheritdoc/>
        public void Run()
        {
            // Activate the current window
            _window!.Activate();

            // Process any messages in the queue
            _window.Dispatcher.ProcessEvents(CoreProcessEventsOption.ProcessUntilQuit);
        }

        /// <inheritdoc/>
        public void Uninitialize()
        {
        }

         private void SetupComposition()
        {
            // Create the compositor
            _compositor = new Compositor();

            // Create the root visual
            _root = _compositor.CreateContainerVisual();
            _root.RelativeSizeAdjustment = Vector2.One; // Fill the window
            _root.Size = new Vector2((float)_window.Bounds.Width, (float)_window.Bounds.Height);

            // Create a red square visual
            SpriteVisual redSquare = _compositor.CreateSpriteVisual();
            
            // Set the size of the square (200x200 pixels)
            redSquare.Size = new Vector2(200, 200);
            
            // Center the square in the window
            redSquare.Offset = new Vector3(
                (float)((_window.Bounds.Width - 200) / 2),
                (float)((_window.Bounds.Height - 200) / 2),
                0);
            
            // Set the color to red
            Color redColor = Color.FromArgb(255, 255, 0, 0);
            redSquare.Brush = _compositor.CreateColorBrush(redColor);

            // Add the red square to the root visual
            _root.Children.InsertAtTop(redSquare);

            _compositionTarget = _compositor.CreateTargetForCurrentView();
            _compositionTarget.Root = _root;

            // In a frameworkless UWP app, we need to use the CoreWindow directly
            // This is a simplified approach for demonstration purposes
            // The actual implementation would require more setup in a real UWP app

            // Note: In a real UWP app, you would typically use:
            // Windows.UI.Composition.CompositionTarget.FromVisual(_window).Root = _root;
            // But this requires proper initialization of the CompositionTarget

            // For this example, we'll leave this as a comment since we can't
            // fully implement it without the proper UWP environment
        }
    }
}
