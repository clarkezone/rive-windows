using System;
using System.ComponentModel;
using System.Threading.Tasks;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Hosting;
using Windows.UI.Composition;
using CSXamlHost.Models;
using CSXamlHost.Services;
using WinRive;

namespace CSXamlHost.Controls
{
    /// <summary>
    /// Enhanced RiveControl wrapper with MVVM support, loading states, and error handling
    /// </summary>
    public sealed partial class RiveViewerControl : UserControl, INotifyPropertyChanged
    {
        private readonly RiveFileService _riveFileService;
        private bool _isLoading;
        private bool _hasError;
        private bool _isContentLoaded;
        private string _errorMessage = string.Empty;
        private string _statusText = "Ready";
        private string _fileInfoText = string.Empty;
        private string _dimensionsText = string.Empty;
        private string _fpsText = string.Empty;
        private RiveFileSource? _currentFileSource;
        private RiveControl? _riveControl;
        private Compositor? _compositor;

        public event PropertyChangedEventHandler? PropertyChanged;

        public RiveViewerControl()
        {
            this.InitializeComponent();
            _riveFileService = new RiveFileService();
            
            // Initialize status
            UpdateStatus("Ready to load Rive animation");
            
            // Initialize Rive control when loaded
            this.Loaded += RiveViewerControl_Loaded;
            this.Unloaded += RiveViewerControl_Unloaded;
        }

        #region Dependency Properties

        /// <summary>
        /// The Rive file source to display
        /// </summary>
        public static readonly DependencyProperty FileSourceProperty =
            DependencyProperty.Register(
                nameof(FileSource),
                typeof(RiveFileSource),
                typeof(RiveViewerControl),
                new PropertyMetadata(null, OnFileSourceChanged));

        public RiveFileSource? FileSource
        {
            get => (RiveFileSource?)GetValue(FileSourceProperty);
            set => SetValue(FileSourceProperty, value);
        }

        /// <summary>
        /// Whether to show the status bar
        /// </summary>
        public static readonly DependencyProperty ShowStatusBarProperty =
            DependencyProperty.Register(
                nameof(ShowStatusBar),
                typeof(bool),
                typeof(RiveViewerControl),
                new PropertyMetadata(true));

        public bool ShowStatusBar
        {
            get => (bool)GetValue(ShowStatusBarProperty);
            set => SetValue(ShowStatusBarProperty, value);
        }

        /// <summary>
        /// Whether to show FPS counter
        /// </summary>
        public static readonly DependencyProperty ShowFpsCounterProperty =
            DependencyProperty.Register(
                nameof(ShowFpsCounter),
                typeof(bool),
                typeof(RiveViewerControl),
                new PropertyMetadata(false));

        public bool ShowFpsCounter
        {
            get => (bool)GetValue(ShowFpsCounterProperty);
            set => SetValue(ShowFpsCounterProperty, value);
        }

        /// <summary>
        /// Placeholder text when no content is loaded
        /// </summary>
        public static readonly DependencyProperty PlaceholderTextProperty =
            DependencyProperty.Register(
                nameof(PlaceholderText),
                typeof(string),
                typeof(RiveViewerControl),
                new PropertyMetadata("Select a Rive file to view"));

        public string PlaceholderText
        {
            get => (string)GetValue(PlaceholderTextProperty);
            set => SetValue(PlaceholderTextProperty, value);
        }

        /// <summary>
        /// Auto-play animation when loaded
        /// </summary>
        public static readonly DependencyProperty AutoPlayProperty =
            DependencyProperty.Register(
                nameof(AutoPlay),
                typeof(bool),
                typeof(RiveViewerControl),
                new PropertyMetadata(true));

        public bool AutoPlay
        {
            get => (bool)GetValue(AutoPlayProperty);
            set => SetValue(AutoPlayProperty, value);
        }

        #endregion

        #region Bindable Properties

