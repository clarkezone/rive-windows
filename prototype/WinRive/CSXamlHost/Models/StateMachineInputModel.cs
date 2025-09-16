using System;
using System.ComponentModel;

namespace CSXamlHost.Models
{
    /// <summary>
    /// MVVM-friendly wrapper for state machine inputs that supports data binding
    /// </summary>
    public class StateMachineInputModel : INotifyPropertyChanged
    {
        private object? _value;
        private bool _booleanValue;
        private double _numberValue;
        private string _stringValue = string.Empty;

        public string Name { get; }
        public StateMachineInputType Type { get; }
        public int Index { get; }

        /// <summary>
        /// Generic value property for binding - use type-specific properties when possible
        /// </summary>
        public object? Value
        {
            get => _value;
            set
            {
                if (SetProperty(ref _value, value))
                {
                    // Update type-specific properties based on the value
                    UpdateTypedProperties();
                }
            }
        }

        /// <summary>
        /// Boolean value for Boolean-type inputs
        /// </summary>
        public bool BooleanValue
        {
            get => _booleanValue;
            set
            {
                if (SetProperty(ref _booleanValue, value))
                {
                    _value = value;
                    OnPropertyChanged(nameof(Value));
                }
            }
        }

        /// <summary>
        /// Numeric value for Number-type inputs
        /// </summary>
        public double NumberValue
        {
            get => _numberValue;
            set
            {
                if (SetProperty(ref _numberValue, value))
                {
                    _value = value;
                    OnPropertyChanged(nameof(Value));
                }
            }
        }

        /// <summary>
        /// String representation of the value for display purposes
        /// </summary>
        public string StringValue
        {
            get => _stringValue;
            set => SetProperty(ref _stringValue, value);
        }

        /// <summary>
        /// Indicates whether this input can be modified by the user
        /// </summary>
        public bool IsEditable => Type != StateMachineInputType.Trigger;

        /// <summary>
        /// Indicates whether this is a trigger input
        /// </summary>
        public bool IsTrigger => Type == StateMachineInputType.Trigger;

        public event PropertyChangedEventHandler? PropertyChanged;

        /// <summary>
        /// Constructor for StateMachineInputModel
        /// </summary>
        /// <param name="name">Name of the input</param>
        /// <param name="type">Type of the input</param>
        /// <param name="index">Index of the input</param>
        /// <param name="initialValue">Initial value for the input</param>
        public StateMachineInputModel(string name, StateMachineInputType type, int index, object? initialValue = null)
        {
            Name = name ?? throw new ArgumentNullException(nameof(name));
            Type = type;
            Index = index;

            // Set initial value based on type
            if (initialValue != null)
            {
                _value = initialValue;
            }
            else
            {
                _value = GetDefaultValueForType(type);
            }

            UpdateTypedProperties();
            UpdateStringValue();
        }

        /// <summary>
        /// Creates a StateMachineInputModel from WinRive StateMachineInput
        /// </summary>
        /// <param name="input">WinRive StateMachineInput</param>
        /// <param name="index">Index of the input</param>
        /// <returns>New StateMachineInputModel</returns>
        public static StateMachineInputModel FromWinRiveInput(WinRive.StateMachineInput input, int index)
        {
            var type = ParseInputType(input.Type);
            object? value = type switch
            {
                StateMachineInputType.Boolean => input.BooleanValue,
                StateMachineInputType.Number => input.NumberValue,
                StateMachineInputType.Trigger => null,
                _ => null
            };

            return new StateMachineInputModel(input.Name, type, index, value);
        }

        /// <summary>
        /// Triggers the input (for trigger-type inputs)
        /// </summary>
        public void FireTrigger()
        {
            if (Type == StateMachineInputType.Trigger)
            {
                OnTriggerFired();
            }
        }

        /// <summary>
        /// Event fired when a trigger input is triggered
        /// </summary>
        public event EventHandler? TriggerFired;

        private static StateMachineInputType ParseInputType(string typeString)
        {
            return typeString?.ToLowerInvariant() switch
            {
                "boolean" => StateMachineInputType.Boolean,
                "number" => StateMachineInputType.Number,
                "trigger" => StateMachineInputType.Trigger,
                _ => StateMachineInputType.Unknown
            };
        }

        private static object GetDefaultValueForType(StateMachineInputType type)
        {
            return type switch
            {
                StateMachineInputType.Boolean => false,
                StateMachineInputType.Number => 0.0,
                StateMachineInputType.Trigger => new object(), // Placeholder
                _ => new object()
            };
        }

        private void UpdateTypedProperties()
        {
            switch (Type)
            {
                case StateMachineInputType.Boolean:
                    if (_value is bool boolValue)
                    {
                        _booleanValue = boolValue;
                        OnPropertyChanged(nameof(BooleanValue));
                    }
                    break;

                case StateMachineInputType.Number:
                    if (_value is double doubleValue)
                    {
                        _numberValue = doubleValue;
                        OnPropertyChanged(nameof(NumberValue));
                    }
                    else if (_value is int intValue)
                    {
                        _numberValue = intValue;
                        OnPropertyChanged(nameof(NumberValue));
                    }
                    else if (_value is float floatValue)
                    {
                        _numberValue = floatValue;
                        OnPropertyChanged(nameof(NumberValue));
                    }
                    break;
            }

            UpdateStringValue();
        }

        private void UpdateStringValue()
        {
            _stringValue = Type switch
            {
                StateMachineInputType.Boolean => _booleanValue.ToString(),
                StateMachineInputType.Number => _numberValue.ToString("F2"),
                StateMachineInputType.Trigger => "Trigger",
                _ => _value?.ToString() ?? "Unknown"
            };
            OnPropertyChanged(nameof(StringValue));
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

        private void OnTriggerFired()
        {
            TriggerFired?.Invoke(this, EventArgs.Empty);
        }

        public override string ToString()
        {
            return $"{Name} ({Type}): {StringValue}";
        }
    }

    /// <summary>
    /// Defines the types of state machine inputs
    /// </summary>
    public enum StateMachineInputType
    {
        Unknown,
        Boolean,
        Number,
        Trigger
    }
}
