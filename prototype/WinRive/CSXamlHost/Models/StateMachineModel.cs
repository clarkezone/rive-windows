using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;

namespace CSXamlHost.Models
{
    /// <summary>
    /// Represents a state machine with its metadata and inputs
    /// </summary>
    public class StateMachineModel : INotifyPropertyChanged
    {
        private bool _isActive;
        private bool _isPlaying;
        private string _status = "Ready";

        public string Name { get; }
        public int Index { get; }
        public bool IsDefault { get; }

        /// <summary>
        /// Collection of inputs for this state machine
        /// </summary>
        public ObservableCollection<StateMachineInputModel> Inputs { get; }

        /// <summary>
        /// Indicates whether this state machine is currently active
        /// </summary>
        public bool IsActive
        {
            get => _isActive;
            set => SetProperty(ref _isActive, value);
        }

        /// <summary>
        /// Indicates whether this state machine is currently playing
        /// </summary>
        public bool IsPlaying
        {
            get => _isPlaying;
            set => SetProperty(ref _isPlaying, value);
        }

        /// <summary>
        /// Current status of the state machine
        /// </summary>
        public string Status
        {
            get => _status;
            set => SetProperty(ref _status, value);
        }

        /// <summary>
        /// Gets the number of inputs in this state machine
        /// </summary>
        public int InputCount => Inputs.Count;

        /// <summary>
        /// Gets a summary of input types
        /// </summary>
        public string InputSummary
        {
            get
            {
                if (Inputs.Count == 0)
                    return "No inputs";

                var booleanCount = Inputs.Count(i => i.Type == StateMachineInputType.Boolean);
                var numberCount = Inputs.Count(i => i.Type == StateMachineInputType.Number);
                var triggerCount = Inputs.Count(i => i.Type == StateMachineInputType.Trigger);

                var parts = new[]
                {
                    booleanCount > 0 ? $"{booleanCount} Boolean" : null,
                    numberCount > 0 ? $"{numberCount} Number" : null,
                    triggerCount > 0 ? $"{triggerCount} Trigger" : null
                }.Where(p => p != null);

                return string.Join(", ", parts);
            }
        }

        /// <summary>
        /// Gets a user-friendly display name
        /// </summary>
        public string DisplayName => IsDefault ? $"{Name} (Default)" : Name;

        public event PropertyChangedEventHandler? PropertyChanged;

        /// <summary>
        /// Constructor for StateMachineModel
        /// </summary>
        /// <param name="name">Name of the state machine</param>
        /// <param name="index">Index of the state machine</param>
        /// <param name="isDefault">Whether this is the default state machine</param>
        public StateMachineModel(string name, int index, bool isDefault = false)
        {
            Name = name ?? throw new ArgumentNullException(nameof(name));
            Index = index;
            IsDefault = isDefault;
            Inputs = new ObservableCollection<StateMachineInputModel>();

            // Subscribe to collection changes to update computed properties
            Inputs.CollectionChanged += (s, e) =>
            {
                OnPropertyChanged(nameof(InputCount));
                OnPropertyChanged(nameof(InputSummary));

                // Subscribe to input property changes
                if (e.NewItems != null)
                {
                    foreach (StateMachineInputModel input in e.NewItems)
                    {
                        input.PropertyChanged += Input_PropertyChanged;
                        input.TriggerFired += Input_TriggerFired;
                    }
                }

                if (e.OldItems != null)
                {
                    foreach (StateMachineInputModel input in e.OldItems)
                    {
                        input.PropertyChanged -= Input_PropertyChanged;
                        input.TriggerFired -= Input_TriggerFired;
                    }
                }
            };
        }

        /// <summary>
        /// Creates a StateMachineModel from WinRive StateMachineInfo
        /// </summary>
        /// <param name="info">WinRive StateMachineInfo</param>
        /// <returns>New StateMachineModel</returns>
        public static StateMachineModel FromWinRiveInfo(WinRive.StateMachineInfo info)
        {
            return new StateMachineModel(info.Name, info.Index, info.IsDefault);
        }

        /// <summary>
        /// Adds an input to this state machine
        /// </summary>
        /// <param name="input">The input to add</param>
        public void AddInput(StateMachineInputModel input)
        {
            if (input != null)
            {
                Inputs.Add(input);
            }
        }

