using System;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Data;

namespace CSXamlHost.Converters
{
    /// <summary>
    /// Converts string values to Visibility values (Visible if string is not null/empty)
    /// </summary>
    public class StringToVisibilityConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, string language)
        {
            if (value is string stringValue)
            {
                // Check if we should invert the result
                bool invert = parameter?.ToString()?.Equals("Invert", StringComparison.OrdinalIgnoreCase) == true;
                
                bool hasValue = !string.IsNullOrEmpty(stringValue);
                bool result = invert ? !hasValue : hasValue;
                
                return result ? Visibility.Visible : Visibility.Collapsed;
            }

            return Visibility.Collapsed;
        }

        public object ConvertBack(object value, Type targetType, object parameter, string language)
        {
            // Not typically needed for one-way binding, but implementing for completeness
            if (value is Visibility visibility)
            {
                return visibility == Visibility.Visible ? "Value" : string.Empty;
            }

            return string.Empty;
        }
    }
}
