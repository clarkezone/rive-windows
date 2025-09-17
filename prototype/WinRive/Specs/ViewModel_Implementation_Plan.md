# ViewModel and ViewModelInstance Support Implementation Plan

## Overview

This document outlines the implementation plan for adding ViewModel and ViewModelInstance support to the Rive Windows application. The implementation will create a new `RiveViewModelPanel` control that allows users to select ViewModels, create instances, and manipulate properties through a dynamic UI.

All work should take place in and only in this location: C:\Users\jeclarke\src\github.com\clarkezone\rive-windows\prototype\WinRive\CSXamlHost

All files refered to below are located in that directory or a descendant.

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

### Phase 1: TabView Integration & Basic Structure ✅ **COMPLETED**
**Goal**: Create TabView layout and basic ViewModel panel structure

#### Tasks
- [x] **1.1** Modify `MainPage.xaml` to use TabView instead of single panel
  - [x] Replace `RiveStateMachinePanel` with `TabView` control
  - [x] Create "State Machines" tab containing existing `RiveStateMachinePanel`
  - [x] Create "View Models" tab for new `RiveViewModelPanel`
  - [x] Ensure both panels reference the same `RiveViewer`

- [x] **1.2** Create `RiveViewModelPanel.xaml`
  - [x] Grid layout with sections for ViewModel selection, instance management, properties, status
  - [x] ComboBox for ViewModel selection
  - [x] Buttons for instance management (Create, Bind)
  - [x] ScrollViewer with StackPanel for dynamic properties
  - [x] Status TextBlocks for messages and errors
  - [x] Follow same styling as `RiveStateMachinePanel`

- [x] **1.3** Create `RiveViewModelPanel.xaml.cs` basic structure
  - [x] Copy dependency property pattern from `RiveStateMachinePanel`
  - [x] Add `RiveViewer`, `ShowStatus`, `AllowExternalFiles` properties
  - [x] Implement `INotifyPropertyChanged`
  - [x] Add event handlers for `Loaded` and `Unloaded`
  - [x] Add basic property change notifications

**Deliverables**: ✅ **COMPLETED**
- ✅ Updated `MainPage.xaml` with Pivot control (UWP-compatible alternative to TabView)
- ✅ Basic `RiveViewModelPanel.xaml` layout with complete UI structure
- ✅ Skeleton `RiveViewModelPanel.xaml.cs` with dependency properties and full interface
- ✅ **Project compiles successfully - verified working**

### Phase 2: ViewModel Discovery & Selection ✅ **COMPLETED**
**Goal**: Implement ViewModel enumeration and selection functionality

#### Tasks
- [x] **2.1** Implement ViewModel discovery
  - [x] Add `ObservableCollection<ViewModel> ViewModels` property
  - [x] Connect to `RiveViewer.FileLoaded` event
  - [x] Call `riveControl.GetViewModels()` on file load
  - [x] Populate ViewModels collection
  - [x] Handle default ViewModel selection with fallback logic

- [x] **2.2** Add ViewModel selection handling
  - [x] Implement `SelectedViewModel` property with proper binding
  - [x] Add ComboBox selection changed handler
  - [x] Update UI state based on selection
  - [x] Display ViewModel information (name, property count)

- [x] **2.3** Add error handling and status updates
  - [x] Status messages for ViewModel operations
  - [x] Error handling for invalid ViewModels with graceful degradation
  - [x] Clear ViewModels on file load errors

**Deliverables**: ✅ **COMPLETED**
- ✅ Working ViewModel discovery and selection with robust error handling
- ✅ Comprehensive status messages for user feedback
- ✅ Graceful error handling for edge cases and API failures
- ✅ **Project compiles successfully - verified working**

### Phase 3: ViewModelInstance Management ✅ **COMPLETED**
**Goal**: Implement single ViewModelInstance creation and binding

#### Tasks
- [x] **3.1** Add ViewModelInstance properties
  - [x] Add `CurrentViewModelInstance` property with proper lifecycle
  - [x] Add `_isInstanceBound` boolean tracking with UI state updates
  - [x] Add comprehensive status properties for instance state tracking

