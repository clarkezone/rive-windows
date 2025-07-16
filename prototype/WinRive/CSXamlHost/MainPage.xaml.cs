using System.Diagnostics;
using System.Collections.Generic;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Hosting;
using System.Collections.ObjectModel;

namespace CSXamlHost
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a <see cref="Frame">.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        private WinRive.RiveControl _riveControl;
        private const string RiveFileName = "vector_feathering__avatar.riv";
        private IReadOnlyList<WinRive.StateMachineInfo> _stateMachines;
        private WinRive.StateMachineInfo _currentStateMachine;

        public MainPage()
        {
            InitializeComponent();
            Loaded += MainPage_Loaded;
            SizeChanged += MainPage_SizeChanged;
        }

        private void MainPage_Loaded(object sender, RoutedEventArgs e)
        {
            // Create the Rive control
            _riveControl = new WinRive.RiveControl();

            // Get the compositor from the XAML visual tree
            var compositor = ElementCompositionPreview.GetElementVisual(RiveContainer).Compositor;

            // Get the CoreWindow for input handling
            var coreWindow = Windows.UI.Core.CoreWindow.GetForCurrentThread();

            // Initialize the Rive control with the compositor, CoreWindow, and initial size
            if (_riveControl.InitializeWithCoreWindow(compositor, coreWindow, (int)RiveContainer.ActualWidth, (int)RiveContainer.ActualHeight))
            {
                // Test the new hosting mode functionality
                var hostingMode = _riveControl.GetHostingMode();
                Debug.WriteLine($"WinRive initialized with hosting mode: {hostingMode}");
                UpdateStatus($"Hosting mode: {hostingMode}");
                // Get the visual from the Rive control
                var riveVisual = _riveControl.GetVisual();

                // Set the visual as the root visual for the XAML element
                ElementCompositionPreview.SetElementChildVisual(RiveContainer, riveVisual);

                // Load the Rive file from the package
                if (_riveControl.LoadRiveFileFromPackage(RiveFileName))
                {
                    Debug.WriteLine($"Successfully loaded Rive file: {RiveFileName}");
                    
                    // Start the render loop
                    _riveControl.StartRenderLoop();

                    // Load state machines after successful file load
                    LoadStateMachines();
                }
                else
                {
                    Debug.WriteLine($"Failed to load Rive file: {RiveFileName}");
                    UpdateStatus("Failed to load Rive file");
                }
            }
            else
            {
                Debug.WriteLine("Failed to initialize Rive control");
                UpdateStatus("Failed to initialize Rive control");
            }
        }

        private void MainPage_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            // Update the size of the Rive control when the container size changes
            if (_riveControl != null)
            {
                _riveControl.SetSize((int)RiveContainer.ActualWidth, (int)RiveContainer.ActualHeight);
            }
        }

        private void ComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count > 0 && _riveControl != null)
            {
                var selectedItem = (ComboBoxItem)e.AddedItems[0];
                string riverFileName = (string)selectedItem.Tag;
                _riveControl.LoadRiveFileFromPackage(riverFileName);
                Debug.WriteLine($"Selected Rive file: {riverFileName}");
                
                // Reload state machines for the new file
                LoadStateMachines();
            } 
            else 
            {
                Debug.WriteLine("No Rive file selected.");
            }
        }

        private void LoadStateMachines()
        {
            try
            {
                if (_riveControl == null)
                {
                    UpdateStatus("No Rive control available");
                    return;
                }

                // Get state machines from the Rive control
                _stateMachines = _riveControl.GetStateMachines();
                
                // Clear the combo box
                StateMachineComboBox.Items.Clear();
                
                if (_stateMachines.Count == 0)
                {
                    UpdateStatus("No state machines found in this Rive file");
                    StateMachineInfoText.Text = "No state machines available";
                    DisableControls();
                    return;
                }

                // Populate the combo box with state machines
                for (int i = 0; i < _stateMachines.Count; i++)
                {
                    var stateMachine = _stateMachines[i];
                    var item = new ComboBoxItem
                    {
                        Content = $"{stateMachine.Name} {(stateMachine.IsDefault ? "(Default)" : "")}",
                        Tag = i
                    };
                    StateMachineComboBox.Items.Add(item);
                }

                // Select the default state machine if available
                var defaultStateMachine = _riveControl.GetDefaultStateMachine();
                if (defaultStateMachine != null)
                {
                    for (int i = 0; i < _stateMachines.Count; i++)
                    {
                        if (_stateMachines[i].Index == defaultStateMachine.Index)
                        {
                            StateMachineComboBox.SelectedIndex = i;
                            break;
                        }
                    }
                }
                else if (_stateMachines.Count > 0)
                {
                    StateMachineComboBox.SelectedIndex = 0;
                }

                UpdateStatus($"Found {_stateMachines.Count} state machine(s)");
            }
            catch (System.Exception ex)
            {
                UpdateStatus($"Error loading state machines: {ex.Message}");
                Debug.WriteLine($"Error in LoadStateMachines: {ex}");
            }
        }

        private void StateMachineComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count > 0 && _riveControl != null && _stateMachines != null)
            {
                var selectedItem = (ComboBoxItem)e.AddedItems[0];
                int index = (int)selectedItem.Tag;
                
                if (index >= 0 && index < _stateMachines.Count)
                {
                    _currentStateMachine = _stateMachines[index];
                    
                    // Set the active state machine
                    _riveControl.SetActiveStateMachine(_currentStateMachine.Index);
                    
                    // Update UI
                    UpdateStateMachineInfo();
                    LoadStateMachineInputs();
                    EnableControls();
                    
                    UpdateStatus($"Selected state machine: {_currentStateMachine.Name}");
                }
            }
        }

        private void UpdateStateMachineInfo()
        {
            if (_currentStateMachine != null)
            {
                StateMachineInfoText.Text = $"Name: {_currentStateMachine.Name}\n" +
                                          $"Index: {_currentStateMachine.Index}\n" +
                                          $"Default: {(_currentStateMachine.IsDefault ? "Yes" : "No")}";
            }
            else
            {
                StateMachineInfoText.Text = "No state machine selected";
            }
        }

        private void LoadStateMachineInputs()
        {
            // Clear existing inputs
            InputsPanel.Children.Clear();

            if (_riveControl == null || _currentStateMachine == null)
                return;

            try
            {
                var inputs = _riveControl.GetStateMachineInputs();
                
                if (inputs.Count == 0)
                {
                    var noInputsText = new TextBlock
                    {
                        Text = "No inputs available",
                        FontStyle = Windows.UI.Text.FontStyle.Italic,
                        Margin = new Thickness(0, 5, 0, 5)
                    };
                    InputsPanel.Children.Add(noInputsText);
                    return;
                }

                foreach (var input in inputs)
                {
                    var inputContainer = new StackPanel
                    {
                        Orientation = Orientation.Horizontal,
                        Margin = new Thickness(0, 5, 0, 5)
                    };

                    var label = new TextBlock
                    {
                        Text = $"{input.Name}:",
                        Width = 100,
                        VerticalAlignment = VerticalAlignment.Center
                    };
                    inputContainer.Children.Add(label);

                    switch (input.Type)
                    {
                        case "Boolean":
                            var toggleSwitch = new ToggleSwitch
                            {
                                Tag = input.Name,
                                IsOn = input.BooleanValue
                            };
                            toggleSwitch.Toggled += (s, e) =>
                            {
                                _riveControl.SetBooleanInput((string)toggleSwitch.Tag, toggleSwitch.IsOn);
                            };
                            inputContainer.Children.Add(toggleSwitch);
                            break;

                        case "Number":
                            var numberBox = new TextBox
                            {
                                Tag = input.Name,
                                Text = input.NumberValue.ToString(),
                                Width = 100
                            };
                            numberBox.TextChanged += (s, e) =>
                            {
                                if (double.TryParse(numberBox.Text, out double value))
                                {
                                    _riveControl.SetNumberInput((string)numberBox.Tag, value);
                                }
                            };
                            inputContainer.Children.Add(numberBox);
                            break;

                        case "Trigger":
                            var triggerButton = new Button
                            {
                                Content = "Fire",
                                Tag = input.Name,
                                Margin = new Thickness(5, 0, 0, 0)
                            };
                            triggerButton.Click += (s, e) =>
                            {
                                _riveControl.FireTrigger((string)triggerButton.Tag);
                            };
                            inputContainer.Children.Add(triggerButton);
                            break;
                    }

                    InputsPanel.Children.Add(inputContainer);
                }
            }
            catch (System.Exception ex)
            {
                UpdateStatus($"Error loading inputs: {ex.Message}");
                Debug.WriteLine($"Error in LoadStateMachineInputs: {ex}");
            }
        }

        private void PlayButton_Click(object sender, RoutedEventArgs e)
        {
            if (_riveControl != null)
            {
                _riveControl.PlayStateMachine();
                UpdateStatus("State machine playing");
            }
        }

        private void PauseButton_Click(object sender, RoutedEventArgs e)
        {
            if (_riveControl != null)
            {
                _riveControl.PauseStateMachine();
                UpdateStatus("State machine paused");
            }
        }

        private void ResetButton_Click(object sender, RoutedEventArgs e)
        {
            if (_riveControl != null)
            {
                _riveControl.ResetStateMachine();
                UpdateStatus("State machine reset");
                
                // Reload inputs to refresh their values
                LoadStateMachineInputs();
            }
        }

        private void RefreshButton_Click(object sender, RoutedEventArgs e)
        {
            LoadStateMachines();
        }

        private void EnableControls()
        {
            PlayButton.IsEnabled = true;
            PauseButton.IsEnabled = true;
            ResetButton.IsEnabled = true;
        }

        private void DisableControls()
        {
            PlayButton.IsEnabled = false;
            PauseButton.IsEnabled = false;
            ResetButton.IsEnabled = false;
        }

        private void UpdateStatus(string message)
        {
            StatusText.Text = message;
            Debug.WriteLine($"Status: {message}");
        }
    }
}
