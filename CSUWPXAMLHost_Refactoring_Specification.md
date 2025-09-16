# CSUWPXAMLHost Rive Refactoring Specification

## Mission Statement
Transform the CSUWPXAMLHost project from a minimal UWP host into a well-architected application with reusable Rive controls that separate rendering concerns from UI manipulation concerns.
The project directory is here:
C:\Users\jeclarke\src\github.com\clarkezone\rive-windows\prototype\WinRive\CSXamlHost

The souce files are here:
C:\Users\jeclarke\src\github.com\clarkezone\rive-windows\prototype\WinRive\CSXamlHost\MainPage.xaml
C:\Users\jeclarke\src\github.com\clarkezone\rive-windows\prototype\WinRive\CSXamlHost\MainPage.xaml.cs

DO NOT LOOK at files outside of the project directory.

## Problem Statement
Currently, the Rive functionality is either:
2. **Tightly coupled** (CSUWPXAMLHost has all logic embedded in MainWindow and all code in a codebehind)
3. **Non-reusable** (Cannot easily use Rive functionality in other projects)
4. **Inflexible** (Hardcoded file lists, no external file support)

## Success Definition
### Primary Objectives (MUST ACHIEVE)
1. **Separation of Concerns**: Create two distinct, reusable controls:
   - `RiveViewerControl`: Handles only Rive rendering and playback
   - `RiveStateMachinePanel`: Handles only UI for state machine manipulation
2. **Configurability**: Replace hardcoded Rive file lists with configurable inputs
3. **External File Support**: Add UWP file picker to load .riv files from outside the package in addition to the ones shipped in the app
4. **Reusability**: Controls can be used independently in other UWP projects

## you MUST ensure that the project compiles at the end of each step

### Secondary Objectives (SHOULD ACHIEVE)
- Clean, maintainable code architecture
- Comprehensive error handling
- Responsive UI design
- Performance optimization
- Documentation and examples

### Success Indicators
An LLM implementing this specification has succeeded when:

#### ✅ **Architectural Success**
- [ ] `RiveViewerControl` can render Rive animations without any UI controls
- [ ] `RiveStateMachinePanel` can manipulate any `RiveViewerControl` instance
- [ ] Controls are loosely coupled (panel works with viewer via properties/events)
- [ ] No business logic is embedded in XAML files

#### ✅ **Functional Success** 
- [ ] All CSWinUI3XAMLHost functionality works in new CSUWPXAMLHost implementation
- [ ] File picker successfully loads external .riv files
- [ ] State machine inputs dynamically generate appropriate UI controls
- [ ] Playback controls (play/pause/reset) function correctly
- [ ] Pointer input forwarding works for interactive Rive content

#### ✅ **Quality Success**
- [ ] No crashes or unhandled exceptions during normal operation
- [ ] Graceful error handling for invalid files or missing assets
- [ ] Responsive layout adapts to different window sizes
- [ ] Memory cleanup occurs properly when controls are destroyed

#### ✅ **Integration Success**
- [ ] Project compiles successfully with all new files
- [ ] All dependencies (WinRive C++ component) work correctly
- [ ] UWP packaging and deployment work without issues
- [ ] File picker respects UWP security and capability requirements

## Current State Analysis
### Existing Components
- **WinRive C++ Component** (`prototype/WinRive/WinRive/`):
  - `RiveControl.h/.cpp`: C++/WinRT wrapper around RiveRenderer
  - `RiveRenderer.h/.cpp`: Core rendering engine with DirectX integration
  - Complete API for: file loading, state machines, input handling, ViewModels
  - **Status**: ✅ Complete and functional - DO NOT MODIFY

- **CSWinUI3XAMLHost** (`prototype/WinRive/CSWinUI3XAMLHost/`):
  - `MainWindow.xaml/.cs`: Working WinUI3 demo with comprehensive UI implementation
  - Features: File selection, state machine controls, input manipulation, status display
  - **Status**: ✅ Reference implementation - USE AS TEMPLATE FOR UI LOGIC

