using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Threading.Tasks;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Data;
using CSXamlHost.Models;
using CSXamlHost.Services;
using WinRive;

namespace CSXamlHost.Controls
{
    /// <summary>
    /// UserControl for managing Rive state machines and their inputs
    /// Provides UI for file selection, state machine control, and dynamic input generation
    /// </summary>
    public sealed partial class RiveStateMachinePanel : UserControl, INotifyPropertyChanged
    {
        private readonly RiveFileService _riveFileService;
        private readonly FilePickerService _filePickerService;
        private readonly RiveFileConfigurationService _configurationService;
        private RiveViewerControl? _riveViewer;
        private RiveFileSource? _selectedFile;
        private StateMachineModel? _selectedStateMachine;
        private string _statusText = "Ready";
        private string _errorMessage = string.Empty;
        private string _fileInfoText = string.Empty;
        private string _stateMachineInfoText = string.Empty;
        private bool _hasError;
        private readonly Dictionary<string, FrameworkElement> _inputControls = new();
        private bool _configurationInitialized = false;

        public event PropertyChangedEventHandler? PropertyChanged;

        public RiveStateMachinePanel()
        {
            this.InitializeComponent();
            
            // Initialize configuration service
            _configurationService = new RiveFileConfigurationService();
            _riveFileService = new RiveFileService(_configurationService);
            _filePickerService = new FilePickerService(_riveFileService);
            
            // Initialize collections
            AvailableFiles = new ObservableCollection<RiveFileSource>();
            StateMachines = new ObservableCollection<StateMachineModel>();
            
            // Set initial status
            UpdateStatus("Ready");
            
            this.Loaded += RiveStateMachinePanel_Loaded;
            this.Unloaded += RiveStateMachinePanel_Unloaded;
        }

        private async Task InitializeConfigurationAsync()
        {
            try
            {
                UpdateStatus("Initializing configuration...");
                
                // Try to load user configuration first, fallback to default
                var userConfig = await _configurationService.LoadUserConfigurationAsync();
                if (userConfig == null)
                {
                    await _configurationService.LoadDefaultConfigurationAsync();
                }
                
                _configurationInitialized = true;
                UpdateStatus("Configuration loaded successfully");
            }
            catch (Exception ex)
            {
                SetError($"Failed to initialize configuration: {ex.Message}");
                _configurationInitialized = true; // Set as initialized even with error for fallback
                UpdateStatus("Using fallback configuration");
            }
        }

        #region Dependency Properties

        /// <summary>
        /// The RiveViewerControl to manage
        /// </summary>
        public static readonly DependencyProperty RiveViewerProperty =
            DependencyProperty.Register(
                nameof(RiveViewer),
                typeof(RiveViewerControl),
                typeof(RiveStateMachinePanel),
                new PropertyMetadata(null, OnRiveViewerChanged));

        public RiveViewerControl? RiveViewer
        {
            get => (RiveViewerControl?)GetValue(RiveViewerProperty);
            set => SetValue(RiveViewerProperty, value);
        }

        /// <summary>
        /// Collection of available Rive files
        /// </summary>
        public static readonly DependencyProperty AvailableFilesProperty =
            DependencyProperty.Register(
                nameof(AvailableFiles),
                typeof(ObservableCollection<RiveFileSource>),
                typeof(RiveStateMachinePanel),
                new PropertyMetadata(null));

        public ObservableCollection<RiveFileSource> AvailableFiles
        {
            get => (ObservableCollection<RiveFileSource>)GetValue(AvailableFilesProperty);
            set => SetValue(AvailableFilesProperty, value);
        }

        /// <summary>
        /// Whether to allow loading external files via file picker
        /// </summary>
        public static readonly DependencyProperty AllowExternalFilesProperty =
            DependencyProperty.Register(
                nameof(AllowExternalFiles),
                typeof(bool),
                typeof(RiveStateMachinePanel),
                new PropertyMetadata(true));

        public bool AllowExternalFiles
        {
            get => (bool)GetValue(AllowExternalFilesProperty);
            set => SetValue(AllowExternalFilesProperty, value);
        }

