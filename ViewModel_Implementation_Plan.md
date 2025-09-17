# ViewModel and ViewModelInstance Support Implementation Plan

## Overview

This document outlines the implementation plan for adding ViewModel and ViewModelInstance support to the Rive Windows application. The implementation will create a new `RiveViewModelPanel` control that allows users to select ViewModels, create instances, and manipulate properties through a dynamic UI.

## Architecture Analysis

### Existing Infrastructure
- ✅ **RiveControl** already has complete ViewModel support methods
- ✅ **ViewModel class hierarchy** exists: `ViewModel`, `ViewModelInstance`, `ViewModelInstanceProperty`
- ✅ **RiveStateMachinePanel** provides proven integration pattern
- ✅ **MainPage layout** uses two-column design suitable for extension

### Key RiveControl Methods Available
```cpp
// ViewModel discovery
Windows::Foundation::Collections::IVectorView<ViewModelInfo> GetViewModels();
ViewModel GetViewModelByName(hstring const& name);
ViewModel GetDefaultViewModel();

// ViewModelInstance management  
ViewModelInstance CreateViewModelInstance();
ViewModelInstance CreateViewModelInstanceByName(hstring const& viewModelName);
bool BindViewModelInstance(ViewModelInstance const& instance);
ViewModelInstance GetBoundViewModelInstance();

// Property manipulation
bool SetViewModelStringProperty(hstring const& propertyName, hstring const& value);
bool SetViewModelNumberProperty(hstring const& propertyName, double value);
bool SetViewModelBooleanProperty(hstring const& propertyName, bool value);
bool SetViewModelColorProperty(hstring const& propertyName, uint32_t color);
bool SetViewModelEnumProperty(hstring const& propertyName, int32_t value);
bool FireViewModelTrigger(hstring const& triggerName);

// Events
event_token ViewModelInstanceBound(...);
event_token ViewModelPropertyChanged(...);
```

## Implementation Requirements

### User Requirements
1. **TabView Layout**: ViewModel panel in tab alongside StateMachine panel
2. **Single Instance Support**: One active ViewModelInstance at a time
3. **Property Controls**:
   - String → TextBox
   - Number → Slider (no ranges)
   - Boolean → ToggleSwitch
   - Color → ColorPicker
   - Enum → ComboBox
   - Trigger → Button

### Technical Requirements
- Follow `RiveStateMachinePanel` integration pattern exactly
- Connect to `RiveViewer.FileLoaded` event for refresh
- Use existing dependency property patterns
- Handle errors gracefully with status messages
- Support real-time property updates via events

## Implementation Plan

### Phase 1: TabView Integration & Basic Structure
**Goal**: Create TabView layout and basic ViewModel panel structure

#### Tasks
- [ ] **1.1** Modify `MainPage.xaml` to use TabView instead of single panel
  - [ ] Replace `RiveStateMachinePanel` with `TabView` control
  - [ ] Create "State Machines" tab containing existing `RiveStateMachinePanel`
  - [ ] Create "View Models" tab for new `RiveViewModelPanel`
  - [ ] Ensure both panels reference the same `RiveViewer`

- [ ] **1.2** Create `RiveViewModelPanel.xaml`
  - [ ] Grid layout with sections for ViewModel selection, instance management, properties, status
  - [ ] ComboBox for ViewModel selection
  - [ ] Buttons for instance management (Create, Bind)
  - [ ] ScrollViewer with StackPanel for dynamic properties
  - [ ] Status TextBlocks for messages and errors
  - [ ] Follow same styling as `RiveStateMachinePanel`

- [ ] **1.3** Create `RiveViewModelPanel.xaml.cs` basic structure
  - [ ] Copy dependency property pattern from `RiveStateMachinePanel`
  - [ ] Add `RiveViewer`, `ShowStatus`, `AllowExternalFiles` properties
  - [ ] Implement `INotifyPropertyChanged`
  - [ ] Add event handlers for `Loaded` and `Unloaded`
  - [ ] Add basic property change notifications

**Deliverables**:
- Updated `MainPage.xaml` with TabView
- Basic `RiveViewModelPanel.xaml` layout
- Skeleton `RiveViewModelPanel.xaml.cs` with dependency properties

### Phase 2: ViewModel Discovery & Selection
**Goal**: Implement ViewModel enumeration and selection functionality

#### Tasks
- [ ] **2.1** Implement ViewModel discovery
  - [ ] Add `ObservableCollection<ViewModel> ViewModels` property
  - [ ] Connect to `RiveViewer.FileLoaded` event
  - [ ] Call `riveControl.GetViewModels()` on file load
  - [ ] Populate ViewModels collection
  - [ ] Handle default ViewModel selection

- [ ] **2.2** Add ViewModel selection handling
  - [ ] Implement `SelectedViewModel` property
  - [ ] Add ComboBox selection changed handler
  - [ ] Update UI state based on selection
  - [ ] Display ViewModel information (name, property count)