- **CSXamlHost** (`prototype/WinRive/CSXamlHost/`):
  - `App.xaml`: Basic UWP application definition
  - `Assets/RiveAssets/`: Contains sample Rive files (datatest.riv, etc.)
  - `CSUWPXAMLHost.csproj`: Valid project file with correct references
  - `Package.appxmanifest`: UWP manifest with identity and capabilities
  - **Status**: ⚠️ Has project structure but MISSING all UI implementation files

### Key Architectural Insights from Analysis
1. **WinRive.RiveControl** provides complete C++ API - wrapper needed, not re-implementation
2. **Composition Integration**: Uses `Windows.UI.Composition` for rendering pipeline
3. **Input Forwarding**: Requires XAML pointer events → C++ RiveControl coordination
4. **State Machine Dynamics**: UI controls must be generated dynamically based on Rive file content
5. **File Loading Patterns**: Support both package assets and external file picker

## Target Architecture
### Control Hierarchy
```
CSUWPXAMLHost Application
├── MainWindow (XAML Window)
│   ├── RiveViewerControl (Custom Control)
│   │   └── WinRive.RiveControl (C++ Component)
│   └── RiveStateMachinePanel (UserControl)
│       └── Dynamic Input Controls (Generated)
```

### Data Flow
```
User Action → RiveStateMachinePanel → RiveViewerControl → WinRive.RiveControl → RiveRenderer
File Picker → RiveFileService → RiveViewerControl → State Change Events → RiveStateMachinePanel UI Update
```

### Responsibility Matrix
| Component | Rendering | UI Generation | File Management | State Machine Control | Input Handling |
|-----------|-----------|---------------|-----------------|----------------------|----------------|
| **RiveViewerControl** | ✅ Primary | ❌ No | ✅ Loading Only | ✅ Execution Only | ✅ Forward Only |
| **RiveStateMachinePanel** | ❌ No | ✅ Primary | ❌ No | ✅ UI Controls Only | ✅ Generate Controls |
| **WinRive.RiveControl** | ✅ Delegate | ❌ No | ❌ No | ✅ Implementation | ✅ Process |
| **Services** | ❌ No | ❌ No | ✅ Primary | ❌ No | ❌ No |

### Component Interfaces (Contract Definitions)
#### RiveViewerControl Public API
```csharp
// Properties (Dependency Properties)
string RiveFileSource { get; set; }           // File path or package URI
bool AutoPlay { get; set; }                   // Start playback automatically
string SelectedStateMachine { get; set; }     // State machine name to activate

// Methods
Task<bool> LoadFileAsync(string path);        // Load Rive file with validation
void StartRendering();                        // Begin render loop
void StopRendering();                         // Stop render loop
void SetSize(double width, double height);    // Update render size

// Events
event EventHandler<RiveFileLoadedEventArgs> FileLoaded;
event EventHandler<RiveErrorEventArgs> FileLoadError;
event EventHandler<StateMachineChangedEventArgs> StateMachineChanged;
```

#### RiveStateMachinePanel Public API
```csharp
// Properties (Dependency Properties)  
RiveViewerControl RiveViewer { get; set; }                          // Target control
ObservableCollection<RiveFileSource> AvailableFiles { get; set; }   // File list
bool AllowExternalFiles { get; set; }                               // Enable picker

// Events
event EventHandler<FileSelectedEventArgs> FileSelected;
event EventHandler<StateMachineSelectedEventArgs> StateMachineSelected;
event EventHandler<StatusChangedEventArgs> StatusChanged;
```

## Implementation Constraints
### Technical Requirements
- **Platform**: UWP (Universal Windows Platform) targeting Windows 10/11
- **Language**: C# for new controls, existing C++ component unchanged
- **UI Framework**: Microsoft.UI.Xaml (UWP XAML)
- **Dependencies**: WinRive C++ component, Microsoft.Windows.CsWinRT
- **File System**: Must respect UWP app container security model

