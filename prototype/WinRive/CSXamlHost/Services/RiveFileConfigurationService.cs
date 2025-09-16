using System;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.Storage;
using CSXamlHost.Models;

namespace CSXamlHost.Services
{
    /// <summary>
    /// Service for loading and managing Rive file configurations
    /// </summary>
    public class RiveFileConfigurationService
    {
        private const string DefaultConfigurationFileName = "Assets/RiveFiles.json";
        private const string UserConfigurationFileName = "UserRiveFiles.json";
        private readonly List<RiveFileSource> _recentlyUsedFiles = new();
        private RiveFileConfiguration? _currentConfiguration;

        /// <summary>
        /// Maximum number of recently used files to track
        /// </summary>
        public int MaxRecentFiles { get; set; } = 10;

        /// <summary>
        /// Gets the currently loaded configuration
        /// </summary>
        public RiveFileConfiguration? CurrentConfiguration => _currentConfiguration;

        /// <summary>
        /// Gets the list of recently used files
        /// </summary>
        public IReadOnlyList<RiveFileSource> RecentlyUsedFiles => _recentlyUsedFiles.AsReadOnly();

        /// <summary>
        /// Event raised when configuration is loaded or changed
        /// </summary>
        public event EventHandler<ConfigurationLoadedEventArgs>? ConfigurationLoaded;

        /// <summary>
        /// Event raised when a file is added to recently used list
        /// </summary>
        public event EventHandler<FileUsedEventArgs>? FileUsed;

        /// <summary>
        /// Loads the default configuration from the package
        /// </summary>
        public async Task<RiveFileConfiguration> LoadDefaultConfigurationAsync()
        {
            try
            {
                // Try to get the Assets folder first
                var packageFolder = Package.Current.InstalledLocation;
                var assetsFolder = await packageFolder.GetFolderAsync("Assets");
                var configFile = await assetsFolder.GetFileAsync("RiveFiles.json");
                
                var configurationText = await FileIO.ReadTextAsync(configFile);
                System.Diagnostics.Debug.WriteLine($"Loaded configuration JSON: {configurationText}");
                
                var configuration = JsonSerializer.Deserialize<RiveFileConfiguration>(configurationText, GetJsonOptions());
                if (configuration == null)
                {
                    throw new InvalidOperationException("Failed to deserialize configuration file");
                }

                System.Diagnostics.Debug.WriteLine($"Loaded configuration with {configuration.AvailableFiles?.Count ?? 0} files");

                // Validate the configuration
                var validationResult = configuration.Validate();
                if (validationResult.HasErrors)
                {
                    System.Diagnostics.Debug.WriteLine($"Configuration validation failed: {validationResult}");
                    throw new InvalidOperationException($"Configuration validation failed: {validationResult}");
                }

                _currentConfiguration = configuration;
                OnConfigurationLoaded(configuration, "Assets/RiveFiles.json");
                
                return configuration;
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Failed to load default configuration: {ex.Message}");
                System.Diagnostics.Debug.WriteLine($"Exception details: {ex}");
                
                // Return a minimal fallback configuration
                var fallbackConfig = CreateFallbackConfiguration();
                _currentConfiguration = fallbackConfig;
                OnConfigurationLoaded(fallbackConfig, "fallback");
                
                return fallbackConfig;
            }
        }

        /// <summary>
        /// Loads a user-defined configuration from local storage
        /// </summary>
        public async Task<RiveFileConfiguration?> LoadUserConfigurationAsync()
        {
            try
            {
                var localFolder = ApplicationData.Current.LocalFolder;
                var configFile = await localFolder.GetFileAsync(UserConfigurationFileName);
                var configurationText = await FileIO.ReadTextAsync(configFile);
                
                var configuration = JsonSerializer.Deserialize<RiveFileConfiguration>(configurationText, GetJsonOptions());
                if (configuration == null)
                {
                    return null;
                }

                // Validate the configuration
                var validationResult = configuration.Validate();
                if (validationResult.HasErrors)
                {
                    System.Diagnostics.Debug.WriteLine($"User configuration validation failed: {validationResult}");
                    return null;
                }

                _currentConfiguration = configuration;
                OnConfigurationLoaded(configuration, UserConfigurationFileName);
                
                return configuration;
            }
            catch (FileNotFoundException)
            {
                // User configuration doesn't exist, which is normal
                return null;
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Failed to load user configuration: {ex.Message}");
                return null;
            }
        }

        /// <summary>
        /// Saves the current configuration as a user configuration
        /// </summary>
        public async Task<bool> SaveUserConfigurationAsync(RiveFileConfiguration configuration)
        {
            try
            {
                if (configuration == null)
                    throw new ArgumentNullException(nameof(configuration));

                // Validate before saving
                var validationResult = configuration.Validate();
                if (validationResult.HasErrors)
                {
                    System.Diagnostics.Debug.WriteLine($"Cannot save invalid configuration: {validationResult}");
                    return false;
                }

                var localFolder = ApplicationData.Current.LocalFolder;
                var configFile = await localFolder.CreateFileAsync(UserConfigurationFileName, CreationCollisionOption.ReplaceExisting);
                
                var configurationText = JsonSerializer.Serialize(configuration, GetJsonOptions());
                await FileIO.WriteTextAsync(configFile, configurationText);

                _currentConfiguration = configuration;
                OnConfigurationLoaded(configuration, UserConfigurationFileName);
                
                return true;
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Failed to save user configuration: {ex.Message}");
                return false;
            }
        }

