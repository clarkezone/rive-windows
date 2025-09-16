using System;
using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.Storage;
using CSXamlHost.Models;

namespace CSXamlHost.Services
{
    /// <summary>
    /// Service for managing Rive file operations including loading, validation, and enumeration
    /// </summary>
    public class RiveFileService
    {
        private static readonly string[] ValidRiveExtensions = { ".riv" };
        private const string RiveAssetsFolder = "Assets/RiveAssets";

        /// <summary>
        /// Gets all packaged Rive files from the application assets
        /// </summary>
        /// <returns>List of RiveFileSource objects for packaged files</returns>
        public async Task<List<RiveFileSource>> GetPackagedRiveFilesAsync()
        {
            var riveFiles = new List<RiveFileSource>();

            try
            {
                // Get the application package
                var packageFolder = Package.Current.InstalledLocation;
                
                // Try to get the RiveAssets folder
                var assetsFolder = await packageFolder.TryGetItemAsync("Assets") as StorageFolder;
                if (assetsFolder == null)
                {
                    return riveFiles; // Return empty list if Assets folder doesn't exist
                }

                var riveAssetsFolder = await assetsFolder.TryGetItemAsync("RiveAssets") as StorageFolder;
                if (riveAssetsFolder == null)
                {
                    return riveFiles; // Return empty list if RiveAssets folder doesn't exist
                }

                // Get all files in the RiveAssets folder
                var files = await riveAssetsFolder.GetFilesAsync();
                
                foreach (var file in files)
                {
                    if (IsValidRiveFile(file.Name))
                    {
                        var displayName = GetDisplayNameFromFilename(file.Name);
                        var relativePath = $"Assets/RiveAssets/{file.Name}";
                        
                        var riveFileSource = new RiveFileSource(
                            relativePath,
                            displayName,
                            RiveFileSourceType.Package
                        );

                        riveFiles.Add(riveFileSource);
                    }
                }
            }
            catch (Exception ex)
            {
                // Log error (in a real app, you'd use proper logging)
                System.Diagnostics.Debug.WriteLine($"Error loading packaged Rive files: {ex.Message}");
            }

            return riveFiles;
        }

        /// <summary>
        /// Creates a RiveFileSource from an external file
        /// </summary>
        /// <param name="file">StorageFile representing the external Rive file</param>
        /// <returns>RiveFileSource for the external file</returns>
        public RiveFileSource CreateExternalFileSource(StorageFile file)
        {
            if (file == null)
                throw new ArgumentNullException(nameof(file));

            var displayName = GetDisplayNameFromFilename(file.Name);
            
            var riveFileSource = new RiveFileSource(
                file.Path,
                displayName,
                RiveFileSourceType.External
            );

            return riveFileSource;
        }

        /// <summary>
        /// Validates whether a file is a valid Rive file
        /// </summary>
        /// <param name="filename">Name of the file to validate</param>
        /// <returns>True if the file appears to be a valid Rive file</returns>
        public bool IsValidRiveFile(string filename)
        {
            if (string.IsNullOrWhiteSpace(filename))
                return false;

            var extension = Path.GetExtension(filename).ToLowerInvariant();
            
            foreach (var validExtension in ValidRiveExtensions)
            {
                if (extension == validExtension)
                    return true;
            }

            return false;
        }

        /// <summary>
        /// Validates whether a StorageFile is a valid Rive file
        /// </summary>
        /// <param name="file">StorageFile to validate</param>
        /// <returns>True if the file appears to be a valid Rive file</returns>
        public bool IsValidRiveFile(StorageFile file)
        {
            if (file == null)
                return false;

            return IsValidRiveFile(file.Name);
        }