### Design Patterns to Follow
- **MVVM-Friendly**: Controls should support data binding
- **Dependency Injection Ready**: Services should be interface-based
- **Event-Driven**: Loose coupling via events, not direct references
- **Async/Await**: File operations must be asynchronous
- **Composition over Inheritance**: Favor composition for control relationships

### Non-Goals (DO NOT IMPLEMENT)
- ❌ Modifications to WinRive C++ component
- ❌ Custom Rive file format parsing
- ❌ DirectX rendering implementation
- ❌ Cross-platform support beyond UWP
- ❌ Rive content creation or editing tools

---

## Implementation Checklist

### Phase 1: Project Setup and Structure
- [ ] **1.1** Create folder structure in CSUWPXAMLHost:
  - [ ] Create `Controls/` folder
  - [ ] Create `Models/` folder  
  - [ ] Create `Services/` folder
  - [ ] Create `Themes/` folder

- [ ] **1.2** Add required NuGet packages to CSUWPXAMLHost.csproj:
  - [ ] Verify Microsoft.Windows.CsWinRT reference exists
  - [ ] Verify WinRive project reference exists
  - [ ] Add any additional UWP-specific packages if needed

### Phase 2: Create RiveViewerControl (Custom Control)
- [ ] **2.1** Create RiveViewerControl.cs:
  - [ ] Inherit from Microsoft.UI.Xaml.Controls.Control
  - [ ] Add dependency properties:
    - [ ] `RiveFileSource` (string) - Path to Rive file
    - [ ] `AutoPlay` (bool) - Auto-start playback
    - [ ] `SelectedStateMachine` (string) - State machine name
    - [ ] `Width` and `Height` override handling
  - [ ] Add routed events:
    - [ ] `FileLoaded` - Fired when Rive file loads successfully
    - [ ] `FileLoadError` - Fired when file loading fails
    - [ ] `StateMachineChanged` - Fired when active state machine changes
    - [ ] `RenderingStarted` - Fired when rendering begins
    - [ ] `RenderingStopped` - Fired when rendering stops

- [ ] **2.2** Implement RiveViewerControl functionality:
  - [ ] Add private WinRive.RiveControl instance
  - [ ] Implement OnApplyTemplate() override
  - [ ] Add Initialize() method for composition setup
  - [ ] Add LoadRiveFile() method with error handling
  - [ ] Add SetSize() method for resize handling
  - [ ] Add StartRendering()/StopRendering() methods
  - [ ] Add Cleanup() method for proper disposal
  - [ ] Implement input forwarding (pointer events)
  - [ ] Add property change handlers for dependency properties

- [ ] **2.3** Create control template in Themes/Generic.xaml:
  - [ ] Add default template for RiveViewerControl
  - [ ] Include placeholder Border for composition visual
  - [ ] Add visual states for Loading, Error, Ready states
  - [ ] Style template for UWP visual guidelines

- [ ] **2.4** Add control registration:
  - [ ] Register control in Generic.xaml resource dictionary
  - [ ] Add proper xmlns declarations
  - [ ] Test basic control instantiation

### Phase 3: Create Models and Data Structures
- [ ] **3.1** Create Models/RiveFileSource.cs:
  - [ ] `string FilePath` - Path to Rive file
  - [ ] `string DisplayName` - User-friendly name
  - [ ] `RiveFileSourceType SourceType` - Package/External/Embedded enum
  - [ ] `bool IsValid` - Validation status
  - [ ] Add validation methods

- [ ] **3.2** Create Models/StateMachineInputModel.cs:
  - [ ] Properties for input binding (Name, Type, Value)
  - [ ] INotifyPropertyChanged implementation
  - [ ] Type-specific value properties (BoolValue, NumberValue, etc.)
  - [ ] Validation and conversion methods