        /// <summary>
        /// Whether to show the status section
        /// </summary>
        public static readonly DependencyProperty ShowStatusProperty =
            DependencyProperty.Register(
                nameof(ShowStatus),
                typeof(bool),
                typeof(RiveStateMachinePanel),
                new PropertyMetadata(true));

        public bool ShowStatus
        {
            get => (bool)GetValue(ShowStatusProperty);
            set => SetValue(ShowStatusProperty, value);
        }

        #endregion

        #region Bindable Properties

        /// <summary>
        /// Currently selected file
        /// </summary>
        public RiveFileSource? SelectedFile
        {
            get => _selectedFile;
            set
            {
                if (_selectedFile != value)
                {
                    _selectedFile = value;
                    OnPropertyChanged();
                    UpdateFileInfo();
                }
            }
        }

        /// <summary>
        /// Available state machines for the selected file
        /// </summary>
        public ObservableCollection<StateMachineModel> StateMachines { get; } = new();

        /// <summary>
        /// Currently selected state machine
        /// </summary>
        public StateMachineModel? SelectedStateMachine
        {
            get => _selectedStateMachine;
            set
            {
                if (_selectedStateMachine != value)
                {
                    _selectedStateMachine = value;
                    OnPropertyChanged();
                    UpdateStateMachineInfo();
                    GenerateInputControls();
                }
            }
        }

