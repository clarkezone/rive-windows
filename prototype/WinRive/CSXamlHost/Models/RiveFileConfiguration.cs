using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;

namespace CSXamlHost.Models
{
    /// <summary>
    /// Configuration model for managing available Rive files in the application
    /// </summary>
    public class RiveFileConfiguration : INotifyPropertyChanged
    {
        private string _version = "1.0";
        private string _description = string.Empty;
        private List<RiveFileConfigurationEntry> _availableFiles = new();

        public event PropertyChangedEventHandler? PropertyChanged;

        /// <summary>
        /// Configuration version for future compatibility
        /// </summary>
        public string Version
        {
            get => _version;
            set
            {
                if (_version != value)
                {
                    _version = value;
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Description of this file configuration
        /// </summary>
        public string Description
        {
            get => _description;
            set
            {
                if (_description != value)
                {
                    _description = value;
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// List of available Rive files
        /// </summary>
        public List<RiveFileConfigurationEntry> AvailableFiles
        {
            get => _availableFiles;
            set
            {
                if (_availableFiles != value)
                {
                    _availableFiles = value ?? new List<RiveFileConfigurationEntry>();
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Gets the default file entry if one exists
        /// </summary>
        public RiveFileConfigurationEntry? DefaultFile =>
            AvailableFiles.FirstOrDefault(f => f.IsDefault) ?? AvailableFiles.FirstOrDefault();

        /// <summary>
        /// Validates the configuration
        /// </summary>
        public ValidationResult Validate()
        {
            var result = new ValidationResult();

            if (string.IsNullOrEmpty(Version))
            {
                result.AddError("Version is required");
            }

            if (AvailableFiles == null || AvailableFiles.Count == 0)
            {
                result.AddWarning("No files configured");
            }
            else
            {
                var defaultCount = AvailableFiles.Count(f => f.IsDefault);
                if (defaultCount > 1)
                {
                    result.AddWarning($"Multiple default files found ({defaultCount}). Only the first will be used.");
                }

                for (int i = 0; i < AvailableFiles.Count; i++)
                {
                    var fileResult = AvailableFiles[i].Validate();
                    if (fileResult.HasErrors || fileResult.HasWarnings)
                    {
                        foreach (var error in fileResult.Errors)
                        {
                            result.AddError($"File {i}: {error}");
                        }
                        foreach (var warning in fileResult.Warnings)
                        {
                            result.AddWarning($"File {i}: {warning}");
                        }
                    }
                }
            }

            return result;
        }

        private void OnPropertyChanged([CallerMemberName] string? propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    /// <summary>
    /// Configuration entry for a single Rive file
    /// </summary>
    public class RiveFileConfigurationEntry : INotifyPropertyChanged
    {
        private string _displayName = string.Empty;
        private string _filePath = string.Empty;
        private RiveFileSourceType _sourceType = RiveFileSourceType.Package;
        private bool _isDefault;
        private bool _isEnabled = true;
        private string _description = string.Empty;
        private List<string> _tags = new();

        public event PropertyChangedEventHandler? PropertyChanged;

        /// <summary>
        /// Display name shown to users
        /// </summary>
        public string DisplayName
        {
            get => _displayName;
            set
            {
                if (_displayName != value)
                {
                    _displayName = value;
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Path to the Rive file (relative to package or absolute for external)
        /// </summary>
        public string FilePath
        {
            get => _filePath;
            set
            {
                if (_filePath != value)
                {
                    _filePath = value;
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Type of file source (Package, External, etc.)
        /// </summary>
        public RiveFileSourceType SourceType
        {
            get => _sourceType;
            set
            {
                if (_sourceType != value)
                {
                    _sourceType = value;
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Whether this is the default file to load
        /// </summary>
        public bool IsDefault
        {
            get => _isDefault;
            set
            {
                if (_isDefault != value)
                {
                    _isDefault = value;
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Whether this file is enabled for selection
        /// </summary>
        public bool IsEnabled
        {
            get => _isEnabled;
            set
            {
                if (_isEnabled != value)
                {
                    _isEnabled = value;
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Optional description for this file
        /// </summary>
        public string Description
        {
            get => _description;
            set
            {
                if (_description != value)
                {
                    _description = value;
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Optional tags for categorization
        /// </summary>
        public List<string> Tags
        {
            get => _tags;
            set
            {
                if (_tags != value)
                {
                    _tags = value ?? new List<string>();
                    OnPropertyChanged();
                }
            }
        }

        /// <summary>
        /// Converts this configuration entry to a RiveFileSource
        /// </summary>
        public RiveFileSource ToRiveFileSource()
        {
            var riveFileSource = new RiveFileSource
            {
                DisplayName = DisplayName,
                FilePath = FilePath,
                SourceType = SourceType
            };
            
            // Validate the file source
            riveFileSource.ValidateFile();
            
            return riveFileSource;
        }

        /// <summary>
        /// Validates this configuration entry
        /// </summary>
        public ValidationResult Validate()
        {
            var result = new ValidationResult();

            if (string.IsNullOrWhiteSpace(DisplayName))
            {
                result.AddError("DisplayName is required");
            }

            if (string.IsNullOrWhiteSpace(FilePath))
            {
                result.AddError("FilePath is required");
            }
            else
            {
                // Basic path validation
                if (SourceType == RiveFileSourceType.Package)
                {
                    if (FilePath.StartsWith("/") || FilePath.Contains(".."))
                    {
                        result.AddWarning("Package file path should be relative without leading slash");
                    }
                    if (!FilePath.EndsWith(".riv", StringComparison.OrdinalIgnoreCase))
                    {
                        result.AddWarning("File path should end with .riv extension");
                    }
                }
            }

            return result;
        }

        private void OnPropertyChanged([CallerMemberName] string? propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    /// <summary>
    /// Validation result for configuration objects
    /// </summary>
    public class ValidationResult
    {
        public List<string> Errors { get; } = new();
        public List<string> Warnings { get; } = new();

        public bool HasErrors => Errors.Count > 0;
        public bool HasWarnings => Warnings.Count > 0;
        public bool IsValid => !HasErrors;

        public void AddError(string error)
        {
            if (!string.IsNullOrWhiteSpace(error))
            {
                Errors.Add(error);
            }
        }

        public void AddWarning(string warning)
        {
            if (!string.IsNullOrWhiteSpace(warning))
            {
                Warnings.Add(warning);
            }
        }

        public override string ToString()
        {
            var messages = new List<string>();
            if (HasErrors)
            {
                messages.Add($"Errors: {string.Join(", ", Errors)}");
            }
            if (HasWarnings)
            {
                messages.Add($"Warnings: {string.Join(", ", Warnings)}");
            }
            return messages.Count > 0 ? string.Join("; ", messages) : "Valid";
        }
    }
}
