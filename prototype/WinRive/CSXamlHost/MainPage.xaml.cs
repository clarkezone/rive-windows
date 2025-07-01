using System.Diagnostics;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Hosting;

namespace CSXamlHost
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a <see cref="Frame">.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        private WinRive.RiveControl _riveControl;
        private const string RiveFileName = "vector_feathering__avatar.riv";

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

            // Initialize the Rive control with the compositor and initial size
            if (_riveControl.Initialize(compositor, (int)RiveContainer.ActualWidth, (int)RiveContainer.ActualHeight))
            {
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
                }
                else
                {
                    Debug.WriteLine($"Failed to load Rive file: {RiveFileName}");
                }
            }
            else
            {
                Debug.WriteLine("Failed to initialize Rive control");
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
            } 
            else 
            {
                Debug.WriteLine("No Rive file selected.");
            }
        }
    }
}
