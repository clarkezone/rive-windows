using System;
using System.Collections.Generic;
using System.Linq;
using CSXamlHost.Models;

namespace CSXamlHost.Services
{
    /// <summary>
    /// Simplified service for managing Rive file configurations with hard-coded file list
    /// </summary>
    public class RiveFileConfigurationService
    {
        private static readonly List<RiveFileSource> _availableFiles = new()
        {
            new RiveFileSource
            {
                DisplayName = "Interactive Bento Grid",
                FilePath = "interactive_bento_grid.riv",
                SourceType = RiveFileSourceType.Package
            },
            new RiveFileSource
            {
                DisplayName = "Dress up",
                FilePath = "dress_up.riv",
                SourceType = RiveFileSourceType.Package
            },
            new RiveFileSource
            {
                DisplayName = "Layout Test",
                FilePath = "layouttest.riv",
                SourceType = RiveFileSourceType.Package
            },
            new RiveFileSource
            {
                DisplayName = "Meeting UI",
                FilePath = "meeting_ui.riv",
                SourceType = RiveFileSourceType.Package
            },
            new RiveFileSource
            {
                DisplayName = "Morphon Icons Demo",
                FilePath = "morphon_icons_(demo).riv",
                SourceType = RiveFileSourceType.Package
            },
            new RiveFileSource
            {
                DisplayName = "Pointer Events",
                FilePath = "pointer_events.riv",
                SourceType = RiveFileSourceType.Package
            },
            new RiveFileSource
            {
                DisplayName = "Vector Feathering Avatar",
                FilePath = "vector_feathering__avatar.riv",
                SourceType = RiveFileSourceType.Package
            },
            new RiveFileSource
            {
                DisplayName = "World Creator",
                FilePath = "world_creator.riv",
                SourceType = RiveFileSourceType.Package
            },
            new RiveFileSource
            {
                DisplayName = "The Sound of Coqui",
                FilePath = "the_sound_of_coqui.riv",
                SourceType = RiveFileSourceType.Package
            },
            new RiveFileSource
            {
                DisplayName = "Data Test",
                FilePath = "datatest.riv",
                SourceType = RiveFileSourceType.Package
            },
            new RiveFileSource
            {
                DisplayName = "Menus",
                FilePath = "menus.riv",
                SourceType = RiveFileSourceType.Package
            },
            new RiveFileSource
            {
                DisplayName = "Hunter X",
                FilePath = "hunter_x_demo.riv",
                SourceType = RiveFileSourceType.Package
            }

        };

        private readonly List<RiveFileSource> _recentlyUsedFiles = new();
        private readonly RiveFileConfiguration _currentConfiguration;

        public RiveFileConfigurationService()
        {
            // Initialize the configuration with our hard-coded files
            _currentConfiguration = new RiveFileConfiguration
            {
                Version = "1.0",
                Description = "Hard-coded configuration",
                AvailableFiles = _availableFiles.Select(f => new RiveFileConfigurationEntry
                {
                    DisplayName = f.DisplayName,
                    FilePath = f.FilePath,
                    SourceType = f.SourceType,
                    IsDefault = f == _availableFiles.FirstOrDefault(),
                    IsEnabled = true
                }).ToList()
            };
        }

        /// <summary>
        /// Gets the current configuration (always returns a valid configuration with our hard-coded files)
        /// </summary>
        public RiveFileConfiguration? CurrentConfiguration => _currentConfiguration;

        /// <summary>
        /// Gets the list of recently used files
        /// </summary>
        public IReadOnlyList<RiveFileSource> RecentlyUsedFiles => _recentlyUsedFiles.AsReadOnly();

        /// <summary>
        /// Gets all available Rive files
        /// </summary>
        /// <returns>List of available Rive file sources</returns>
        public List<RiveFileSource> GetAvailableFiles()
        {
            // Return a copy to prevent external modification
            return _availableFiles.ToList();
        }

        /// <summary>
        /// Gets the default Rive file (first in the list)
        /// </summary>
        /// <returns>The default RiveFileSource, or null if no files available</returns>
        public RiveFileSource? GetDefaultFile()
        {
            return _availableFiles.FirstOrDefault();
        }

        /// <summary>
        /// Adds a file to the recently used list (simplified implementation)
        /// </summary>
        /// <param name="fileSource">The file source to add</param>
        public void AddRecentlyUsedFile(RiveFileSource fileSource)
        {
            if (fileSource == null) return;

            // Remove if already exists to move it to the top
            _recentlyUsedFiles.RemoveAll(f => f.FilePath.Equals(fileSource.FilePath, StringComparison.OrdinalIgnoreCase));
            
            // Add to the beginning
            _recentlyUsedFiles.Insert(0, fileSource);
            
            // Keep only the last 10 files
            while (_recentlyUsedFiles.Count > 10)
            {
                _recentlyUsedFiles.RemoveAt(_recentlyUsedFiles.Count - 1);
            }
        }
    }
}
