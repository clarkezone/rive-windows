using System;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Data;

namespace CSXamlHost.Converters
{
    /// <summary>
    /// Converts boolean values to Visibility values
    /// </summary>
    public class BooleanToVisibilityConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, string language)
        {
            if (value is bool boolValue)
            {
                // Check if we should invert the result
                bool invert = parameter?.ToString()?.Equals("Invert", StringComparison.OrdinalIgnoreCase) == true;
                
                bool result = invert ? !boolValue : boolValue;
                return result ? Visibility.Visible : Visibility.Collapsed;
            }

            return Visibility.Collapsed;
        }

        public object ConvertBack(object value, Type targetType, object parameter, string language)
        {
            if (value is Visibility visibility)
            {
                bool result = visibility == Visibility.Visible;
                
                // Check if we should invert the result
                bool invert = parameter?.ToString()?.Equals("Invert", StringComparison.OrdinalIgnoreCase) == true;
                
                return invert ? !result : result;
            }

            return false;
        }
    }
}