        /// <summary>
        /// Current status message
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
        /// State machine information text
        /// </summary>
        public string StateMachineInfoText
        {
            get => _stateMachineInfoText;
            private set
            {
                if (_stateMachineInfoText != value)
                {
                    _stateMachineInfoText = value;
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Whether there's currently an error
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
        /// Whether there are available state machines
        /// </summary>
        public bool HasStateMachines => StateMachines.Count > 0;

        /// <summary>
        /// Whether there are available inputs for the selected state machine
        /// </summary>
        public bool HasInputs => SelectedStateMachine?.Inputs.Count > 0;

        /// <summary>
        /// Whether there's a connected RiveViewer control
        /// </summary>
        public bool HasRiveViewer => RiveViewer != null;

        /// <summary>
        /// Whether playback can be controlled
        /// </summary>
        public bool CanControlPlayback => HasRiveViewer && SelectedFile != null;

        #endregion

        #region Events

        /// <summary>
        /// Raised when a file is selected
        /// </summary>
        public event EventHandler<FileSelectedEventArgs>? FileSelected;

        /// <summary>
        /// Raised when a state machine is selected
        /// </summary>
        public event EventHandler<StateMachineSelectedEventArgs>? StateMachineSelected;

        /// <summary>
        /// Raised when the status changes
        /// </summary>
        public event EventHandler<StatusChangedEventArgs>? StatusChanged;

        #endregion

        #region Public Methods

        /// <summary>
        /// Loads the available files using the configuration system
        /// </summary>
        public async Task LoadDefaultFilesAsync()
        {
            try
            {
                UpdateStatus("Loading available files...");
                var availableFiles = await _riveFileService.GetAvailableRiveFilesAsync();
                
                AvailableFiles.Clear();
                foreach (var file in availableFiles)
                {
                    AvailableFiles.Add(file);
                }

                // Try to select the default file if available
                var defaultFile = _riveFileService.GetDefaultFile();
                if (defaultFile != null && AvailableFiles.Contains(defaultFile))
                {
                    SelectedFile = defaultFile;
                }

                UpdateStatus($"Loaded {availableFiles.Count} available files from configuration");
                ClearError();
            }
            catch (Exception ex)
            {
                SetError($"Failed to load files: {ex.Message}");
            }
        }

        /// <summary>
        /// Loads an external file via file picker
        /// </summary>
        public async Task LoadExternalFileAsync()
        {
            try
            {
                UpdateStatus("Opening file picker...");
                var file = await _filePickerService.PickSingleRiveFileAsync();
                
                if (file != null)
                {
                    // Add to available files if not already present
                    if (!AvailableFiles.Any(f => f.FilePath.Equals(file.FilePath, StringComparison.OrdinalIgnoreCase)))
                    {
                        AvailableFiles.Add(file);
                    }

                    // Select the loaded file
                    SelectedFile = file;
                    UpdateStatus($"Loaded external file: {file.DisplayName}");
                    ClearError();

                    // Notify that a file was selected
                    OnFileSelected(file);
                }
                else
                {
                    UpdateStatus("File selection cancelled");
                }
            }
            catch (Exception ex)
            {
                SetError($"Failed to load external file: {ex.Message}");
            }
        }

        #endregion

        #region Private Methods

        private static void OnRiveViewerChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is RiveStateMachinePanel panel)
            {
                panel.OnRiveViewerChanged(e.OldValue as RiveViewerControl, e.NewValue as RiveViewerControl);
            }
        }

        private void OnRiveViewerChanged(RiveViewerControl? oldViewer, RiveViewerControl? newViewer)
        {
            // Unsubscribe from old viewer
            if (oldViewer != null)
            {
                oldViewer.FileLoaded -= RiveViewer_FileLoaded;
                oldViewer.ErrorOccurred -= RiveViewer_ErrorOccurred;
            }

            // Subscribe to new viewer
            if (newViewer != null)
            {
                newViewer.FileLoaded += RiveViewer_FileLoaded;
                newViewer.ErrorOccurred += RiveViewer_ErrorOccurred;
            }

            _riveViewer = newViewer;
            OnPropertyChanged(nameof(HasRiveViewer));
            OnPropertyChanged(nameof(CanControlPlayback));
        }

        private void UpdateStatus(string status)
        {
            StatusText = status;
            OnStatusChanged(status);
        }

        private void SetError(string errorMessage)
        {
            HasError = true;
            ErrorMessage = errorMessage;
            StatusText = "Error occurred";
            OnStatusChanged($"Error: {errorMessage}");
        }

        private void ClearError()
        {
            HasError = false;
            ErrorMessage = string.Empty;
        }

        private void UpdateFileInfo()
        {
            if (SelectedFile != null)
            {
                FileInfoText = $"File: {SelectedFile.DisplayName} ({SelectedFile.SourceType})";
            }
            else
            {
                FileInfoText = string.Empty;
            }
        }

        private void UpdateStateMachineInfo()
        {
            if (SelectedStateMachine != null)
            {
                var inputCount = SelectedStateMachine.Inputs.Count;
                StateMachineInfoText = $"State Machine: {SelectedStateMachine.Name} ({inputCount} inputs)";
            }
            else
            {
                StateMachineInfoText = string.Empty;
            }

            OnPropertyChanged(nameof(HasInputs));
        }

        private void GenerateInputControls()
        {
            // Clear existing controls
            DynamicInputsPanel.Children.Clear();
            _inputControls.Clear();

            if (SelectedStateMachine?.Inputs == null)
            {
                return;
            }

            // Generate controls for each input
            foreach (var input in SelectedStateMachine.Inputs)
            {
                var controlContainer = CreateInputControl(input);
                if (controlContainer != null)
                {
                    DynamicInputsPanel.Children.Add(controlContainer);
                }
            }
        }

        private FrameworkElement? CreateInputControl(StateMachineInputModel input)
        {
            StackPanel container = new StackPanel
            {
                Orientation = Orientation.Vertical,
                Spacing = 4,
                Margin = new Thickness(0, 4, 0, 4)
            };

            // Add label
            TextBlock label = new TextBlock
            {
                Text = input.Name,
                Style = Application.Current.Resources["BodyTextBlockStyle"] as Style,
                Margin = new Thickness(0, 0, 0, 4)
            };
            container.Children.Add(label);

            FrameworkElement? inputControl = null;

            switch (input.Type)
            {
                case StateMachineInputType.Boolean:
                    inputControl = CreateBooleanInputControl(input);
                    break;

                case StateMachineInputType.Number:
                    inputControl = CreateNumberInputControl(input);
                    break;

                case StateMachineInputType.Trigger:
                    inputControl = CreateTriggerInputControl(input);
                    break;
            }

            if (inputControl != null)
            {
                container.Children.Add(inputControl);
                _inputControls[input.Name] = inputControl;
                return container;
            }

            return null;
        }

        private ToggleSwitch CreateBooleanInputControl(StateMachineInputModel input)
        {
            var toggle = new ToggleSwitch
            {
                IsOn = input.BooleanValue,
                HorizontalAlignment = HorizontalAlignment.Left
            };

            toggle.Toggled += (s, e) =>
            {
                input.BooleanValue = toggle.IsOn;
                ApplyInputToRiveControl(input);
            };

            return toggle;
        }

        private Slider CreateNumberInputControl(StateMachineInputModel input)
        {
            var slider = new Slider
            {
                Minimum = 0,
                Maximum = 100,
                Value = input.NumberValue,
                HorizontalAlignment = HorizontalAlignment.Stretch
            };

            slider.ValueChanged += (s, e) =>
            {
                input.NumberValue = slider.Value;
                ApplyInputToRiveControl(input);
            };

            return slider;
        }

        private Button CreateTriggerInputControl(StateMachineInputModel input)
        {
            var button = new Button
            {
                Content = $"Fire {input.Name}",
                HorizontalAlignment = HorizontalAlignment.Left
            };

            button.Click += (s, e) =>
            {
                ApplyInputToRiveControl(input);
            };

            return button;
        }

        private void ApplyInputToRiveControl(StateMachineInputModel input)
        {
            try
            {
                var riveControl = RiveViewer?.GetRiveControl();
                if (riveControl == null) return;

                switch (input.Type)
                {
                    case StateMachineInputType.Boolean:
                        riveControl.SetBooleanInput(input.Name, input.BooleanValue);
                        break;

                    case StateMachineInputType.Number:
                        riveControl.SetNumberInput(input.Name, (float)input.NumberValue);
                        break;

                    case StateMachineInputType.Trigger:
                        riveControl.FireTrigger(input.Name);
                        break;
                }
            }
            catch (Exception ex)
            {
                SetError($"Failed to apply input '{input.Name}': {ex.Message}");
            }
        }

        private async void LoadStateMachinesFromFile(RiveFileSource fileSource)
        {
            try
            {
                StateMachines.Clear();
                
                // For now, create a demo state machine since the RiveControl API
                // for querying state machines is not yet available
                // TODO: Integrate with actual RiveControl state machine discovery when API is available
                CreateDemoStateMachine();
                
                // Select the first available state machine
                if (StateMachines.Count > 0)
                {
                    SelectedStateMachine = StateMachines.FirstOrDefault(sm => sm.IsDefault) ?? StateMachines[0];
                }

                OnPropertyChanged(nameof(HasStateMachines));
                UpdateStatus($"Loaded {StateMachines.Count} state machine{(StateMachines.Count != 1 ? "s" : "")} (demo for UI testing)");
            }
            catch (Exception ex)
            {
                SetError($"Failed to load state machines: {ex.Message}");
            }
        }

        private void CreateDemoStateMachine()
        {
            var demoStateMachine = new StateMachineModel("Demo State Machine", 0, true);

            // Add demo inputs for testing the UI
            demoStateMachine.Inputs.Add(new StateMachineInputModel(
                "isPlaying",
                StateMachineInputType.Boolean,
                0,
                false
            ));

            demoStateMachine.Inputs.Add(new StateMachineInputModel(
                "speed",
                StateMachineInputType.Number,
                1,
                1.0
            ));

            demoStateMachine.Inputs.Add(new StateMachineInputModel(
                "reset",
                StateMachineInputType.Trigger,
                2
            ));

            StateMachines.Add(demoStateMachine);
        }

        private void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string? propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        private void OnFileSelected(RiveFileSource fileSource)
        {
            FileSelected?.Invoke(this, new FileSelectedEventArgs(fileSource));
        }

        private void OnStateMachineSelected(StateMachineModel stateMachine)
        {
            StateMachineSelected?.Invoke(this, new StateMachineSelectedEventArgs(stateMachine));
        }

        private void OnStatusChanged(string status)
        {
            StatusChanged?.Invoke(this, new StatusChangedEventArgs(status));
        }

        #endregion

        #region Event Handlers

        private async void RiveStateMachinePanel_Loaded(object sender, RoutedEventArgs e)
        {
            // Initialize configuration first, then load files
            if (!_configurationInitialized)
            {
                await InitializeConfigurationAsync();
            }
            
            // Load default files when control loads
            await LoadDefaultFilesAsync();
        }

        private void RiveStateMachinePanel_Unloaded(object sender, RoutedEventArgs e)
        {
            // Cleanup when unloaded
            if (_riveViewer != null)
            {
                _riveViewer.FileLoaded -= RiveViewer_FileLoaded;
                _riveViewer.ErrorOccurred -= RiveViewer_ErrorOccurred;
            }
        }

        private async void FileSelectionComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.FirstOrDefault() is RiveFileSource selectedFile && RiveViewer != null)
            {
                try
                {
                    UpdateStatus($"Loading file: {selectedFile.DisplayName}...");
                    await RiveViewer.LoadFileAsync(selectedFile);
                    OnFileSelected(selectedFile);
                }
                catch (Exception ex)
                {
                    SetError($"Failed to load file: {ex.Message}");
                }
            }
        }