- [ ] **2.3** Add error handling and status updates
  - [ ] Status messages for ViewModel operations
  - [ ] Error handling for invalid ViewModels
  - [ ] Clear ViewModels on file load errors

**Deliverables**:
- Working ViewModel discovery and selection
- Status messages for user feedback
- Error handling for edge cases

### Phase 3: ViewModelInstance Management
**Goal**: Implement single ViewModelInstance creation and binding

#### Tasks
- [ ] **3.1** Add ViewModelInstance properties
  - [ ] Add `CurrentViewModelInstance` property
  - [ ] Add `IsInstanceBound` boolean property
  - [ ] Add status properties for instance state

- [ ] **3.2** Implement instance creation
  - [ ] "Create Instance" button handler
  - [ ] Call `riveControl.CreateViewModelInstance()` or `CreateViewModelInstanceByName()`
  - [ ] Update `CurrentViewModelInstance` property
  - [ ] Update UI state (enable/disable controls)

- [ ] **3.3** Implement instance binding
  - [ ] "Bind Instance" button handler  
  - [ ] Call `riveControl.BindViewModelInstance()`
  - [ ] Subscribe to `RiveControl.ViewModelInstanceBound` event
  - [ ] Update `IsInstanceBound` property
  - [ ] Clear instance on new file load

- [ ] **3.4** Add instance lifecycle management
  - [ ] Clear instance when ViewModel selection changes
  - [ ] Handle instance disposal properly
  - [ ] Update UI controls based on instance state

**Deliverables**:
- Working ViewModelInstance creation and binding
- Proper instance lifecycle management
- UI state management for instance operations

### Phase 4: Dynamic Property Controls Generation
**Goal**: Generate type-appropriate controls for ViewModelInstance properties

#### Tasks
- [ ] **4.1** Implement property enumeration
  - [ ] Call `viewModelInstance.GetProperties()` when instance bound
  - [ ] Cache properties for performance
  - [ ] Handle property type detection

- [ ] **4.2** Create dynamic control generation system
  - [ ] `GeneratePropertyControls()` method similar to StateMachine panel
  - [ ] `CreatePropertyControl(ViewModelInstanceProperty)` method
  - [ ] Clear and rebuild controls when instance changes

- [ ] **4.3** Implement type-specific controls
  - [ ] **String**: TextBox with TextChanged event
  - [ ] **Number**: Slider with ValueChanged event
  - [ ] **Boolean**: ToggleSwitch with Toggled event
  - [ ] **Color**: ColorPicker with ColorChanged event
  - [ ] **Enum**: ComboBox with SelectionChanged event
  - [ ] **Trigger**: Button with Click event

- [ ] **4.4** Wire property value changes
  - [ ] Connect control events to property setters
  - [ ] Call appropriate `SetViewModelXXXProperty()` methods
  - [ ] Handle property validation and errors
  - [ ] Update status on successful changes

**Deliverables**:
- Dynamic property control generation
- Type-specific property editors
- Property value change handling

### Phase 5: Real-time Updates & Event Handling
**Goal**: Handle external property changes and provide live updates

#### Tasks
- [ ] **5.1** Subscribe to property change events
  - [ ] Connect to `RiveControl.ViewModelPropertyChanged` event
  - [ ] Update UI controls when properties change externally
  - [ ] Handle event unsubscription properly

- [ ] **5.2** Implement bidirectional property binding
  - [ ] Update control values from ViewModelInstance
  - [ ] Prevent infinite loops during updates
  - [ ] Handle property change conflicts

- [ ] **5.3** Add property refresh capabilities
  - [ ] Manual refresh button/method
  - [ ] Automatic refresh on significant events
  - [ ] Handle property cache invalidation

**Deliverables**:
- Real-time property updates
- Bidirectional data binding
- Event-driven UI refresh

### Phase 6: Error Handling & Polish
**Goal**: Add robust error handling and user experience improvements

#### Tasks
- [ ] **6.1** Comprehensive error handling
  - [ ] Try-catch blocks around all RiveControl operations
  - [ ] User-friendly error messages
  - [ ] Graceful degradation on failures
  - [ ] Log errors for debugging

- [ ] **6.2** UI state management
  - [ ] Enable/disable controls based on state
  - [ ] Show/hide sections based on availability
  - [ ] Loading indicators for async operations
  - [ ] Proper control focus management

- [ ] **6.3** User experience improvements
  - [ ] Tooltips for controls and properties
  - [ ] Context menus where appropriate
  - [ ] Keyboard shortcuts
  - [ ] Accessibility support

- [ ] **6.4** Performance optimization
  - [ ] Lazy loading of properties
  - [ ] Control virtualization for large property lists
  - [ ] Debounce rapid property changes
  - [ ] Memory leak prevention

**Deliverables**:
- Production-ready error handling
- Polished user interface
- Performance optimizations

### Phase 7: Integration Testing & Validation
**Goal**: Ensure complete integration and functionality

#### Tasks
- [ ] **7.1** Integration testing
  - [ ] Test with various Rive files
  - [ ] Test ViewModel/StateMachine panel switching
  - [ ] Test file loading scenarios
  - [ ] Test error conditions

