using System;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;
using CSXamlHost.Controls;
using CSXamlHost.Models;

namespace CSXamlHost
{
    /// <summary>
    /// Main page for the Rive UWP demo application.
    /// Uses the refactored RiveViewerControl and RiveStateMachinePanel architecture
    /// for clean separation of concerns between rendering and UI manipulation.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();
            this.Loaded += MainPage_Loaded;
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);
            
            // Handle any navigation parameters if needed
            if (e.Parameter != null)
            {
                System.Diagnostics.Debug.WriteLine($"MainPage navigated with parameter: {e.Parameter}");
            }
        }

        private async void MainPage_Loaded(object sender, RoutedEventArgs e)
        {
            try
            {
                // Initialize the application with proper error handling
                await InitializeApplicationAsync();
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Error during MainPage initialization: {ex}");
                
                // Show error to user (you could implement a more sophisticated error dialog)
                var errorDialog = new Windows.UI.Popups.MessageDialog(
                    $"Failed to initialize the application: {ex.Message}",
                    "Initialization Error");
                await errorDialog.ShowAsync();
            }
        }

        private async System.Threading.Tasks.Task InitializeApplicationAsync()
        {
            // Set up event handlers for the controls
            SetupEventHandlers();
            
            // The controls will initialize themselves automatically
            // RiveViewerControl will set up the composition and rendering
            // RiveStateMachinePanel will load available files and connect to the viewer
            
            System.Diagnostics.Debug.WriteLine("MainPage initialized successfully with new control architecture");
        }

        private void SetupEventHandlers()
        {
            // Subscribe to events from our controls for additional application-level handling
            
            // RiveViewer events
            if (RiveViewer != null)
            {
                RiveViewer.FileLoaded += RiveViewer_FileLoaded;
                RiveViewer.ErrorOccurred += RiveViewer_ErrorOccurred;
            }

            // StateMachinePanel events  
            if (StateMachinePanel != null)
            {
                StateMachinePanel.FileSelected += StateMachinePanel_FileSelected;
                StateMachinePanel.StateMachineSelected += StateMachinePanel_StateMachineSelected;
                StateMachinePanel.StatusChanged += StateMachinePanel_StatusChanged;
            }
        }

        #region Event Handlers

        private void RiveViewer_FileLoaded(object sender, RiveFileLoadedEventArgs e)
        {
            System.Diagnostics.Debug.WriteLine($"MainPage: File loaded successfully - {e.FileSource.DisplayName}");
            
            // You could add application-level logic here, such as:
            // - Analytics tracking
            // - Recent files management
            // - Window title updates
        }

        private void RiveViewer_ErrorOccurred(object sender, RiveErrorEventArgs e)
        {
            System.Diagnostics.Debug.WriteLine($"MainPage: RiveViewer error - {e.ErrorMessage}");
            
            // You could add application-level error handling here, such as:
            // - Error logging
            // - User notification
            // - Fallback file loading
        }

        private void StateMachinePanel_FileSelected(object sender, FileSelectedEventArgs e)
        {
            System.Diagnostics.Debug.WriteLine($"MainPage: File selected - {e.FileSource.DisplayName}");
            
            // You could add application-level logic here, such as:
            // - Update window title with file name
            // - Save user preferences
            // - Update recent files list
        }

        private void StateMachinePanel_StateMachineSelected(object sender, StateMachineSelectedEventArgs e)
        {
            System.Diagnostics.Debug.WriteLine($"MainPage: State machine selected - {e.StateMachine.Name}");
            
            // You could add application-level logic here, such as:
            // - Analytics tracking
            // - Save user preferences for state machine selection
        }

        private void StateMachinePanel_StatusChanged(object sender, StatusChangedEventArgs e)
        {
            System.Diagnostics.Debug.WriteLine($"MainPage: Status changed - {e.Status}");
            
            // You could add application-level logic here, such as:
            // - Update application status bar
            // - Log status changes
            // - Update window title with status
        }

        #endregion

        #region Cleanup

        private void MainPage_Unloaded(object sender, RoutedEventArgs e)
        {
            // Clean up event handlers to prevent memory leaks
            if (RiveViewer != null)
            {
                RiveViewer.FileLoaded -= RiveViewer_FileLoaded;
                RiveViewer.ErrorOccurred -= RiveViewer_ErrorOccurred;
            }

            if (StateMachinePanel != null)
            {
                StateMachinePanel.FileSelected -= StateMachinePanel_FileSelected;
                StateMachinePanel.StateMachineSelected -= StateMachinePanel_StateMachineSelected;
                StateMachinePanel.StatusChanged -= StateMachinePanel_StatusChanged;
            }

            System.Diagnostics.Debug.WriteLine("MainPage cleanup completed");
        }

        #endregion
    }
}