        /// <summary>
        /// Gets all available files from the current configuration
        /// </summary>
        public List<RiveFileSource> GetAvailableFiles()
        {
            var files = new List<RiveFileSource>();

            if (_currentConfiguration?.AvailableFiles != null)
            {
                foreach (var configEntry in _currentConfiguration.AvailableFiles)
                {
                    if (configEntry.IsEnabled)
                    {
                        files.Add(configEntry.ToRiveFileSource());
                    }
                }
            }

            return files;
        }

        /// <summary>
        /// Gets the default file from the current configuration
        /// </summary>
        public RiveFileSource? GetDefaultFile()
        {
            return _currentConfiguration?.DefaultFile?.ToRiveFileSource();
        }

        /// <summary>
        /// Adds a file to the recently used list
        /// </summary>
        public void AddRecentlyUsedFile(RiveFileSource fileSource)
        {
            if (fileSource == null) return;

            // Remove if already exists to move it to the top
            _recentlyUsedFiles.RemoveAll(f => f.FilePath.Equals(fileSource.FilePath, StringComparison.OrdinalIgnoreCase));
            
            // Add to the beginning
            _recentlyUsedFiles.Insert(0, fileSource);
            
            // Trim to max size
            while (_recentlyUsedFiles.Count > MaxRecentFiles)
            {
                _recentlyUsedFiles.RemoveAt(_recentlyUsedFiles.Count - 1);
            }

            OnFileUsed(fileSource);
        }

        /// <summary>
        /// Clears the recently used files list
        /// </summary>
        public void ClearRecentlyUsedFiles()
        {
            _recentlyUsedFiles.Clear();
        }

        /// <summary>
        /// Adds a new file entry to the current configuration
        /// </summary>
        public void AddFileToConfiguration(RiveFileConfigurationEntry entry)
        {
            if (_currentConfiguration?.AvailableFiles != null && entry != null)
            {
                _currentConfiguration.AvailableFiles.Add(entry);
            }
        }

        /// <summary>
        /// Removes a file entry from the current configuration
        /// </summary>
        public bool RemoveFileFromConfiguration(string filePath)
        {
            if (_currentConfiguration?.AvailableFiles != null)
            {
                return _currentConfiguration.AvailableFiles.RemoveAll(
                    f => f.FilePath.Equals(filePath, StringComparison.OrdinalIgnoreCase)) > 0;
            }
            return false;
        }

        private static JsonSerializerOptions GetJsonOptions()
        {
            return new JsonSerializerOptions
            {
                PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
                WriteIndented = true,
                PropertyNameCaseInsensitive = true
            };
        }

        private static RiveFileConfiguration CreateFallbackConfiguration()
        {
            return new RiveFileConfiguration
            {
                Version = "1.0",
                Description = "Fallback configuration when default configuration cannot be loaded",
                AvailableFiles = new List<RiveFileConfigurationEntry>
                {
                    new()
                    {
                        DisplayName = "Data Test (Fallback)",
                        //FilePath = "Assets\\RiveAssets\\datatest.riv",
                        FilePath = "datatest.riv",
                        SourceType = RiveFileSourceType.Package,
                        IsDefault = true,
                        IsEnabled = true,
                        Description = "Fallback test animation",
                        Tags = new List<string> { "fallback", "demo" }
                    }
                }
            };
        }

        private void OnConfigurationLoaded(RiveFileConfiguration configuration, string source)
        {
            ConfigurationLoaded?.Invoke(this, new ConfigurationLoadedEventArgs(configuration, source));
        }

        private void OnFileUsed(RiveFileSource fileSource)
        {
            FileUsed?.Invoke(this, new FileUsedEventArgs(fileSource));
        }
    }

    /// <summary>
    /// Event arguments for configuration loaded events
    /// </summary>
    public class ConfigurationLoadedEventArgs : EventArgs
    {
        public RiveFileConfiguration Configuration { get; }
        public string Source { get; }

        public ConfigurationLoadedEventArgs(RiveFileConfiguration configuration, string source)
        {
            Configuration = configuration ?? throw new ArgumentNullException(nameof(configuration));
            Source = source ?? throw new ArgumentNullException(nameof(source));
        }
    }

    /// <summary>
    /// Event arguments for file used events
    /// </summary>
    public class FileUsedEventArgs : EventArgs
    {
        public RiveFileSource FileSource { get; }

        public FileUsedEventArgs(RiveFileSource fileSource)
        {
            FileSource = fileSource ?? throw new ArgumentNullException(nameof(fileSource));
        }
    }
}