        private async void LoadExternalFileButton_Click(object sender, RoutedEventArgs e)
        {
            await LoadExternalFileAsync();
        }

        private void StateMachineComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.FirstOrDefault() is StateMachineModel selectedStateMachine)
            {
                OnStateMachineSelected(selectedStateMachine);
            }
        }

        private void PlayButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                RiveViewer?.Play();
                UpdateStatus("Playback started");
            }
            catch (Exception ex)
            {
                SetError($"Failed to start playback: {ex.Message}");
            }
        }

        private void PauseButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                RiveViewer?.Pause();
                UpdateStatus("Playback paused");
            }
            catch (Exception ex)
            {
                SetError($"Failed to pause playback: {ex.Message}");
            }
        }

        private void StopButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                RiveViewer?.Stop();
                UpdateStatus("Playback stopped");
            }
            catch (Exception ex)
            {
                SetError($"Failed to stop playback: {ex.Message}");
            }
        }

        private void RiveViewer_FileLoaded(object sender, RiveFileLoadedEventArgs e)
        {
            LoadStateMachinesFromFile(e.FileSource);
            UpdateStatus($"File loaded successfully: {e.FileSource.DisplayName}");
        }

        private void RiveViewer_ErrorOccurred(object sender, RiveErrorEventArgs e)
        {
            SetError($"RiveViewer error: {e.ErrorMessage}");
        }

        #endregion
    }

    #region Event Args Classes

    /// <summary>
    /// Event arguments for file selection events
    /// </summary>
    public class FileSelectedEventArgs : EventArgs
    {
        public RiveFileSource FileSource { get; }

        public FileSelectedEventArgs(RiveFileSource fileSource)
        {
            FileSource = fileSource ?? throw new ArgumentNullException(nameof(fileSource));
        }
    }

    /// <summary>
    /// Event arguments for state machine selection events
    /// </summary>
    public class StateMachineSelectedEventArgs : EventArgs
    {
        public StateMachineModel StateMachine { get; }

        public StateMachineSelectedEventArgs(StateMachineModel stateMachine)
        {
            StateMachine = stateMachine ?? throw new ArgumentNullException(nameof(stateMachine));
        }
    }

    /// <summary>
    /// Event arguments for status change events
    /// </summary>
    public class StatusChangedEventArgs : EventArgs
    {
        public string Status { get; }

        public StatusChangedEventArgs(string status)
        {
            Status = status ?? throw new ArgumentNullException(nameof(status));
        }
    }

    #endregion
}