- [ ] **3.3** Create Models/StateMachineModel.cs:
  - [ ] Properties for state machine info (Name, Index, IsDefault)
  - [ ] Collection of input models
  - [ ] Playback status properties
  - [ ] Event handling for state changes

### Phase 4: Create Services Layer
- [ ] **4.1** Create Services/RiveFileService.cs:
  - [ ] Method to load Rive files from package assets
  - [ ] Method to validate Rive file format
  - [ ] Method to enumerate packaged Rive files
  - [ ] Method to load external Rive files
  - [ ] Error handling and logging
  - [ ] Async file operations support

- [ ] **4.2** Create Services/FilePickerService.cs:
  - [ ] Wrapper around Windows.Storage.Pickers.FileOpenPicker
  - [ ] UWP-specific file picker configuration
  - [ ] File type filtering for .riv files
  - [ ] Error handling for picker cancellation
  - [ ] Result validation and processing

### Phase 5: Create RiveStateMachinePanel (UserControl)
- [ ] **5.1** Create RiveStateMachinePanel.xaml:
  - [ ] Layout with sections for:
    - [ ] File selection (ComboBox + Load External button)
    - [ ] State machine selection (ComboBox)
    - [ ] State machine info display (TextBlock)
    - [ ] Playback controls (Play/Pause/Reset buttons)
    - [ ] Dynamic inputs panel (StackPanel)
    - [ ] Status display (TextBlock)
  - [ ] Responsive design for different window sizes
  - [ ] UWP styling and visual guidelines compliance

- [ ] **5.2** Create RiveStateMachinePanel.xaml.cs:
  - [ ] Add dependency properties:
    - [ ] `RiveViewer` (RiveViewerControl) - Target control
    - [ ] `AvailableFiles` (ObservableCollection<RiveFileSource>)
    - [ ] `AllowExternalFiles` (bool) - Enable file picker
  - [ ] Add routed events:
    - [ ] `FileSelected` - User selected different file
    - [ ] `StateMachineSelected` - User selected different state machine
    - [ ] `StatusChanged` - Status message updates

- [ ] **5.3** Implement RiveStateMachinePanel functionality:
  - [ ] Wire up file selection ComboBox
  - [ ] Implement Load External File button handler
  - [ ] Create dynamic state machine selection
  - [ ] Implement playback control handlers
  - [ ] Add dynamic input control generation:
    - [ ] Boolean inputs -> ToggleSwitch controls
    - [ ] Number inputs -> NumberBox controls  
    - [ ] Trigger inputs -> Button controls
  - [ ] Add status message handling
  - [ ] Implement error display and recovery

### Phase 6: Integration with RiveViewerControl
- [ ] **6.1** Connect RiveStateMachinePanel to RiveViewerControl:
  - [ ] Handle RiveViewer property changes
  - [ ] Subscribe to RiveViewer events
  - [ ] Forward commands to RiveViewer
  - [ ] Sync state between controls

- [ ] **6.2** Implement bidirectional communication:
  - [ ] Panel updates when RiveViewer state changes
  - [ ] Panel commands trigger RiveViewer actions
  - [ ] Error handling across control boundaries
  - [ ] Event bubbling and routing

### Phase 7: Update CSUWPXAMLHost Application
- [ ] **7.1** Create/Update App.xaml.cs:
  - [ ] Basic UWP application setup
  - [ ] Error handling and crash reporting
  - [ ] Resource initialization

- [ ] **7.2** Create/Update MainWindow.xaml:
  - [ ] Replace hardcoded UI with new controls
  - [ ] Add RiveViewerControl instance
  - [ ] Add RiveStateMachinePanel instance
  - [ ] Connect panel to viewer control
  - [ ] Add sample file configuration
  - [ ] Implement responsive layout

- [ ] **7.3** Create/Update MainWindow.xaml.cs:
  - [ ] Remove old embedded logic
  - [ ] Add initialization for new controls
  - [ ] Wire up control events
  - [ ] Add error handling
  - [ ] Implement window lifecycle management