        /// <summary>
        /// Attempts to load file contents for validation (basic header check)
        /// </summary>
        /// <param name="file">StorageFile to validate</param>
        /// <returns>True if file appears to be a valid Rive file based on content</returns>
        public async Task<bool> ValidateRiveFileContentAsync(StorageFile file)
        {
            if (file == null)
                return false;

            try
            {
                // Basic validation - check if file exists and has reasonable size
                var properties = await file.GetBasicPropertiesAsync();
                
                // Rive files should be at least a few bytes and not excessively large
                if (properties.Size < 10 || properties.Size > 100 * 1024 * 1024) // 100MB max
                {
                    return false;
                }

                // Additional content validation could be added here
                // For now, we rely on extension and size checks
                return IsValidRiveFile(file.Name);
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Error validating Rive file content: {ex.Message}");
                return false;
            }
        }

        /// <summary>
        /// Gets a user-friendly display name from a filename
        /// </summary>
        /// <param name="filename">Original filename</param>
        /// <returns>User-friendly display name</returns>
        public string GetDisplayNameFromFilename(string filename)
        {
            if (string.IsNullOrWhiteSpace(filename))
                return "Unknown File";

            // Remove extension
            var nameWithoutExtension = Path.GetFileNameWithoutExtension(filename);
            
            // Replace underscores and hyphens with spaces
            var displayName = nameWithoutExtension
                .Replace('_', ' ')
                .Replace('-', ' ');

            // Capitalize first letter of each word
            var words = displayName.Split(' ', StringSplitOptions.RemoveEmptyEntries);
            for (int i = 0; i < words.Length; i++)
            {
                if (words[i].Length > 0)
                {
                    words[i] = char.ToUpperInvariant(words[i][0]) + 
                              (words[i].Length > 1 ? words[i].Substring(1).ToLowerInvariant() : "");
                }
            }

            return string.Join(" ", words);
        }

        /// <summary>
        /// Creates a list of default/sample Rive file sources
        /// This can be used as fallback or for demo purposes
        /// </summary>
        /// <returns>List of sample RiveFileSource objects</returns>
        public List<RiveFileSource> GetDefaultRiveFiles()
        {
            return new List<RiveFileSource>
            {
                new RiveFileSource("Assets/RiveAssets/vector_feathering__avatar.riv", "Vector Feathering Avatar", RiveFileSourceType.Package),
                new RiveFileSource("Assets/RiveAssets/meeting_ui.riv", "Meeting UI", RiveFileSourceType.Package),
                new RiveFileSource("Assets/RiveAssets/layouttest.riv", "Layout Test", RiveFileSourceType.Package),
                new RiveFileSource("Assets/RiveAssets/world_creator.riv", "World Creator", RiveFileSourceType.Package),
                new RiveFileSource("Assets/RiveAssets/pointer_events.riv", "Pointer Events", RiveFileSourceType.Package),
                new RiveFileSource("Assets/RiveAssets/interactive_bento_grid.riv", "Interactive Bento Grid", RiveFileSourceType.Package),
                new RiveFileSource("Assets/RiveAssets/the_sound_of_coqui.riv", "The Sound of Coqui", RiveFileSourceType.Package),
                new RiveFileSource("Assets/RiveAssets/datatest.riv", "Data Test", RiveFileSourceType.Package)
            };
        }

        /// <summary>
        /// Gets file size in a human-readable format
        /// </summary>
        /// <param name="file">StorageFile to get size for</param>
        /// <returns>Human-readable file size string</returns>
        public async Task<string> GetFileSizeStringAsync(StorageFile file)
        {
            if (file == null)
                return "Unknown";

            try
            {
                var properties = await file.GetBasicPropertiesAsync();
                var size = properties.Size;

                if (size < 1024)
                    return $"{size} B";
                else if (size < 1024 * 1024)
                    return $"{size / 1024.0:F1} KB";
                else if (size < 1024 * 1024 * 1024)
                    return $"{size / (1024.0 * 1024.0):F1} MB";
                else
                    return $"{size / (1024.0 * 1024.0 * 1024.0):F1} GB";
            }
            catch
            {
                return "Unknown";
            }
        }
    }
}
