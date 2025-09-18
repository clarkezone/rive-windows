using System;
using System.ComponentModel;
using System.Threading.Tasks;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Hosting;
using Windows.UI.Composition;
using Windows.Graphics.Display;
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
            var configurationService = new RiveFileConfigurationService();
            _riveFileService = new RiveFileService(configurationService);
            
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

        /// <summary>
        /// Transforms XAML coordinates to physical pixels for the Rive renderer
        /// </summary>
        /// <param name="xamlPosition">The position from XAML pointer events</param>
        /// <param name="physicalX">Output physical X coordinate</param>
        /// <param name="physicalY">Output physical Y coordinate</param>
        /// <returns>True if transformation was successful</returns>
        private bool TransformToPhysicalCoordinates(Windows.Foundation.Point xamlPosition, out float physicalX, out float physicalY)
        {
            physicalX = 0f;
            physicalY = 0f;

            try
            {
                // Get DPI scale factor
                var displayInfo = DisplayInformation.GetForCurrentView();
                double dpiScale = displayInfo.RawPixelsPerViewPixel;

                // Get detailed layout information for debugging
                double containerWidth = RiveControlContainer.ActualWidth;
                double containerHeight = RiveControlContainer.ActualHeight;
                
                // Get RiveViewerBorder dimensions and offsets
                double borderWidth = RiveViewerBorder.ActualWidth;
                double borderHeight = RiveViewerBorder.ActualHeight;
                var borderThickness = RiveViewerBorder.BorderThickness;
                var borderPadding = RiveViewerBorder.Padding;
                
                // Get RootGrid dimensions  
                double rootWidth = RootGrid.ActualWidth;
                double rootHeight = RootGrid.ActualHeight;
                
                // Log comprehensive layout information
                System.Diagnostics.Debug.WriteLine($"=== LAYOUT DEBUG ===");
                System.Diagnostics.Debug.WriteLine($"RootGrid: {rootWidth:F2} x {rootHeight:F2}");
                System.Diagnostics.Debug.WriteLine($"RiveViewerBorder: {borderWidth:F2} x {borderHeight:F2}");
                System.Diagnostics.Debug.WriteLine($"BorderThickness: L={borderThickness.Left}, T={borderThickness.Top}, R={borderThickness.Right}, B={borderThickness.Bottom}");
                System.Diagnostics.Debug.WriteLine($"BorderPadding: L={borderPadding.Left}, T={borderPadding.Top}, R={borderPadding.Right}, B={borderPadding.Bottom}");
                System.Diagnostics.Debug.WriteLine($"RiveControlContainer: {containerWidth:F2} x {containerHeight:F2}");

                // Calculate total offset from borders and padding
                double leftOffset = borderThickness.Left + borderPadding.Left;
                double topOffset = borderThickness.Top + borderPadding.Top;
                
                // The coordinates should already be relative to RiveControlContainer
                // But let's check if we need to account for any visual positioning
                double adjustedX = xamlPosition.X;
                double adjustedY = xamlPosition.Y;
                
                System.Diagnostics.Debug.WriteLine($"Border/Padding Offsets: Left={leftOffset:F2}, Top={topOffset:F2}");
                
                // Check if RiveControl visual has any transform or offset
                if (_riveControl != null)
                {
                    var visual = _riveControl.GetVisual();
                    if (visual != null)
                    {
                        var offset = visual.Offset;
                        var size = visual.Size;
                        var scale = visual.Scale;
                        System.Diagnostics.Debug.WriteLine($"RiveVisual - Offset=({offset.X:F2}, {offset.Y:F2}), Size=({size.X:F2} x {size.Y:F2}), Scale=({scale.X:F2}, {scale.Y:F2})");
                        
                        // If there's a visual offset, we might need to account for it
                        // Note: The visual offset might be the issue!
                    }
                }

                // Validate input coordinates are within bounds
                if (xamlPosition.X < 0 || xamlPosition.X > containerWidth ||
                    xamlPosition.Y < 0 || xamlPosition.Y > containerHeight)
                {
                    System.Diagnostics.Debug.WriteLine($"Coordinates out of bounds: ({xamlPosition.X:F2}, {xamlPosition.Y:F2}) vs container ({containerWidth:F2}x{containerHeight:F2})");
                    return false;
                }

                // CRITICAL FIX: The coordinates should be in renderer space, NOT physical pixels
                // The Rive renderer expects coordinates matching its internal coordinate system
                // which is the same as the XAML container size, NOT physical pixels
                
                // Use XAML coordinates directly - they match the renderer's coordinate system
                physicalX = (float)adjustedX;
                physicalY = (float)adjustedY;

                // Log final transformation
                System.Diagnostics.Debug.WriteLine($"CORRECTED Transform: XAML({xamlPosition.X:F2}, {xamlPosition.Y:F2}) -> Adjusted({adjustedX:F2}, {adjustedY:F2}) -> RendererSpace({physicalX:F2}, {physicalY:F2}) [DPI={dpiScale:F2}] - Using XAML coordinates directly");
                System.Diagnostics.Debug.WriteLine($"==================");

                return true;
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Error transforming coordinates: {ex.Message}");
                return false;
            }
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
                // Get DPI scale factor for physical dimensions
                var displayInfo = DisplayInformation.GetForCurrentView();
                double dpiScale = 1.0;
                
                // Convert to physical pixels
                int physicalWidth = (int)(e.NewSize.Width * dpiScale);
                int physicalHeight = (int)(e.NewSize.Height * dpiScale);
                
                System.Diagnostics.Debug.WriteLine($"Resizing RiveControl: XAML size ({e.NewSize.Width:F2}x{e.NewSize.Height:F2}) -> Physical size ({physicalWidth}x{physicalHeight}) [DPI={dpiScale:F2}]");
                
                _riveControl.SetSize(physicalWidth, physicalHeight);
            }
        }

        private void RiveControlContainer_PointerMoved(object sender, Windows.UI.Xaml.Input.PointerRoutedEventArgs e)
        {
            System.Diagnostics.Debug.WriteLine("PointerMoved event triggered");
            try
            {
                if (_riveControl != null && IsContentLoaded)
                {
                    // Get position relative to the RiveControlContainer element itself
                    var position = e.GetCurrentPoint(RiveControlContainer);
                    
                    // Also get position relative to the root for comparison
                    var rootPosition = e.GetCurrentPoint(this);
                    
                    System.Diagnostics.Debug.WriteLine($"Raw positions: Container=({position.Position.X:F2}, {position.Position.Y:F2}), Root=({rootPosition.Position.X:F2}, {rootPosition.Position.Y:F2})");
                    
                    if (TransformToPhysicalCoordinates(position.Position, out float physicalX, out float physicalY))
                    {
                        _riveControl.QueuePointerMove(physicalX, physicalY);
                    }
                    else
                    {
                        System.Diagnostics.Debug.WriteLine($"Failed to transform pointer move coordinates");
                    }
                }
                else
                {
                    System.Diagnostics.Debug.WriteLine($"Pointer moved but conditions not met: _riveControl={_riveControl}, IsContentLoaded={IsContentLoaded}");
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Error in pointer moved: {ex.Message}");
            }
        }

        private void RiveControlContainer_PointerPressed(object sender, Windows.UI.Xaml.Input.PointerRoutedEventArgs e)
        {
            System.Diagnostics.Debug.WriteLine("PointerPressed event triggered");
            try
            {
                if (_riveControl != null && IsContentLoaded)
                {
                    var position = e.GetCurrentPoint(RiveControlContainer);
                    
                    if (TransformToPhysicalCoordinates(position.Position, out float physicalX, out float physicalY))
                    {
                        _riveControl.QueuePointerPress(physicalX, physicalY);
                        
                        // Capture pointer for proper tracking
                        RiveControlContainer.CapturePointer(e.Pointer);
                    }
                    else
                    {
                        System.Diagnostics.Debug.WriteLine($"Failed to transform pointer press coordinates");
                    }
                }
                else
                {
                    System.Diagnostics.Debug.WriteLine($"Pointer pressed but conditions not met: _riveControl={_riveControl}, IsContentLoaded={IsContentLoaded}");
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Error in pointer pressed: {ex.Message}");
            }
        }

        private void RiveControlContainer_PointerReleased(object sender, Windows.UI.Xaml.Input.PointerRoutedEventArgs e)
        {
            System.Diagnostics.Debug.WriteLine("PointerReleased event triggered");
            try
            {
                if (_riveControl != null && IsContentLoaded)
                {
                    var position = e.GetCurrentPoint(RiveControlContainer);
                    
                    if (TransformToPhysicalCoordinates(position.Position, out float physicalX, out float physicalY))
                    {
                        _riveControl.QueuePointerRelease(physicalX, physicalY);
                        
                        // Release pointer capture
                        RiveControlContainer.ReleasePointerCapture(e.Pointer);
                    }
                    else
                    {
                        System.Diagnostics.Debug.WriteLine($"Failed to transform pointer release coordinates");
                    }
                }
                else
                {
                    System.Diagnostics.Debug.WriteLine($"Pointer released but conditions not met: _riveControl={_riveControl}, IsContentLoaded={IsContentLoaded}");
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Error in pointer released: {ex.Message}");
            }
        }

        private void RiveControlContainer_PointerCaptureLost(object sender, Windows.UI.Xaml.Input.PointerRoutedEventArgs e)
        {
            try
            {
                if (_riveControl != null && IsContentLoaded)
                {
                    // Send a pointer release at the last known position to clean up state
                    var position = e.GetCurrentPoint(RiveControlContainer);
                    
                    if (TransformToPhysicalCoordinates(position.Position, out float physicalX, out float physicalY))
                    {
                        _riveControl.QueuePointerRelease(physicalX, physicalY);
                    }
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Error in pointer capture lost: {ex.Message}");
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
                
                // Initialize with compositor and DPI-aware size
                double containerWidth = RiveControlContainer.ActualWidth;
                double containerHeight = RiveControlContainer.ActualHeight;
                if (containerWidth > 0 && containerHeight > 0)
                {
                    // Get DPI scale factor for physical dimensions
                    var displayInfo = DisplayInformation.GetForCurrentView();
                    //double dpiScale = displayInfo.RawPixelsPerViewPixel;
                    
                    // Convert to physical pixels
                    int physicalWidth = (int)(containerWidth );
                    int physicalHeight = (int)(containerHeight );
                    ////int physicalWidth = (int)(containerWidth * dpiScale);
                    //int physicalHeight = (int)(containerHeight * dpiScale);
                    
                    //System.Diagnostics.Debug.WriteLine($"Initializing RiveControl: XAML size ({containerWidth:F2}x{containerHeight:F2}) -> Physical size ({physicalWidth}x{physicalHeight}) [DPI={dpiScale:F2}]");
                    
                    bool initResult = _riveControl.Initialize(_compositor, physicalWidth, physicalHeight);
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
