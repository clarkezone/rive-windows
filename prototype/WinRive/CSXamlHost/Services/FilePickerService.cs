using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Windows.Storage;
using Windows.Storage.Pickers;
using CSXamlHost.Models;

namespace CSXamlHost.Services
{
    /// <summary>
    /// Service for handling file picker operations in UWP applications
    /// </summary>
    public class FilePickerService
    {
        private readonly RiveFileService _riveFileService;

        /// <summary>
        /// Constructor for FilePickerService
        /// </summary>
        /// <param name="riveFileService">Service for Rive file operations</param>
        public FilePickerService(RiveFileService riveFileService)
        {
            _riveFileService = riveFileService ?? throw new ArgumentNullException(nameof(riveFileService));
        }

        /// <summary>
        /// Opens a file picker to select a single Rive file
        /// </summary>
        /// <returns>RiveFileSource for the selected file, or null if no file was selected</returns>
        public async Task<RiveFileSource?> PickSingleRiveFileAsync()
        {
            try
            {
                var picker = new FileOpenPicker();
                
                // Configure the picker
                picker.ViewMode = PickerViewMode.List;
                picker.SuggestedStartLocation = PickerLocationId.Downloads;
                picker.FileTypeFilter.Add(".riv");

                // For WinUI 3 / UWP compatibility, we need to set the picker window
                if (Windows.ApplicationModel.Core.CoreApplication.MainView.CoreWindow != null)
                {
                    // UWP scenario
                    var file = await picker.PickSingleFileAsync();
                    return await ProcessSelectedFileAsync(file);
                }
                else
                {
                    // This would be for WinUI 3 scenarios, but since we're in UWP, 
                    // this branch shouldn't be hit
                    throw new NotSupportedException("This picker configuration is not supported in the current context.");
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Error picking Rive file: {ex.Message}");
                return null;
            }
        }

        /// <summary>
        /// Opens a file picker to select multiple Rive files
        /// </summary>
        /// <returns>List of RiveFileSource objects for selected files</returns>
        public async Task<List<RiveFileSource>> PickMultipleRiveFilesAsync()
        {
            var results = new List<RiveFileSource>();

            try
            {
                var picker = new FileOpenPicker();
                
                // Configure the picker
                picker.ViewMode = PickerViewMode.List;
                picker.SuggestedStartLocation = PickerLocationId.Downloads;
                picker.FileTypeFilter.Add(".riv");

                // For UWP
                if (Windows.ApplicationModel.Core.CoreApplication.MainView.CoreWindow != null)
                {
                    var files = await picker.PickMultipleFilesAsync();
                    
                    foreach (var file in files)
                    {
                        var riveFileSource = await ProcessSelectedFileAsync(file);
                        if (riveFileSource != null)
                        {
                            results.Add(riveFileSource);
                        }
                    }
                }
                else
                {
                    throw new NotSupportedException("This picker configuration is not supported in the current context.");
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Error picking multiple Rive files: {ex.Message}");
            }

            return results;
        }

        /// <summary>
        /// Opens a file picker with custom file type filters
        /// </summary>
        /// <param name="fileTypes">List of file extensions to filter (e.g., ".riv", ".json")</param>
        /// <param name="suggestedLocation">Suggested starting location for the picker</param>
        /// <returns>Selected StorageFile or null if no file was selected</returns>
        public async Task<StorageFile?> PickFileWithCustomFilterAsync(
            IList<string> fileTypes, 
            PickerLocationId suggestedLocation = PickerLocationId.Downloads)
        {
            try
            {
                var picker = new FileOpenPicker();
                
                // Configure the picker
                picker.ViewMode = PickerViewMode.List;
                picker.SuggestedStartLocation = suggestedLocation;
                
                // Add file type filters
                foreach (var fileType in fileTypes)
                {
                    picker.FileTypeFilter.Add(fileType);
                }

                // For UWP
                if (Windows.ApplicationModel.Core.CoreApplication.MainView.CoreWindow != null)
                {
                    return await picker.PickSingleFileAsync();
                }
                else
                {
                    throw new NotSupportedException("This picker configuration is not supported in the current context.");
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Error picking file with custom filter: {ex.Message}");
                return null;
            }
        }

        /// <summary>
        /// Opens a save file picker to save a file
        /// </summary>
        /// <param name="suggestedFileName">Suggested name for the file</param>
        /// <param name="fileType">File extension (e.g., ".riv")</param>
        /// <param name="fileTypeDisplayName">Display name for the file type (e.g., "Rive Files")</param>
        /// <returns>Selected StorageFile for saving, or null if cancelled</returns>
        public async Task<StorageFile?> PickSaveFileAsync(
            string suggestedFileName, 
            string fileType, 
            string fileTypeDisplayName)
        {
            try
            {
                var picker = new FileSavePicker();
                
                // Configure the picker
                picker.SuggestedStartLocation = PickerLocationId.Downloads;
                picker.SuggestedFileName = suggestedFileName;
                
                // Set up file type choices
                picker.FileTypeChoices.Add(fileTypeDisplayName, new List<string> { fileType });

                // For UWP
                if (Windows.ApplicationModel.Core.CoreApplication.MainView.CoreWindow != null)
                {
                    return await picker.PickSaveFileAsync();
                }
                else
                {
                    throw new NotSupportedException("This picker configuration is not supported in the current context.");
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Error picking save file: {ex.Message}");
                return null;
            }
        }

        /// <summary>
        /// Validates and processes a selected file
        /// </summary>
        /// <param name="file">StorageFile to process</param>
        /// <returns>RiveFileSource if valid, null otherwise</returns>
        private async Task<RiveFileSource?> ProcessSelectedFileAsync(StorageFile? file)
        {
            if (file == null)
                return null;

            // Validate the file
            if (!_riveFileService.IsValidRiveFile(file))
            {
                System.Diagnostics.Debug.WriteLine($"Selected file is not a valid Rive file: {file.Name}");
                return null;
            }

            // Perform content validation
            if (!await _riveFileService.ValidateRiveFileContentAsync(file))
            {
                System.Diagnostics.Debug.WriteLine($"Selected file failed content validation: {file.Name}");
                return null;
            }

            // Create and return RiveFileSource
            return _riveFileService.CreateExternalFileSource(file);
        }

        /// <summary>
        /// Shows a message to the user about picker limitations or errors
        /// This is a placeholder for actual UI notification system
        /// </summary>
        /// <param name="message">Message to display</param>
        private void ShowUserMessage(string message)
        {
            // In a real application, you would show this message through
            // a proper notification system, dialog, or status bar
            System.Diagnostics.Debug.WriteLine($"FilePickerService Message: {message}");
        }

        /// <summary>
        /// Gets the appropriate picker location based on file type
        /// </summary>
        /// <param name="isRiveFile">True if picking Rive files</param>
        /// <returns>Appropriate PickerLocationId</returns>
        public PickerLocationId GetSuggestedLocation(bool isRiveFile = true)
        {
            if (isRiveFile)
            {
                // For Rive files, start in Downloads or Documents
                return PickerLocationId.Downloads;
            }
            else
            {
                return PickerLocationId.DocumentsLibrary;
            }
        }

        /// <summary>
        /// Creates a configured file picker for Rive files
        /// </summary>
        /// <returns>Configured FileOpenPicker</returns>
        public FileOpenPicker CreateRiveFilePicker()
        {
            var picker = new FileOpenPicker();
            picker.ViewMode = PickerViewMode.List;
            picker.SuggestedStartLocation = PickerLocationId.Downloads;
            picker.FileTypeFilter.Add(".riv");
            return picker;
        }

        /// <summary>
        /// Creates a configured save file picker for Rive files
        /// </summary>
        /// <param name="suggestedFileName">Suggested filename</param>
        /// <returns>Configured FileSavePicker</returns>
        public FileSavePicker CreateRiveSavePicker(string suggestedFileName = "animation.riv")
        {
            var picker = new FileSavePicker();
            picker.SuggestedStartLocation = PickerLocationId.Downloads;
            picker.SuggestedFileName = suggestedFileName;
            picker.FileTypeChoices.Add("Rive Files", new List<string> { ".riv" });
            return picker;
        }
    }
}