- [ ] **7.2** Property type testing
  - [ ] Test all property types individually
  - [ ] Test property value ranges and limits
  - [ ] Test enum property options
  - [ ] Test trigger firing

- [ ] **7.3** Event handling validation
  - [ ] Test external property changes
  - [ ] Test event subscription/unsubscription
  - [ ] Test memory leaks during repeated operations
  - [ ] Test concurrent property modifications

- [ ] **7.4** User workflow testing
  - [ ] Test complete user workflows
  - [ ] Test edge cases and error scenarios
  - [ ] Test performance with large ViewModels
  - [ ] Validate UI responsiveness

**Deliverables**:
- Fully tested implementation
- Validated user workflows
- Performance benchmarks

## File Structure

### New Files to Create
```
prototype/WinRive/CSXamlHost/Controls/
├── RiveViewModelPanel.xaml
└── RiveViewModelPanel.xaml.cs

prototype/WinRive/CSXamlHost/Models/ (optional)
└── ViewModelModel.cs (if needed for additional abstraction)
```

### Files to Modify
```
prototype/WinRive/CSXamlHost/
├── MainPage.xaml (TabView integration)
└── MainPage.xaml.cs (TabView references)
```

## Technical Specifications

### RiveViewModelPanel Public Interface
```csharp
public sealed partial class RiveViewModelPanel : UserControl, INotifyPropertyChanged
{
    // Dependency Properties (matching RiveStateMachinePanel)
    public RiveViewerControl RiveViewer { get; set; }
    public bool AllowExternalFiles { get; set; }
    public bool ShowStatus { get; set; }
    
    // ViewModel Properties
    public ObservableCollection<ViewModel> ViewModels { get; }
    public ViewModel SelectedViewModel { get; set; }
    public ViewModelInstance CurrentViewModelInstance { get; set; }
    public bool IsInstanceBound { get; set; }
    
    // Status Properties
    public string StatusText { get; }
    public string ErrorMessage { get; }
    public bool HasError { get; }
    
    // Events
    public event EventHandler<ViewModelSelectedEventArgs> ViewModelSelected;
    public event EventHandler<InstanceBoundEventArgs> InstanceBound;
    public event EventHandler<StatusChangedEventArgs> StatusChanged;
}
```

### Key Integration Points
- **File Loading**: Connect to `RiveViewer.FileLoaded` event
- **ViewModel Discovery**: Use `RiveControl.GetViewModels()`
- **Instance Management**: Use `RiveControl.CreateViewModelInstance()` and `BindViewModelInstance()`
- **Property Updates**: Use `RiveControl.SetViewModelXXXProperty()` methods
- **Event Handling**: Subscribe to `RiveControl.ViewModelPropertyChanged`

## Success Criteria

### Functional Requirements
- [ ] User can select ViewModels from dropdown
- [ ] User can create and bind ViewModelInstances
- [ ] User can modify properties through appropriate controls
- [ ] Property changes are reflected in Rive animation
- [ ] External property changes update the UI
- [ ] Error conditions are handled gracefully

### Non-Functional Requirements
- [ ] UI follows existing design patterns
- [ ] Performance is comparable to StateMachine panel
- [ ] Memory usage is reasonable
- [ ] No memory leaks during repeated operations
- [ ] Accessible via keyboard and screen readers

## Risk Mitigation

### Technical Risks
- **Complex property types**: Start with basic types, add complex ones incrementally
- **Performance with many properties**: Implement virtualization if needed
- **Memory leaks**: Follow RAII patterns and proper event unsubscription
- **Threading issues**: Ensure all UI updates on main thread

### Integration Risks
- **Breaking existing functionality**: Preserve all existing StateMachine functionality
- **RiveControl API changes**: Use existing APIs, avoid modifications
- **XAML binding issues**: Test thoroughly with various data scenarios

## Implementation Notes

### Following RiveStateMachinePanel Pattern
The implementation should closely mirror the existing `RiveStateMachinePanel` to ensure consistency:

1. **Dependency Properties**: Same pattern for `RiveViewer`, `AllowExternalFiles`, `ShowStatus`
2. **Event Handling**: Same pattern for `FileLoaded`, `ErrorOccurred` events
3. **UI Generation**: Similar dynamic control generation approach
4. **Error Handling**: Same status/error message pattern
5. **Lifecycle Management**: Same loading/unloading event handlers

### Property Control Guidelines
- **String Properties**: Use TextBox with immediate binding for responsive updates
- **Number Properties**: Use Slider for intuitive manipulation, show current value
- **Boolean Properties**: Use ToggleSwitch for clear on/off state
- **Color Properties**: Use ColorPicker if available, fallback to hex TextBox
- **Enum Properties**: Use ComboBox populated with enum options/names
- **Trigger Properties**: Use Button with clear "Fire" action

This plan provides a comprehensive roadmap for implementing ViewModel and ViewModelInstance support while maintaining the existing architecture and user experience patterns.
