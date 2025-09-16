using System;

namespace CSXamlHost.Models
{
    /// <summary>
    /// Represents a source for a Rive file, including its location and metadata
    /// </summary>
    public class RiveFileSource
    {
        public string FilePath { get; set; } = string.Empty;
        public string DisplayName { get; set; } = string.Empty;
        public RiveFileSourceType SourceType { get; set; } = RiveFileSourceType.Package;
        public bool IsValid { get; private set; } = true;

        /// <summary>
        /// Default constructor
        /// </summary>
        public RiveFileSource()
        {
        }

        /// <summary>
        /// Constructor for creating a RiveFileSource
        /// </summary>
        /// <param name="filePath">Path to the Rive file</param>
        /// <param name="displayName">User-friendly display name</param>
        /// <param name="sourceType">Type of file source</param>
        public RiveFileSource(string filePath, string displayName, RiveFileSourceType sourceType = RiveFileSourceType.Package)
        {
            FilePath = filePath ?? throw new ArgumentNullException(nameof(filePath));
            DisplayName = displayName ?? throw new ArgumentNullException(nameof(displayName));
            SourceType = sourceType;
            ValidateFile();
        }

        /// <summary>
        /// Validates the file source and updates IsValid property
        /// </summary>
        public void ValidateFile()
        {
            try
            {
                // Basic validation - check if file path is not empty and has .riv extension
                IsValid = !string.IsNullOrWhiteSpace(FilePath) && 
                         FilePath.EndsWith(".riv", StringComparison.OrdinalIgnoreCase);

                // For package files, we assume they're valid if they follow naming convention
                // For external files, we could add file existence checks here if needed
            }
            catch
            {
                IsValid = false;
            }
        }

        /// <summary>
        /// Gets a user-friendly description of the file source
        /// </summary>
        public string GetDescription()
        {
            var typeDescription = SourceType switch
            {
                RiveFileSourceType.Package => "Package Asset",
                RiveFileSourceType.External => "External File",
                RiveFileSourceType.Embedded => "Embedded Resource",
                _ => "Unknown"
            };

            return $"{DisplayName} ({typeDescription})";
        }

        public override string ToString()
        {
            return DisplayName;
        }

        public override bool Equals(object? obj)
        {
            if (obj is RiveFileSource other)
            {
                return FilePath.Equals(other.FilePath, StringComparison.OrdinalIgnoreCase) &&
                       SourceType == other.SourceType;
            }
            return false;
        }

        public override int GetHashCode()
        {
            return HashCode.Combine(
                FilePath.ToLowerInvariant(),
                SourceType
            );
        }
    }

    /// <summary>
    /// Defines the source type for Rive files
    /// </summary>
    public enum RiveFileSourceType
    {
        /// <summary>
        /// File is included in the application package
        /// </summary>
        Package,

        /// <summary>
        /// File is loaded from external location via file picker
        /// </summary>
        External,

        /// <summary>
        /// File is embedded as a resource in the application
        /// </summary>
        Embedded
    }
}
