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
    /// UserControl for managing Rive view models and their instances/properties
    /// Provides UI for file selection, view model control, and dynamic property generation
    /// </summary>
    public sealed partial class RiveViewModelPanel : UserControl, INotifyPropertyChanged
    {
        private readonly RiveFileService _riveFileService;
        private readonly FilePickerService _filePickerService;
        private readonly RiveFileConfigurationService _configurationService;
        private RiveViewerControl? _riveViewer;
        private RiveFileSource? _selectedFile;
        private ViewModel? _selectedViewModel;
        private ViewModelInstance? _currentViewModelInstance;
        private string _statusText = "Ready";
        private string _errorMessage = string.Empty;
        private string _fileInfoText = string.Empty;
        private string _viewModelInfoText = string.Empty;
        private string _instanceStatusText = string.Empty;
        private bool _hasError;
        private bool _isInstanceBound;
        private readonly Dictionary<string, FrameworkElement> _propertyControls = new();

        public event PropertyChangedEventHandler? PropertyChanged;

        public RiveViewModelPanel()
        {
            this.InitializeComponent();
            
            // Initialize configuration service
            _configurationService = new RiveFileConfigurationService();
            _riveFileService = new RiveFileService(_configurationService);
            _filePickerService = new FilePickerService(_riveFileService);
            
            // Initialize collections
            AvailableFiles = new ObservableCollection<RiveFileSource>();
            ViewModels = new ObservableCollection<ViewModel>();
            
            // Set initial status
            UpdateStatus("Ready");
            
            this.Loaded += RiveViewModelPanel_Loaded;
            this.Unloaded += RiveViewModelPanel_Unloaded;
        }

        #region Dependency Properties

        /// <summary>
        /// The RiveViewerControl to manage
        /// </summary>
        public static readonly DependencyProperty RiveViewerProperty =
            DependencyProperty.Register(
                nameof(RiveViewer),
                typeof(RiveViewerControl),
                typeof(RiveViewModelPanel),
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
                typeof(RiveViewModelPanel),
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
                typeof(RiveViewModelPanel),
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
                typeof(RiveViewModelPanel),
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
        /// Available view models for the selected file
        /// </summary>
        public ObservableCollection<ViewModel> ViewModels { get; } = new();

        /// <summary>
        /// Currently selected view model
        /// </summary>
        public ViewModel? SelectedViewModel
        {
            get => _selectedViewModel;
            set
            {
                if (_selectedViewModel != value)
                {
                    _selectedViewModel = value;
                    OnPropertyChanged();
                    OnPropertyChanged(nameof(HasSelectedViewModel));
                    OnPropertyChanged(nameof(CanCreateInstance));
                    UpdateViewModelInfo();
                    ClearCurrentInstance();
                }
            }
        }

        /// <summary>
        /// Current view model instance
        /// </summary>
        public ViewModelInstance? CurrentViewModelInstance
        {
            get => _currentViewModelInstance;
            private set
            {
                if (_currentViewModelInstance != value)
                {
                    _currentViewModelInstance = value;
                    OnPropertyChanged();
                    OnPropertyChanged(nameof(HasInstance));
                    OnPropertyChanged(nameof(CanBindInstance));
                    OnPropertyChanged(nameof(CanCreateInstance));
                    UpdateInstanceStatus();
                    GeneratePropertyControls();
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
        /// View model information text
        /// </summary>
        public string ViewModelInfoText
        {
            get => _viewModelInfoText;
            private set
            {
                if (_viewModelInfoText != value)
                {
                    _viewModelInfoText = value;
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Instance status text
        /// </summary>
        public string InstanceStatusText
        {
            get => _instanceStatusText;
            private set
            {
                if (_instanceStatusText != value)
                {
                    _instanceStatusText = value;
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
        /// Whether there are available view models
        /// </summary>
        public bool HasViewModels => ViewModels.Count > 0;

        /// <summary>
        /// Whether there's a selected view model
        /// </summary>
        public bool HasSelectedViewModel => SelectedViewModel != null;

        /// <summary>
        /// Whether there's a current view model instance
        /// </summary>
        public bool HasInstance => CurrentViewModelInstance != null;

        /// <summary>
        /// Whether there are available properties for the bound instance
        /// </summary>
        public bool HasProperties => _isInstanceBound && CurrentViewModelInstance != null;

        /// <summary>
        /// Whether an instance can be created
        /// </summary>
        public bool CanCreateInstance => SelectedViewModel != null && CurrentViewModelInstance == null;

        /// <summary>
        /// Whether an instance can be bound
        /// </summary>
        public bool CanBindInstance => CurrentViewModelInstance != null && !_isInstanceBound;

        /// <summary>
        /// Whether there's a connected RiveViewer control
        /// </summary>
        public bool HasRiveViewer => RiveViewer != null;

        #endregion

        #region Events

        /// <summary>
        /// Raised when a file is selected
        /// </summary>
        public event EventHandler<FileSelectedEventArgs>? FileSelected;

        /// <summary>
        /// Raised when a view model is selected
        /// </summary>
        public event EventHandler<ViewModelSelectedEventArgs>? ViewModelSelected;

        /// <summary>
        /// Raised when a view model instance is bound
        /// </summary>
        public event EventHandler<ViewModelInstanceBoundEventArgs>? ViewModelInstanceBound;

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
            if (d is RiveViewModelPanel panel)
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

        private void UpdateViewModelInfo()
        {
            if (SelectedViewModel != null)
            {
                var propertyCount = SelectedViewModel.GetPropertyCount();
                ViewModelInfoText = $"View Model: {SelectedViewModel.Name} ({propertyCount} properties)";
            }
            else
            {
                ViewModelInfoText = string.Empty;
            }
        }

        private void UpdateInstanceStatus()
        {
            if (CurrentViewModelInstance != null)
            {
                var status = _isInstanceBound ? "Bound" : "Created (not bound)";
                var viewModelName = CurrentViewModelInstance.ViewModel?.Name ?? "Unknown";
                InstanceStatusText = $"Instance: {viewModelName} - {status}";
            }
            else
            {
                InstanceStatusText = string.Empty;
            }
        }

        private void ClearCurrentInstance()
        {
            CurrentViewModelInstance = null;
            _isInstanceBound = false;
            OnPropertyChanged(nameof(HasProperties));
            ClearPropertyControls();
        }

        private void GeneratePropertyControls()
        {
            ClearPropertyControls();

            if (!_isInstanceBound || CurrentViewModelInstance == null)
            {
                OnPropertyChanged(nameof(HasProperties));
                return;
            }

            try
            {
                // Get properties from the view model instance
                var properties = CurrentViewModelInstance.GetProperties();
                foreach (var property in properties)
                {
                    var controlContainer = CreatePropertyControl(property);
                    if (controlContainer != null)
                    {
                        DynamicPropertiesPanel.Children.Add(controlContainer);
                    }
                }

                OnPropertyChanged(nameof(HasProperties));
            }
            catch (Exception ex)
            {
                SetError($"Failed to generate property controls: {ex.Message}");
            }
        }

        private void ClearPropertyControls()
        {
            DynamicPropertiesPanel.Children.Clear();
            _propertyControls.Clear();
        }

        private FrameworkElement? CreatePropertyControl(ViewModelInstanceProperty property)
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
                Text = property.Name,
                Style = Application.Current.Resources["BodyTextBlockStyle"] as Style,
                Margin = new Thickness(0, 0, 0, 4)
            };
            container.Children.Add(label);

            FrameworkElement? inputControl = null;

            switch (property.Type)
            {
                case ViewModelPropertyType.String:
                    inputControl = CreateStringPropertyControl(property);
                    break;

                case ViewModelPropertyType.Number:
                    inputControl = CreateNumberPropertyControl(property);
                    break;

                case ViewModelPropertyType.Boolean:
                    inputControl = CreateBooleanPropertyControl(property);
                    break;

                case ViewModelPropertyType.Color:
                    inputControl = CreateColorPropertyControl(property);
                    break;

                case ViewModelPropertyType.Enum:
                    inputControl = CreateEnumPropertyControl(property);
                    break;

                case ViewModelPropertyType.Trigger:
                    inputControl = CreateTriggerPropertyControl(property);
                    break;
            }

            if (inputControl != null)
            {
                container.Children.Add(inputControl);
                _propertyControls[property.Name] = inputControl;
                return container;
            }

            return null;
        }

        private TextBox CreateStringPropertyControl(ViewModelInstanceProperty property)
        {
            var textBox = new TextBox
            {
                Text = property.StringValue ?? string.Empty,
                HorizontalAlignment = HorizontalAlignment.Stretch
            };

            textBox.TextChanged += (s, e) =>
            {
                property.StringValue = textBox.Text;
                ApplyPropertyToRiveControl(property);
            };

            return textBox;
        }

        private Slider CreateNumberPropertyControl(ViewModelInstanceProperty property)
        {
            var slider = new Slider
            {
                Minimum = 0,
                Maximum = 100,
                Value = property.NumberValue,
                HorizontalAlignment = HorizontalAlignment.Stretch
            };

            slider.ValueChanged += (s, e) =>
            {
                property.NumberValue = slider.Value;
                ApplyPropertyToRiveControl(property);
            };

            return slider;
        }

        private ToggleSwitch CreateBooleanPropertyControl(ViewModelInstanceProperty property)
        {
            var toggle = new ToggleSwitch
            {
                IsOn = property.BooleanValue,
                HorizontalAlignment = HorizontalAlignment.Left
            };

            toggle.Toggled += (s, e) =>
            {
                property.BooleanValue = toggle.IsOn;
                ApplyPropertyToRiveControl(property);
            };

            return toggle;
        }

        private TextBox CreateColorPropertyControl(ViewModelInstanceProperty property)
        {
            // For now, use a TextBox for color input (hex format)
            // TODO: Use ColorPicker when available
            var textBox = new TextBox
            {
                Text = $"#{property.ColorValue:X8}",
                HorizontalAlignment = HorizontalAlignment.Stretch,
                PlaceholderText = "#AARRGGBB"
            };

            textBox.TextChanged += (s, e) =>
            {
                if (uint.TryParse(textBox.Text.Replace("#", ""), System.Globalization.NumberStyles.HexNumber, null, out uint colorValue))
                {
                    property.ColorValue = colorValue;
                    ApplyPropertyToRiveControl(property);
                }
            };

            return textBox;
        }

        private ComboBox CreateEnumPropertyControl(ViewModelInstanceProperty property)
        {
            // For now, create a simple numeric ComboBox
            // TODO: Get actual enum values from ViewModel API
            var comboBox = new ComboBox
            {
                HorizontalAlignment = HorizontalAlignment.Stretch
            };

            // Add some placeholder enum values
            for (int i = 0; i < 5; i++)
            {
                comboBox.Items.Add($"Option {i}");
            }

            comboBox.SelectedIndex = Math.Max(0, Math.Min(property.EnumValue, comboBox.Items.Count - 1));

            comboBox.SelectionChanged += (s, e) =>
            {
                property.EnumValue = comboBox.SelectedIndex;
                ApplyPropertyToRiveControl(property);
            };

            return comboBox;
        }

        private Button CreateTriggerPropertyControl(ViewModelInstanceProperty property)
        {
            var button = new Button
            {
                Content = $"Fire {property.Name}",
                HorizontalAlignment = HorizontalAlignment.Left
            };

            button.Click += (s, e) =>
            {
                property.Fire();
                ApplyPropertyToRiveControl(property);
            };

            return button;
        }

        private void ApplyPropertyToRiveControl(ViewModelInstanceProperty property)
        {
            try
            {
                var riveControl = RiveViewer?.GetRiveControl();
                if (riveControl == null) return;

                switch (property.Type)
                {
                    case ViewModelPropertyType.String:
                        riveControl.SetViewModelStringProperty(property.Name, property.StringValue ?? string.Empty);
                        break;

                    case ViewModelPropertyType.Number:
                        riveControl.SetViewModelNumberProperty(property.Name, property.NumberValue);
                        break;

                    case ViewModelPropertyType.Boolean:
                        riveControl.SetViewModelBooleanProperty(property.Name, property.BooleanValue);
                        break;

                    case ViewModelPropertyType.Color:
                        riveControl.SetViewModelColorProperty(property.Name, property.ColorValue);
                        break;

                    case ViewModelPropertyType.Enum:
                        riveControl.SetViewModelEnumProperty(property.Name, property.EnumValue);
                        break;

                    case ViewModelPropertyType.Trigger:
                        riveControl.FireViewModelTrigger(property.Name);
                        break;
                }
            }
            catch (Exception ex)
            {
                SetError($"Failed to apply property '{property.Name}': {ex.Message}");
            }
        }

        private void LoadViewModelsFromFile(RiveFileSource fileSource)
        {
            try
            {
                ViewModels.Clear();
                ClearCurrentInstance();
                
                // Get the RiveControl instance from the connected viewer
                var riveControl = RiveViewer?.GetRiveControl();
                if (riveControl == null)
                {
                    UpdateStatus("No RiveControl available - cannot load view models");
                    return;
                }

                // Use the real RiveControl API to get view models
                var viewModelsInfo = riveControl.GetViewModels();
                if (viewModelsInfo == null || viewModelsInfo.Count == 0)
                {
                    UpdateStatus("No view models found in the loaded Rive file");
                    OnPropertyChanged(nameof(HasViewModels));
                    return;
                }

                // Load each view model using the API
                foreach (var vmInfo in viewModelsInfo)
                {
                    try
                    {
                        var viewModel = riveControl.GetViewModelByName(vmInfo.Name);
                        if (viewModel != null)
                        {
                            ViewModels.Add(viewModel);
                        }
                        else
                        {
                            // Try by index if name lookup fails
                            viewModel = riveControl.GetViewModelAt(vmInfo.Index);
                            if (viewModel != null)
                            {
                                ViewModels.Add(viewModel);
                            }
                        }
                    }
                    catch (Exception vmEx)
                    {
                        // Log but don't fail the entire operation for one view model
                        System.Diagnostics.Debug.WriteLine($"Failed to load view model '{vmInfo.Name}': {vmEx.Message}");
                    }
                }
                
                // Select the default view model, or first available
                if (ViewModels.Count > 0)
                {
                    try
                    {
                        var defaultViewModel = riveControl.GetDefaultViewModel();
                        if (defaultViewModel != null && ViewModels.Any(vm => vm.Name == defaultViewModel.Name))
                        {
                            SelectedViewModel = ViewModels.First(vm => vm.Name == defaultViewModel.Name);
                        }
                        else
                        {
                            // Select the first available view model
                            SelectedViewModel = ViewModels[0];
                        }
                    }
                    catch (Exception defaultEx)
                    {
                        // If we can't get default, just select the first one
                        System.Diagnostics.Debug.WriteLine($"Failed to select default view model: {defaultEx.Message}");
                        SelectedViewModel = ViewModels[0];
                    }
                }

                OnPropertyChanged(nameof(HasViewModels));
                UpdateStatus($"Loaded {ViewModels.Count} view model{(ViewModels.Count != 1 ? "s" : "")} from {fileSource.DisplayName}");
                ClearError();
            }
            catch (Exception ex)
            {
                SetError($"Failed to load view models: {ex.Message}");
                OnPropertyChanged(nameof(HasViewModels));
            }
        }

        private void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string? propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        private void OnFileSelected(RiveFileSource fileSource)
        {
            FileSelected?.Invoke(this, new FileSelectedEventArgs(fileSource));
        }

        private void OnViewModelSelected(ViewModel viewModel)
        {
            ViewModelSelected?.Invoke(this, new ViewModelSelectedEventArgs(viewModel));
        }

        private void OnViewModelInstanceBound(ViewModelInstance instance)
        {
            ViewModelInstanceBound?.Invoke(this, new ViewModelInstanceBoundEventArgs(instance));
        }

        private void OnStatusChanged(string status)
        {
            StatusChanged?.Invoke(this, new StatusChangedEventArgs(status));
        }

        #endregion

        #region Event Handlers

        private async void RiveViewModelPanel_Loaded(object sender, RoutedEventArgs e)
        {
            // Load default files when control loads
            await LoadDefaultFilesAsync();
        }

        private void RiveViewModelPanel_Unloaded(object sender, RoutedEventArgs e)
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

        private void ViewModelComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.FirstOrDefault() is ViewModel selectedViewModel)
            {
                OnViewModelSelected(selectedViewModel);
            }
        }

        private void CreateInstanceButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                if (SelectedViewModel == null)
                {
                    SetError("No view model selected");
                    return;
                }

                var riveControl = RiveViewer?.GetRiveControl();
                if (riveControl == null)
                {
                    SetError("No RiveControl available");
                    return;
                }

                // Create instance by name for selected view model
                var instance = riveControl.CreateViewModelInstanceByName(SelectedViewModel.Name);
                if (instance != null)
                {
                    CurrentViewModelInstance = instance;
                    UpdateStatus($"Created instance for view model '{SelectedViewModel.Name}'");
                    ClearError();
                }
                else
                {
                    SetError($"Failed to create instance for view model '{SelectedViewModel.Name}'");
                }
            }
            catch (Exception ex)
            {
                SetError($"Failed to create view model instance: {ex.Message}");
            }
        }

        private void BindInstanceButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                if (CurrentViewModelInstance == null)
                {
                    SetError("No view model instance to bind");
                    return;
                }

                var riveControl = RiveViewer?.GetRiveControl();
                if (riveControl == null)
                {
                    SetError("No RiveControl available");
                    return;
                }

                // Bind the instance
                var success = riveControl.BindViewModelInstance(CurrentViewModelInstance);
                if (success)
                {
                    _isInstanceBound = true;
                    OnPropertyChanged(nameof(CanBindInstance));
                    OnPropertyChanged(nameof(HasProperties));
                    UpdateInstanceStatus();
                    GeneratePropertyControls();
                    UpdateStatus($"Bound view model instance '{CurrentViewModelInstance.ViewModel?.Name}'");
                    ClearError();
                    
                    OnViewModelInstanceBound(CurrentViewModelInstance);
                }
                else
                {
                    SetError("Failed to bind view model instance");
                }
            }
            catch (Exception ex)
            {
                SetError($"Failed to bind view model instance: {ex.Message}");
            }
        }

        private void ClearInstanceButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                ClearCurrentInstance();
                UpdateStatus("Cleared view model instance");
                ClearError();
            }
            catch (Exception ex)
            {
                SetError($"Failed to clear instance: {ex.Message}");
            }
        }

        private void RiveViewer_FileLoaded(object sender, RiveFileLoadedEventArgs e)
        {
            LoadViewModelsFromFile(e.FileSource);
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
    /// Event arguments for view model selection events
    /// </summary>
    public class ViewModelSelectedEventArgs : EventArgs
    {
        public ViewModel ViewModel { get; }

        public ViewModelSelectedEventArgs(ViewModel viewModel)
        {
            ViewModel = viewModel ?? throw new ArgumentNullException(nameof(viewModel));
        }
    }

    /// <summary>
    /// Event arguments for view model instance bound events
    /// </summary>
    public class ViewModelInstanceBoundEventArgs : EventArgs
    {
        public ViewModelInstance Instance { get; }

        public ViewModelInstanceBoundEventArgs(ViewModelInstance instance)
        {
            Instance = instance ?? throw new ArgumentNullException(nameof(instance));
        }
    }

    #endregion
}