        /// <summary>
        /// Creates a StateMachineModel from WinRive state machine info
        /// </summary>
        /// <param name="info">The WinRive state machine info</param>
        /// <returns>A new StateMachineModel instance</returns>
        public static StateMachineModel FromWinRiveInfo(dynamic info)
        {
            try
            {
                // Extract properties from WinRive state machine info
                string name = info?.Name ?? "Unknown";
                int index = info?.Index ?? 0;
                bool isDefault = info?.IsDefault ?? false;

                return new StateMachineModel(name, index, isDefault);
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Failed to create StateMachineModel from WinRive info: {ex.Message}");
                
                // Return a fallback state machine
                return new StateMachineModel("Unknown State Machine", 0, true);
            }
        }

        /// <summary>
        /// Removes all inputs from this state machine
        /// </summary>
        public void ClearInputs()
        {
            Inputs.Clear();
        }

        /// <summary>
        /// Gets an input by name
        /// </summary>
        /// <param name="inputName">Name of the input</param>
        /// <returns>Input model or null if not found</returns>
        public StateMachineInputModel? GetInput(string inputName)
        {
            return Inputs.FirstOrDefault(i => i.Name.Equals(inputName, StringComparison.OrdinalIgnoreCase));
        }

        /// <summary>
        /// Gets all inputs of a specific type
        /// </summary>
        /// <param name="inputType">Type of inputs to get</param>
        /// <returns>Collection of inputs of the specified type</returns>
        public ObservableCollection<StateMachineInputModel> GetInputsByType(StateMachineInputType inputType)
        {
            var filteredInputs = new ObservableCollection<StateMachineInputModel>();
            foreach (var input in Inputs.Where(i => i.Type == inputType))
            {
                filteredInputs.Add(input);
            }
            return filteredInputs;
        }

        /// <summary>
        /// Updates the status and notifies observers
        /// </summary>
        /// <param name="newStatus">New status message</param>
        public void UpdateStatus(string newStatus)
        {
            Status = newStatus;
        }

        /// <summary>
        /// Resets all inputs to their default values
        /// </summary>
        public void ResetInputs()
        {
            foreach (var input in Inputs)
            {
                switch (input.Type)
                {
                    case StateMachineInputType.Boolean:
                        input.BooleanValue = false;
                        break;
                    case StateMachineInputType.Number:
                        input.NumberValue = 0.0;
                        break;
                    // Triggers don't have persistent values
                }
            }

            UpdateStatus("Inputs reset");
        }

        /// <summary>
        /// Event fired when an input value changes
        /// </summary>
        public event EventHandler<InputChangedEventArgs>? InputChanged;

        /// <summary>
        /// Event fired when a trigger input is fired
        /// </summary>
        public event EventHandler<TriggerFiredEventArgs>? TriggerFired;

        private void Input_PropertyChanged(object? sender, PropertyChangedEventArgs e)
        {
            if (sender is StateMachineInputModel input && e.PropertyName == nameof(StateMachineInputModel.Value))
            {
                OnInputChanged(input);
            }
        }

        private void Input_TriggerFired(object? sender, EventArgs e)
        {
            if (sender is StateMachineInputModel input)
            {
                OnTriggerFired(input);
            }
        }

        private void OnInputChanged(StateMachineInputModel input)
        {
            InputChanged?.Invoke(this, new InputChangedEventArgs(input));
        }

        private void OnTriggerFired(StateMachineInputModel input)
        {
            TriggerFired?.Invoke(this, new TriggerFiredEventArgs(input));
        }

        private bool SetProperty<T>(ref T field, T value, [System.Runtime.CompilerServices.CallerMemberName] string? propertyName = null)
        {
            if (!Equals(field, value))
            {
                field = value;
                OnPropertyChanged(propertyName);
                return true;
            }
            return false;
        }

        private void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string? propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        public override string ToString()
        {
            return DisplayName;
        }

        public override bool Equals(object? obj)
        {
            if (obj is StateMachineModel other)
            {
                return Index == other.Index && Name.Equals(other.Name, StringComparison.OrdinalIgnoreCase);
            }
            return false;
        }

        public override int GetHashCode()
        {
            return HashCode.Combine(Name.ToLowerInvariant(), Index);
        }
    }

    /// <summary>
    /// Event arguments for input change events
    /// </summary>
    public class InputChangedEventArgs : EventArgs
    {
        public StateMachineInputModel Input { get; }

        public InputChangedEventArgs(StateMachineInputModel input)
        {
            Input = input ?? throw new ArgumentNullException(nameof(input));
        }
    }

    /// <summary>
    /// Event arguments for trigger fired events
    /// </summary>
    public class TriggerFiredEventArgs : EventArgs
    {
        public StateMachineInputModel TriggerInput { get; }

        public TriggerFiredEventArgs(StateMachineInputModel triggerInput)
        {
            TriggerInput = triggerInput ?? throw new ArgumentNullException(nameof(triggerInput));
        }
    }
}