### Phase 8: File Configuration System
- [ ] **8.1** Create configurable file list:
  - [ ] Define JSON structure for file configuration
  - [ ] Create default configuration with existing files
  - [ ] Add configuration loading service
  - [ ] Support for package and external file sources

- [ ] **8.2** Implement file picker integration:
  - [ ] Add file picker service to RiveStateMachinePanel
  - [ ] Handle file validation and loading
  - [ ] Add recently used files tracking
  - [ ] Implement error handling for invalid files

- [ ] **8.3** Update Package.appxmanifest:
  - [ ] Add file association for .riv files (if desired)
  - [ ] Add capability declarations as needed
  - [ ] Update application identity and metadata

### Phase 9: Testing and Validation
- [ ] **9.1** Unit testing setup:
  - [ ] Create test project structure
  - [ ] Add test cases for core functionality
  - [ ] Mock WinRive.RiveControl for testing
  - [ ] Test file loading and validation

- [ ] **9.2** Integration testing:
  - [ ] Test control integration
  - [ ] Test file picker functionality
  - [ ] Test state machine manipulation
  - [ ] Test error handling scenarios

- [ ] **9.3** Manual testing checklist:
  - [ ] Load different Rive files from package
  - [ ] Load external Rive files via picker
  - [ ] Test all state machine controls
  - [ ] Test input manipulation (Boolean, Number, Trigger)
  - [ ] Test playback controls (Play/Pause/Reset)
  - [ ] Test window resizing and layout
  - [ ] Test error scenarios (invalid files, missing files)

### Phase 10: Documentation and Examples
- [ ] **10.1** Create usage documentation:
  - [ ] Document RiveViewerControl API
  - [ ] Document RiveStateMachinePanel API
  - [ ] Create integration examples
  - [ ] Document configuration options

- [ ] **10.2** Create sample implementations:
  - [ ] Simple viewer example
  - [ ] Complex dashboard example
  - [ ] Custom styling examples
  - [ ] Performance optimization tips

---

## File Structure After Implementation

```
CSUWPXAMLHost/
├── Controls/
│   ├── RiveViewerControl.cs              # Custom control for Rive rendering
│   ├── RiveStateMachinePanel.xaml        # UserControl for state machine UI
│   └── RiveStateMachinePanel.xaml.cs     # UserControl code-behind
├── Models/
│   ├── RiveFileSource.cs                 # File source configuration model
│   ├── StateMachineInputModel.cs         # State machine input binding model
│   └── StateMachineModel.cs              # State machine data model
├── Services/
│   ├── RiveFileService.cs                # File management and validation service
│   └── FilePickerService.cs              # UWP file picker wrapper service
├── Themes/
│   └── Generic.xaml                      # Control templates and styles
├── Assets/
│   └── RiveAssets/                       # Package Rive files
├── App.xaml                              # Application definition
├── App.xaml.cs                           # Application code-behind
├── MainWindow.xaml                       # Main application window
├── MainWindow.xaml.cs                    # Main window code-behind
├── Package.appxmanifest                  # UWP application manifest
└── CSUWPXAMLHost.csproj                  # Project file
```

## Success Criteria
- [ ] RiveViewerControl can be used standalone for simple Rive playback
- [ ] RiveStateMachinePanel can be used with any RiveViewerControl
- [ ] External file loading works via UWP file picker
- [ ] All existing functionality from CSWinUI3XAMLHost is preserved
- [ ] Controls are reusable across different UWP applications
- [ ] Proper error handling and user feedback
- [ ] Clean separation of concerns between rendering and UI
- [ ] Comprehensive documentation and examples

## Migration Notes
- Existing CSWinUI3XAMLHost code serves as reference implementation
- WinRive C++ component remains unchanged
- New architecture allows for future extensibility
- Controls can be packaged as separate NuGet package if desired