- [x] **3.2** Implement instance creation
  - [x] "Create Instance" button handler with error handling
  - [x] Call `riveControl.CreateViewModelInstanceByName()` with selected ViewModel
  - [x] Update `CurrentViewModelInstance` property with property change notifications
  - [x] Update UI state (enable/disable controls) via CanCreateInstance, CanBindInstance

- [x] **3.3** Implement instance binding
  - [x] "Bind Instance" button handler with comprehensive error handling
  - [x] Call `riveControl.BindViewModelInstance()` with success validation
  - [x] Subscribe to `RiveControl.ViewModelInstanceBound` event for external binding notifications
  - [x] Update `_isInstanceBound` property with UI refresh triggers
  - [x] Clear instance on new file load and ViewModel selection changes

- [x] **3.4** Add instance lifecycle management
  - [x] Clear instance when ViewModel selection changes with `ClearCurrentInstance()`
  - [x] Handle instance disposal properly in event handlers and cleanup
  - [x] Update UI controls based on instance state via HasProperties, HasInstance properties
  - [x] Subscribe/unsubscribe from RiveControl events properly in viewer changes

**Deliverables**: ✅ **COMPLETED**
- ✅ Working ViewModelInstance creation and binding with robust error handling
- ✅ Complete instance lifecycle management with proper cleanup
- ✅ Comprehensive UI state management for instance operations
- ✅ **Project compiles successfully - verified working**

### Phase 4: Dynamic Property Controls Generation ✅ **COMPLETED**
**Goal**: Generate type-appropriate controls for ViewModelInstance properties

#### Tasks
- [x] **4.1** Implement property enumeration framework
  - [x] Call `viewModelInstance.GetProperties()` when instance bound with robust error handling
  - [x] Handle property type detection via ViewModelPropertyType enum with fallback support
  - [x] Complete property access infrastructure with comprehensive debugging
  - [x] Property enumeration framework ready for testing

- [x] **4.2** Create dynamic control generation system
  - [x] `GeneratePropertyControls()` method with complete UI generation framework
  - [x] `CreatePropertyControl(ViewModelInstanceProperty)` method with type-specific creation
  - [x] Clear and rebuild controls when instance changes via `ClearPropertyControls()`
  - [x] Complete control generation system with error handling and status updates

- [x] **4.3** Implement type-specific controls with Update buttons
  - [x] **String**: TextBox with Update button for explicit property updates
  - [x] **Number**: Slider with value label and Update button for controlled changes
  - [x] **Boolean**: ToggleSwitch with immediate updates (no button needed)
  - [x] **Color**: TextBox (hex format) with Update button for color validation
  - [x] **Enum**: ComboBox with Update button for selection confirmation
  - [x] **Trigger**: Button with immediate fire action (no additional update needed)
  - [x] All control types implemented with proper layout and user experience

- [x] **4.4** Wire property value changes with Update button pattern
  - [x] Connect Update button events to property setters for explicit control
  - [x] Call appropriate `SetViewModelXXXProperty()` methods on button clicks
  - [x] Remove real-time updates in favor of explicit Update button pattern
  - [x] Complete property setting infrastructure ready for testing

**Deliverables**: ✅ **COMPLETED**
- ✅ Complete dynamic property control generation system with Update buttons
- ✅ All property types implemented with appropriate UI controls
- ✅ Explicit Update button pattern for controlled property updates
- ✅ **Project compiles successfully - verified working**

### Phase 5: Real-time Updates & Event Handling  
**Goal**: Handle external property changes and provide live updates

#### Tasks
- [ ] **5.1** Subscribe to property change events
  - [x] Connect to `RiveControl.ViewModelPropertyChanged` event (infrastructure exists)
  - [ ] Verify external property changes are detected
  - [x] Handle event unsubscription properly (implemented)

- [ ] **5.2** Implement bidirectional property binding
  - [ ] Verify control values update from ViewModelInstance
  - [x] Prevent infinite loops during updates (infrastructure exists)
  - [ ] Handle property change conflicts

- [ ] **5.3** Add property refresh capabilities  
  - [x] Automatic refresh on ViewModelInstanceBound events (implemented)
  - [x] Automatic refresh on significant events (implemented)
  - [x] Handle property cache invalidation (implemented)

**Deliverables**:
- Real-time property updates (needs verification)
- Bidirectional data binding (needs testing) 
- Event-driven UI refresh (infrastructure exists)

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