        /// <summary>
        /// Whether content is currently loading
        /// </summary>
        public bool IsLoading
        {
            get => _isLoading;
            private set
            {
                if (_isLoading != value)
                {
                    _isLoading = value;
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Whether there's an error state
        /// </summary>
        public bool HasError
        {
            get => _hasError;
            private set
            {
                if (_hasError != value)
                {
                    _hasError = value;
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Whether content is successfully loaded
        /// </summary>
        public bool IsContentLoaded
        {
            get => _isContentLoaded;
            private set
            {
                if (_isContentLoaded != value)
                {
                    _isContentLoaded = value;
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Current error message
        /// </summary>
        public string ErrorMessage
        {
            get => _errorMessage;
            private set
            {
                if (_errorMessage != value)
                {
                    _errorMessage = value;
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Current status text
        /// </summary>
        public string StatusText
        {
            get => _statusText;
            private set
            {
                if (_statusText != value)
                {
                    _statusText = value;
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// File information text
        /// </summary>
        public string FileInfoText
        {
            get => _fileInfoText;
            private set
            {
                if (_fileInfoText != value)
                {
                    _fileInfoText = value;
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Dimensions text
        /// </summary>
        public string DimensionsText
        {
            get => _dimensionsText;
            private set
            {
                if (_dimensionsText != value)
                {
                    _dimensionsText = value;
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// FPS text for performance monitoring
        /// </summary>
        public string FpsText
        {
            get => _fpsText;
            private set
            {
                if (_fpsText != value)
                {
                    _fpsText = value;
                    OnPropertyChanged();
                }
            }
        }

        #endregion

        #region Public Methods

        /// <summary>
        /// Loads a Rive file from the specified source
        /// </summary>
        /// <param name="fileSource">The file source to load</param>
        public async Task LoadFileAsync(RiveFileSource fileSource)
        {
            if (fileSource == null)
            {
                ClearContent();
                return;
            }

            try
            {
                SetLoadingState(true);
                ClearError();

                // Validate the file source
                if (!fileSource.IsValid)
                {
                    throw new InvalidOperationException($"Invalid file source: {fileSource.FilePath}");
                }

                // Update current file source
                _currentFileSource = fileSource;

                // Load the file into the WinRive control
                UpdateStatus($"Loading {fileSource.DisplayName}...");
                
                // Use the WinRive control's LoadFile method
                await LoadIntoRiveControlAsync(fileSource);

                // Update UI state
                IsContentLoaded = true;
                UpdateFileInfo();
                UpdateStatus($"Loaded {fileSource.DisplayName}");

                // Auto-play if enabled
                if (AutoPlay)
                {
                    Play();
                }

                // Raise loaded event
                OnFileLoaded(fileSource);
            }
            catch (Exception ex)
            {
                SetErrorState($"Failed to load {fileSource.DisplayName}: {ex.Message}");
            }
            finally
            {
                SetLoadingState(false);
            }
        }

        /// <summary>
        /// Clears the current content
        /// </summary>
        public void ClearContent()
        {
            try
            {
                if (_riveControl != null)
                {
                    _riveControl.StopRenderLoop();
                }
                _currentFileSource = null;
                IsContentLoaded = false;
                ClearError();
                UpdateStatus("Ready to load Rive animation");
                FileInfoText = string.Empty;
                DimensionsText = string.Empty;
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Error clearing content: {ex.Message}");
            }
        }

        /// <summary>
        /// Plays the current animation
        /// </summary>
        public void Play()
        {
            try
            {
                if (IsContentLoaded && _riveControl != null)
                {
                    _riveControl.PlayStateMachine();
                    UpdateStatus("Playing");
                }
            }
            catch (Exception ex)
            {
                SetErrorState($"Failed to play animation: {ex.Message}");
            }
        }

        /// <summary>
        /// Pauses the current animation
        /// </summary>
        public void Pause()
        {
            try
            {
                if (IsContentLoaded && _riveControl != null)
                {
                    _riveControl.PauseStateMachine();
                    UpdateStatus("Paused");
                }
            }
            catch (Exception ex)
            {
                SetErrorState($"Failed to pause animation: {ex.Message}");
            }
        }

        /// <summary>
        /// Stops the current animation
        /// </summary>
        public void Stop()
        {
            try
            {
                if (IsContentLoaded && _riveControl != null)
                {
                    _riveControl.ResetStateMachine();
                    UpdateStatus("Stopped");
                }
            }
            catch (Exception ex)
            {
                SetErrorState($"Failed to stop animation: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets access to the underlying WinRive control
        /// </summary>
        public WinRive.RiveControl? GetRiveControl()
        {
            return _riveControl;
        }

        #endregion

        #region Events

        /// <summary>
        /// Raised when a file is successfully loaded
        /// </summary>
        public event EventHandler<RiveFileLoadedEventArgs>? FileLoaded;

        /// <summary>
        /// Raised when an error occurs
        /// </summary>
        public event EventHandler<RiveErrorEventArgs>? ErrorOccurred;

        #endregion

        #region Private Methods

        private static void OnFileSourceChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is RiveViewerControl control && e.NewValue is RiveFileSource newSource)
            {
                _ = control.LoadFileAsync(newSource);
            }
        }

        private async Task LoadIntoRiveControlAsync(RiveFileSource fileSource)
        {
            // This method interfaces with the actual WinRive control
            // The exact implementation depends on the WinRive control's API
            try
            {
                if (_riveControl == null)
                    throw new InvalidOperationException("RiveControl not initialized");

                _riveControl.StopRenderLoop();
                bool loadResult = false;
                switch (fileSource.SourceType)
                {
                    case RiveFileSourceType.Package:
                        // Load from app package
                        loadResult = _riveControl.LoadRiveFileFromPackage(fileSource.FilePath);
                        break;

                    case RiveFileSourceType.External:
                        // Load from external file
                        loadResult = _riveControl.LoadRiveFile(fileSource.FilePath);
                        break;

                    default:
                        throw new NotSupportedException($"File source type {fileSource.SourceType} is not supported");
                }

                if (!loadResult)
                {
                    throw new InvalidOperationException("Failed to load Rive file");
                }

                // Start the render loop
                _riveControl.StartRenderLoop();
                
                await Task.CompletedTask; // Make method async-compatible
            }
            catch (Exception ex)
            {
                throw new InvalidOperationException($"Failed to load file into RiveControl: {ex.Message}", ex);
            }
        }

        private void SetLoadingState(bool isLoading)
        {
            IsLoading = isLoading;
            if (isLoading)
            {
                HasError = false;
                ErrorMessage = string.Empty;
            }
        }

        private void SetErrorState(string errorMessage)
        {
            HasError = true;
            ErrorMessage = errorMessage;
            IsContentLoaded = false;
            IsLoading = false;
            UpdateStatus($"Error: {errorMessage}");
            OnErrorOccurred(errorMessage);
        }

        private void ClearError()
        {
            HasError = false;
            ErrorMessage = string.Empty;
        }

        private void UpdateStatus(string status)
        {
            StatusText = status;
        }

        private void UpdateFileInfo()
        {
            if (_currentFileSource != null)
            {
                FileInfoText = _currentFileSource.DisplayName;
            }
            else
            {
                FileInfoText = string.Empty;
            }
        }

        private void UpdateDimensions(double width, double height)
        {
            if (width > 0 && height > 0)
            {
                DimensionsText = $"{width:F0} × {height:F0}";
            }
            else
            {
                DimensionsText = string.Empty;
            }
        }

        private void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string? propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        private void OnFileLoaded(RiveFileSource fileSource)
        {
            FileLoaded?.Invoke(this, new RiveFileLoadedEventArgs(fileSource));
        }

        private void OnErrorOccurred(string errorMessage)
        {
            ErrorOccurred?.Invoke(this, new RiveErrorEventArgs(errorMessage));
        }

        #endregion

        #region Event Handlers

        private void RiveViewerControl_Loaded(object sender, RoutedEventArgs e)
        {
            // Initialize the RiveControl when this control is loaded
            InitializeRiveControl();
        }

        private void RiveViewerControl_Unloaded(object sender, RoutedEventArgs e)
        {
            // Cleanup when unloaded
            CleanupRiveControl();
        }

        private void RiveControlContainer_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            // Update dimensions when size changes
            UpdateDimensions(e.NewSize.Width, e.NewSize.Height);
            
            // Update RiveControl size if initialized
            if (_riveControl != null && e.NewSize.Width > 0 && e.NewSize.Height > 0)
            {
                _riveControl.SetSize((int)e.NewSize.Width, (int)e.NewSize.Height);
            }
        }

        private void InitializeRiveControl()
        {
            try
            {
                // Get the compositor from the RiveControlContainer
                var containerElement = ElementCompositionPreview.GetElementVisual(RiveControlContainer);
                _compositor = containerElement.Compositor;

                // Create and initialize the RiveControl
                _riveControl = new RiveControl();
                
                // Initialize with compositor and initial size
                double containerWidth = RiveControlContainer.ActualWidth;
                double containerHeight = RiveControlContainer.ActualHeight;
                if (containerWidth > 0 && containerHeight > 0)
                {
                    bool initResult = _riveControl.Initialize(_compositor, (int)containerWidth, (int)containerHeight);
                    if (!initResult)
                    {
                        throw new InvalidOperationException("Failed to initialize RiveControl");
                    }

                    // Get the visual and add it to the container
                    var riveVisual = _riveControl.GetVisual();
                    if (riveVisual != null)
                    {
                        ElementCompositionPreview.SetElementChildVisual(RiveControlContainer, riveVisual);
                    }
                }
                
                UpdateStatus("RiveControl initialized");
            }
            catch (Exception ex)
            {
                SetErrorState($"Failed to initialize RiveControl: {ex.Message}");
            }
        }

        private void CleanupRiveControl()
        {
            try
            {
                if (_riveControl != null)
                {
                    _riveControl.StopRenderLoop();
                    _riveControl.Shutdown();
                    _riveControl = null;
                }

                // Clear the visual from the container
                ElementCompositionPreview.SetElementChildVisual(RiveControlContainer, null);
                
                _compositor = null;
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Error during cleanup: {ex.Message}");
            }
        }

        #endregion
    }

    #region Event Args Classes

    /// <summary>
    /// Event arguments for file loaded events
    /// </summary>
    public class RiveFileLoadedEventArgs : EventArgs
    {
        public RiveFileSource FileSource { get; }

        public RiveFileLoadedEventArgs(RiveFileSource fileSource)
        {
            FileSource = fileSource ?? throw new ArgumentNullException(nameof(fileSource));
        }
    }

    /// <summary>
    /// Event arguments for error events
    /// </summary>
    public class RiveErrorEventArgs : EventArgs
    {
        public string ErrorMessage { get; }

        public RiveErrorEventArgs(string errorMessage)
        {
            ErrorMessage = errorMessage ?? throw new ArgumentNullException(nameof(errorMessage));
        }
    }

    #endregion
}
