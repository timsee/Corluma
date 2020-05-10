# Change Log

### **v0.1** 
* Initial version

### **v0.1.1** 
#### Documentation Update
* Created custom doxygen pages for giving class overviews.
* Added more in-code commenting. 

### **v0.1.3** 
#### Code Cleaning Update
* Added in smart pointers, std::vectors, and QSignalMappers.
* Fixed bugs in RGB sliders.

### **v0.3.0** 
#### ColorPicker Update
* Added a Color Picker widget.
* Removed ColorWheel in favor of ColorPicker.
* Added more control on the ArrayColorsPage.
* Fixed bugs in OS X support.
* Fixed miscellaneous bugs.

### **v0.5.0** 
#### Color Presets Update
* Added 17 color presets. These can be used with multi color routines and are based around basic themes, such as *Fire* or *Snow*.
* Refactored the GUI's PresetArrayPage to support all of the new color presets.
* Fixed miscellaneous bugs.

### **v0.6.0** 
#### Memory Reduction Update
* Removed unnecessary variables.
* Documented all headers.
* Fixed miscellaneous bugs.

### **v0.7.0** 
#### API and Bug Fixes Update
* Refactored names of enumerated types and API calls to be more uniform.
* Polished menu bar.
* Added Application Icon.
* Fixed bugs in LightsSlider.
* Fixed state management bugs.
* Updated project documentation to reflect API changes. 

### **v0.8.0** 
#### Wireless, Android, and iOS Update
* Extended the CommLayer to support HTTP and UDP.
* Added support for Android and iOS builds.
* Updated layout to support the new screen sizes.
* Fixed bugs, probably created more. 

### **v0.8.5** 
#### Settings Page Update
* Added the option to switch to different connections on the settings screen.
* Cleaned up the settings sliders. 
* Added the ability to store previous connections between sessions.
* Fixed connection issues.
* Fixed layout issues on certain devices.

### **v0.9.0** 
#### Arduino Library and Linux Update
* Tested and added Linux support.
* Fixed layout issues.
* Fixed edge cases in Settings Page.
* Added support for 4 new single color routines. 
* Fixed warnings.

### **v0.9.2** 
#### Phillips Hue Update
* Added support for the Phillips Hue. 
* Added discovery methods for the Hue Bridge.
* Refactored code to support more types of connections. 
* Fixed bugs.

### **v0.9.4** 
#### New Repo Update
* Created a new repo for this project based on the GUI subproject of [RGB-LED-Routines](https://github.com/timsee/RGB-LED-Routines)
* Added two way communication between the GUI and all light controllers.
* Added discovery methods to all data streams.
* Synced the state of the GUI with the updates coming from its lights.
* Added a GUI implementation of RoutinesRGB.
* Fixed bugs on the settings page. 
* Fixed miscellaneous bugs.

___
## The Backend Updates
### **v0.9.5** 
#### Obligatory Confusing Backend Update
* Rewrote system that stores and switches between different connections.
### **v0.9.55** 
#### CommLayer Update
* Adjusted from having only one connection (UDP, HTTP, Phillips Hue, etc.) active at a time to having all connections active simultaneously.
* Added independent heartbeats and maintenance to each connection.
### **v0.9.6** 
#### Group Update
* Added the ability to select multiple connections and control them all simultaneously. This includes support for simultaneous control of lights of different types (e.g. an arduino over serial and a Phillips Hue). 
### **v0.9.65** 
#### CommType Update
* Simplified internal data structures for storing and accessing lights.
* Simplified APIs.
___

### **v0.9.7**
#### Connection Page Update
* Moved connecting to devices from Settings Page to the new Connection Page.
* When no devices are connected, assets that can't be used now grey themselves out.
* Rewrote system for turning on and off CommTypes.

### **v0.9.8**
#### Reliability Update
* Added reliability layer to all packets. 
* Added support for Hue Ambient Lights.
* Added GUI for saving and choosing preset groups of lights.

### **v0.9.82**
#### Miscellaneous Cleanup Update
* Fixed edge cases in DataSync.
* Cleaned up old code.
* Updated message parsing to match the new [RGB-LED-Routines](https://github.com/timsee/RGB-LED-Routines) protocols. 
* Fixed miscellaneous bugs.

### **v0.9.85**
#### Discovery Page Update
* Added page for discovering new connections.
* Added support for Hue Light Strips, Blooms, and White bulbs.
* Added initial Hue Schedule support.
* Changed the layout of pages to be more uniform and require less clicks to use.
* Fixed miscellaneous bugs.

### **v0.9.9**
#### UX Update
* Redesigned Connection Page so that it is easier to control multiple devices at once.
* Added an edit page for groups of devices.
* Sped up backend code for changing multiple devices at once. 
* Added the ability to save and load groups of devices as JSON data.
* Made minor tweaks to make actions require less clicks to execute.

### **v0.9.91**
#### Why-Do-I-Name-These Update
* Did a QA round, fixed a lot of edge cases and bugs.
* Updated Discovery Page, Connection Page, and Settings Page.
* Added a new version of the Hue Color Temperature Picker.
* Fixed many layout issues, broke some in certain environments (woops).

### **v0.9.92**
#### Why-Do-I-Name-These Update Part 2: The Great Refactor
* Removed [QDarkStyleSheet](https://github.com/ColinDuquesnoy/QDarkStyleSheet) in favor of a project specific stylesheet.
* Refactored ColorPicker to do more and pick colors better. 
* Refactored FloatingLayout. 
* Started to add transitions.
* Removed ColorPickerAmbient and CustomColorsPage.
* Fixed miscellaneous bugs.

### **v0.9.96**
#### The Great Refactor Part 2: The CommLayer, Subwidget, and Continued Bad Naming Convention Update
* Refactored complicated widgets into smaller subwidgets.
* Improved iOS support.
* Simplified commuincation with Arduino projects 
* Added an optional CRC for Arduino projects.
* Added directories to source code. 
* Fixed miscellaneous issues.

### **v0.9.98**
#### The Minor Bug Fixes Update
* Minor bug fixes. 
* Removed some barely used .ui files.
* Split the TopMenu of the MainWindow into its own widget. 
* More minor bug fixes. 

### **v0.9.10**
#### Another Optimizations Update
* Redid the ConnectionPage's UI code to run smoother.
* Moved the Moods picker to the GroupPage. 
* Fixed quality of life issues like inconsistent UX and hard to press buttons.
* Fixed miscellaneous bugs. 

### **v0.9.11**
#### A Miscelleanous GUI Update
* Fixed inconsistencies with text size across environments.
* Fixed issues with scrollbars in some environments.
* Changed the ListDeviceWidget.
* Fixed miscellaneous bugs.


### **v0.10.0**
#### The Menus Update
* Rearranged the menus.
* Added more page transitions. 
* Cleaned up the layout of scrolling widgets.
* Minor bug fixes. 


### **v0.10.1**
#### The Settings Page Update Part 2
* Added a Copyright Page and FAQ page.
* Refactored and redesigned the settings page.
* standardized more UI elements.
* Fixed miscellaneous bugs. 

### **v0.10.12**
#### Minor Bug Fixes Update
* Fixed layout of SettingsPage in some environments.
* Fixed miscellaneous bugs. 

### **v0.10.15**
#### The Moar Threads Update
* Turned DataSync into a Base class with a separate derived class for syncing arduinos, syncing hues, and syncing global settings.
* Added better support for groups and schedules being stored on a Hue Bridge
* Added initial prototype for the ColorSchemePicker. 
* Fixed miscellaneous bugs.


### **v0.11.0**
#### The Hue Bridge Update 
*The point of this update is to make Corluma do everything that the Phillips Hue app can do, with some minor caveats. The next few updates will remove those caveats and will give the Hue's all the capabilities that RGB-LED-Routines has.*
* Added new widgets for Hue Light information and discovery.
* Added backend support for Hue light discovery.
* Added frontend support for renaming and deleting Hue lights. 
* Added new widgets for common tasks, like searching and editing fields.
* Fixed miscellaneous layout issues.


### **v0.11.1**
#### The Boring Hue Update 
*This update handles some of the more "boring" support of the Phillips Hue Bridge. This fixes support for things like discovery edge cases, string length edge cases, etc.*
* Added better support for manual Hue discovery based off of serial numbers.
* Added better support for manual Hue Bridge discovery based off of IP address.
* Added support for using Hues in preset color groups such as ice, water, fire, etc. 
* Added better UI catches for incorrect string lengths and illegal arguments.
* Fixed miscellaneous bugs.

### **v0.11.11**
#### Minor Bug Fixes Update
*This update fixes some bugs with layout and resizing*
*  Minor Bug Fixes

### **v0.11.5**
#### The Rooms Update
*This update refactors how Phillips Hue data is stored to more closely match how it is handled in the Phillips Hue app.*
* Added backend support for Rooms and LightGroups for both Phillips Hue and Arduino. Lights can only be in one room, but can be in multiple light groups.
* All Hue Room and LightGroup data is now stored and saved on the Bridge, instead of in JSON. 
* Changed discovery methods of Phillips Hue lights to not count as fully discovered until the app is aware of all lights, schedules, and groups. 
* Changed Connection Page to show Rooms and Light Groups separately. 
* Fixed android rendering and display bugs. 
* Fixed miscellaneous bugs.

### **v0.11.7**
#### Communication Update
* Updated the messaging protocols to match RGB-LED-Routines protocols introduced in v2.8.
* Added system to DataSyncArduino to send more efficient packets to arduinos.
* Sped up communication to arduinos over wireless. 
* Fixed bug with using manual IP addresses to discovery Hue Bridges.

### **v0.12.0**
#### The Second Great Refactor Update
* Added support for RGB-LED-Routines API 2.1, which adds unique names to arduino lights.
* Added namespace `hue` for hue specific code.
* Added namespace `cor` for files that started with `Corluma` and functions that were previously part of `utils`.
* Removed namespace `utils`.
* Turned structs `SDeviceController`, `SLightDevice`, and `SHueLight` into classes.
* Fixed warnings from Clang Static Analyzer and expensive functions found by Valgrind function profiler.

### **v0.12.5**
#### The List and Light Switches Update
* Added support for ArduCor API 3.0.
* Added an on/off light switch to the ListDeviceWidget.
* Added the product type (a lightbulb, light cube, etc.) to the ListDeviceWidget.
* Refactored the ColorSchemeGrid to cor::Palette.
* Refactored the ListCollectionWidgets to be easier to maintain.

### **v0.12.8**
#### The Nanoleaf Update
* Added support for the Nanoleaf Aurora.
* Made better use of Q_DECLARE_METATYPE so that more signals and slots used enums. 
* Grouped the various CommTypes of the ArduCor (Serial, HTTP, UDP) into ArduCor specific settings instead of their own settings.
* Added a UPnPDiscovery object, so that the same UPnP stream can be used for multiple types of hardware.
* Fixed miscellaneous bugs.

### **v0.12.9**
#### The API Update
* Renamed ELightingRoutine to ERoutine and EColorGroup to EPalette.
* Refactored routines, color, and speed. They are now combined into a single command instead of requiring three separate for certain changes. 
* Changed Routine functions to rely more heavily on JSON data. 
* Split EProtocolType and ECommType. A protocol type refers to the type of commands it receives, a comm type refers to the hardware it uses to communicate. 
* Moved speed bar from SettingsPage to GroupPage.

### **v0.12.10**
#### The TopMenu Update
* Added a color palette to the top menu that displays the currently selected devices.
* Added a string to the top menu that represents the currently selected devices.
* Combined the Groups and Rooms pages. Split Moods to its own main page. 
* Simplified the cor::Button widget.
* Fixed miscellaneous bugs. 

### **v0.12.12**
#### Another Bug Fixes Update
* Fixed platform-specific layout issues (certain buttons too small on Mac OS X, etc.).
* Fixed bugs in the `EditGroupPage`. 
* Fixed edge cases in `CommLayer`.
* Fixed miscellaneous bugs.


### **v0.13.0**
#### The Great Refactor Part 3: The Why-Do-I-Name-These? Update
* Renamed enums from `EClass::eName` to `EClass::name`
* Removed the last `.ui` file, enforcing that all widgets follow the same design pattern. 
* Large naming refactor:
	* `ConnectionPage`-> `LightPage`
	* `GroupsPage`-> `PalettePage`
	* `MoodsPage`-> `MoodPage`
	* `lightingpage`-> `cor::Page`
	* `PaletteWidget`-> `LightVectorWidget`
* Removed `lightingprotocols.h` and moved its contents into `cor/protocols.h`.
* Refactored the constructors of widgets to be cleaner. 
* Added a `Palette` object for storing info on groups of colors.
* Refactored how preset palettes from `ArduCor` are handled. Instead of being stored in `DataLayer` they are now defined in a JSON file which is read and accessed by `PresetPalette`. 
* Classes no longer use preprocessors to be ignored, instead this is handled by the build files. 
* Started a push towards removeing `DataLayer` in favor of a design that doesn't require a god object for internal states. 
* Added icons and support for connecting to more types of Philips Hue lights. 
* Fixed miscellaneous bugs.

### **v0.13.1**
#### The Nanoleaf Update Part 2: Simultaneous Connections
* Added support for multiple simultaneous nanoleaf connections.
* Added Nanoleaf support for custom color palettes.
* Added the ability to rename Nanoleafs.
* Removed unnecessary conversion to ArduCor packets for Hue and Nanoleaf.
* Moved ProtocolSettings out of DataLayer.
* Switched HueInfoListWidget to be more generic and instead show information on all Lights.

### **v0.13.2**
#### The Multiple Bridges Update
* Rewrote BridgeDiscovery to handle connecting to multiple Phillips Bridges simultaneously.
* Fixed bugs in communicating with Hue products.
* Sped up discovery and handshake of Bridges. 
* Created JSONSaveData for handling saving and loading JSON from disk. 
* Fixed bugs in the GroupsParser.
* Fixed miscellaneous bugs.

### **v0.13.3**
#### The CommArduCor Update
* Created CommArduCor and ArduCorDiscovery objects to match the design of CommHue and CommNanoleaf.
* Removed CommArduCor specific functions and variables from CommLayer and CommType. 
* Removed CommLayer and DataLayer as dependencies of `ColorPage`, `MoodPage`, `ListDevicesGroupWidget`, `cor::Button`, and `GlobalSettingsWidget`.
* Created a system to detect when a light is not reachable.
* Fixed miscellaneous bugs in Hue's and Nanoleaf's new discovery objects.
* Rewrote the `cor::Light` and `GroupsParser` to use uniqueIDs for lights and controllers.
* Moved all hardware data that can potentially change (such as a Hue's name) out of save.json.
* Fixed miscellaneous bugs. 

### **v0.13.4**
#### The DeviceList Update
* Converted the `DataLayer` class to the `DeviceList` class.
* Added ArduCor lights to the LightInfo widget. 
* Simplified the handling of brightness when using Hues and Nanoleafs.
* Fixed bugs in displaying connection states on the discovery page.
* Fixed a bug with the first time loading of app default data. 
* Fixed a bug where the settings page was getting resized incorrectly.
* Fixed bugs with data not showing up in the LightInfo widget.
* Fixed miscellaneous bugs.

### **v0.13.45**
#### The AppSettings Update
* Refactored the `ProtocolSettings` object into a more general, `AppSettings` object.
* Fixed bugs in the arducor timeouts.
* Bound Hue groups and schedules to individual Hue bridges instead of storing them all in one large list. 
* Fixed bugs in Hue timeouts. 
* Fixed miscellaneous bugs. 

### **v0.13.47**
#### The HSV Update
*This update breaks the save data of moods by converting all RGB values to HSV.*
* Standardized all UI code to work off of HSV colorspace.
* Changed all backend code to convert immediately into HSV when light states are received, instead of maintaining different colorspaces for different hardware.
* Changed all abckend code to convert from HSV in its last steps before sending packets to lighting hardware.
* Started a new UI for the Hue Discovery page.
* Added widgets to see more information on what data is stored on a Philip's Bridge. 
* Added the ability to set a custom name for Bridges.
 
### **v0.13.5**
#### Yet Another Bug Fixes Update
*This update addresses bugs that have been sitting in the backlog for a while.*
* Removed `cor::Light` dependencies from the `ColorPicker`.
* Added a `SwatchVectorWidget` for picking and displaying color schemes.
* Fixed bugs with updating the custom color array for arducor devices.
* moved `DeviceList` into the namespace `cor::DeviceList`.
* Fixed bug with connecting to multiple Nanoleafs with manual IP addresses.
* Fixed miscellaneous bugs.

### **v0.13.55**
#### The Clang Code Model Update
* Fixed tons of clang warnings.
* Continued updating the Hue Bridge Discovery Page.
* Fixed miscellaneous bugs.

### **v0.13.6**
#### The ListWidget Refactor
* Removed the `ListCollectionWidget` and `ListCollectionSubWidget`.
* Split the functionality of `ListCollectionWidget` into two classes: a `DropDownTopWidget` and a `ListLayout`. 
* Made more scrollable sets of widgets use the same system.
* Fixed bugs on the edit page.

### **v0.13.7**
#### The ListWidget Refactor Part 2
* Refactored `ListGroupWidget` into `ListRoomWidget`.
* Added `GroupButtonsWidget`.
* Fixed miscellaneous bugs.

### **v0.13.8**
#### The LightPage And Dictionary Update
* Added a Miscellaneous section to the `LightPage` to show additional light groups.
* Fixed miscellaneous bugs with displaying groups on `LightPage`.
* Refactored exception handling.
* Added a templated dictionary class with a constant runtime for lookups.
* Refactored expensive lookups throughout the application to use the dictionary class. 
* Added [Catch2](https://github.com/catchorg/Catch2) for unit tests.

### **v0.13.9**
#### The Edge Cases Update
* Fixed bugs in Nanoleaf packet parsers.
* Added checks for wifi to the application.
* Added a `NoWifiWidget` for displaying when no wifi is detected.
* Added a disabled stated to `GroupButton`.
* Fixed miscellaneous edge cases with the app's layout. 
* Fixed miscellaneous bugs. 

### **v0.13.91**
#### Minor Bug Fixes
* Fixed bug with displaying connection state errors.
* Fixed bugs with the `cor::Slider` widget.
* Fixed miscellaneous bugs.

### **v0.14.0**
#### The Group and Mood Update
* Split `LightGroup.h` into `Group.h` and `Mood.h`
* Changed groups to use unique IDs for lights instead of full copies of the `Light` object
* Refactored all group and mood handling to use unique IDs as much as possible. 
* Renamed `GroupsParser` to `GroupData` and refactored its internal data structures to use dictionaries.
* Changed moods to allow more complicated options such as default states for lights in a room that are not explicitly defined.
* Changed layout of `MoodPage`.
* Optimized light and group lookups. 
* Added `ListMoodDetailedWidget`.
* Fixed miscellaneous bugs.

### **v0.14.20**
#### Another Edge Cases Update
* Added back button functionality for android. 
* Split `utils.h` into multiple files: `utils/color.h`, `utils/math.h`, and `utils/qt.h`.
* Added utility for moving widgets programmatically.
* Changed light save data so that if a light was ever discovered but cannot currently be reached, it now shows up as "Not Reachable" on the `LightPage` until it is explicitly deleted.
* Fixed bugs related to deleting nanoleafs and arducor lights. 
* Fixed miscellaneous bugs.

### **v0.15.0**
#### The Landscape Update
* Added a `LeftHandMenu`.
* Refactored support for landscape.
* Reorganized the `TopMenu` and `SettingsPage` to better support a `LeftHandMenu`.
* Fixed a crash when wifi is off. 
* Fixed miscellaneous bugs.

### **v0.15.1**
#### The Optimization Update
* Optimized how the `ListLightWidget` renders.
* Fixed some bugs with the `EditGroupPage`.
* Fixed edge cases with the `LeftHandMenu`.
* Fixed miscellaneous bugs.

### **v0.15.2**
#### Yet Another Edge Cases Update
* Drastically improved rendering performance on mobile devices.
* Removed the `LightPage` in favor of lights in the `LeftHandMenu` instead.
* Changed the layout of the `LeftHandMenu`.
* Fixed miscellaneous bugs.

### **v0.15.3**
#### The Clang-Tidy and Clazy Update
* Fixed warnings from Clang-Tidy and Clazy.
* Fixed miscellaneous bugs.

### **v0.16.0**
#### The ColorPicker Refactor Update
* Split `ColorPicker` into two: a `SingleColorPicker` and a `MultiColorPicker`.
* Added a HSV color wheel and updated the color temperature wheel, 
* Replaced the pre-rendered `.pngs` used for color wheels with wheels rendered by `QPaint`. 
* Added the ability to select different color schemes from color wheels on the `MultiColorPage`.
* Removed `CustomColorPicker` and `BrightnessSlider`.
* Updated `cor::Slider`.
* Fixed bugs for controlling ambient and dimmable lights. 
* Fixed miscellaneous bugs.

### **v0.16.1**
#### The Clang-Format Update
* Ran `clang-format`on codebase.

### **v0.16.15**
#### The Code Reorg Update
* Moved the `nanoleaf` `hue` and `arducor` directories into the `comm` directory.
* Added an `objects` and a `widgets` directory to the `cor` directory. 
* Fixed bugs with nanoleaf discovery.
* Synced top brightness control bar with `SingleColorPicker`.

### **v0.16.17**
#### The ColorPicker Refactor Part 2 Update
* Added the ability to generate color scheme icons to the `ColorSchemeCircles`.
* Optimized the `ColorWheel`. 
* Added visual feedback to touches on the `ColorWheel`.
* Fixed bugs with setting custom color vectors. 
* Fixed miscellaneous bugs and edge cases.

### **v0.16.19**
#### The SyncWidget Update
* Added `SyncStatus` for tracking whether or not any thread in the app is currently syncing lights.
* Added `SingleColorStateWidget` and `MultiColorStateWidget` to show the state the single and multi color pages are setting lights to and whether or not the selected lights are currently in sync.
* Fixed compilation issues with the iOS build.
* Fixed miscellaneous bugs.

### **v0.16.20**
#### The Resize Bug Fixes Update
* Fixed resize issues preventing widgets from rendering on iOS.
* Fixed miscellaneous bugs.

### **v0.17.0**
#### The Settings Update
* Added native share support to iOS and Android for backing up and loading save data.
* Changed the layout and text of the Settings Page. 
* Added a `TimeoutWidget` for controlling the timeout setting.
* Fixed bugs with resizing on iOS.
* Fixed bugs with supporting iPads. 
* Fixed miscellaneous bugs.

### **v0.17.1**
#### The Settings Polish Update
* Polished the `ShareUtils` integration.
* Implemented Nanoleaf idle timeouts. 
* Made a minor change to `clang-format` settings.

### **v0.17.2**
#### The Miscellaneous Bugs Update
* Fixed bugs with greyout. 
* Removed major lag when clicking for more information about a mood.
* Fixed sizes of certain assets.
* Fixed miscellaneous bugs. 

### **v0.17.3**
#### Another Bug Fixes Update
* Fixed bugs with scroll areas scrolling horizontally.
* Fixed places where text is cut off on small screen sizes.
* Fixed miscellaneous bugs.

### **v0.17.4**
#### Environment Specific Bug Fixes Update
* Fixed bugs in supporting smaller screens such as an iPod touch.
* Refactored the `LightInfoListWidget`.
* Fixed issues with scroll area layouts in specific environments.
* Fixed bugs with discovering multiple Hue Bridges. 
* Fixed miscellaneous bugs.

### **v0.17.41**
#### The Qt 5.13 Update
* Updated the application to support Qt 5.13 android builds. This bumps the minimum Qt version to 5.13.
* Fixed bugs with windows builds.
* Added support for more android phones.

### **v0.17.42**
#### The Android Update
* Updated android icons.
* Updated compliance with Google Play Store requirements.
* Fixed miscellaneous bugs.

### **v0.17.43**
#### The Setup Update
* Fixed a bug that required reloading the application after loading JSON data.
* Fixed a bug with discovering ArduCor lights on certain devices.
* Fixed a bug with lights being selected that can't be unselected.
* Fixed text resizing improperly in certain environments.
* Fixed deprecated Qt functions from the update to 5.13.
* Fixed miscellaneous bugs.

### **v0.17.44**
#### The Hue Manual IP Update
* Added a new widget to replace `QInputDialog` on mobile.
* Fixed bugs with discovering Hue Bridges using an IP address.
* Fixed miscellaneous bugs. 

### **v0.17.45**
#### The Resizing Update
* Fixed bugs with resizing the `LeftHandMenu`.
* Fixed miscellaneous resize bugs.
* Tapping outside of the `LeftHandMenu` now closes the menu.
* Fixed bugs with discovering Hue Bridges.
* Fixed `clang-tidy` warnings.
* Fixed miscellaneous bugs.

### **v0.17.46**
#### The App States Update
* `SyncWidgets` no longer show when nothning is being synced.
* Everything behind the `LeftHandMenu` is now greyed out when the menu is open.
* Fixed UI issues with loading new app data from json.
* Fixed miscellaneous bugs.


### **v0.17.5**
#### The Vector Update
- Changed all std::list to std::vector
- Adjusted `cor::Group` and `cor::Mood` to use proper getters/setters.
- Added a visual indication when a light can not be reached. 
- Split `EditGrouPage` into `EditPage`, `EditGroupPage` and `EditMoodPage`.


### **v0.17.55**
#### The Core Data Types Update
- Split `cor::Group` and `cor::Room` into distinct objects.
- Added new constructors for derived classes for `cor::Light`.
- Added proper getters/setters to `cor::Light`.


### **v0.17.56**
#### The cor::Controller and Nanoleaf Update
- Updated `cor::Controller`.
- Cleaned up the objects associated with Nanoleafs.
- Fixed miscellaneous bugs.


### **v0.17.57**
#### The Bug Fixes and Metadata Update
- Added a unique `HueMetadata`, `LeafMetadata`, and `ArduCorMetadata` class, each with their own API. These are used during discovery and to store light-specific information such as firmware version or hardware type.
- Fixed miscellaneous discovery bugs.
- Fixed bugs with the `SyncWidget`.
- Fixed issues with reachability changes breaking UI.
- Fixed miscellaneous bugs. 


### **v0.17.6**
#### The Discovery Edge Cases Update
- Removed `controller()` from `cor::Light`.
- Fixed edge cases in ArduCor discovery.
- Fixed miscellaneous bugs.


### **v0.17.63**
#### The LightState Update
- Added a `LightState` object and removed state data from `cor::Light`.
- Reduced the reliance on `QJsonObject` for tracking light state data.
- Fixed miscellaneous bugs.


### **v0.17.65**
#### The Nanoleaf Firmware Update
- Added support for Nanoleaf 3.2.0 firmware
- Rewrote nanoleaf code for detecting and setting routines and palettes.
- Fixed edge cases in nanoleaf discovery code.
- Added initial support for sending packets to hue groups and rooms. 


### **v0.17.68**
#### The Miscellaneous Cleanup Update
- Adjusted Nanoleaf to use global brightness levels, even for palettes. 
- Simplified `GroupData`.
- Fixed bugs with saving and modifying groups, rooms, and moods.
- Fixed layout of `ListLightWidget`.
- Fixed issues with handling app's global brightness in UI elements.
- Fixed msicellaneous bugs.


### **v0.17.7**
#### The Brightness Update
- Simplified how brightness is handled across UI widgets and backend code.
- Moved brightness and on/off switch out of `TopMenu` and into `GlobalBrightnessWidget`.
- Added a `SingleLightBrightnessWidget` which replaces the `GlobalBrightnessWidget` in certain cases.
- Fixed UI issues on small screens such as the iPod Touch.
- Fixed discovery issue with Nanoleafs.
- Fixed bugs in the `cor::LightVectorWidget`.
- Fixed miscellaneous bugs.


### **v0.17.72**
#### A Minor Bug Fixes Update
- Fixed a bug with page layouts when in portrait mode.
- Fixed unncessary rendering in `TopMenu`.
- Lights are now listed alphabetically in the info widget.
- Fixed miscellaneous bugs.


### **v0.17.73**
#### The 2020 Update
- Updated copyright to 2020.


### **v0.17.74**
#### The Docs Update
- Fixed bug with ambient color picker.
- Updated images and docs.


### **v0.18.0**
#### The StateObserver Update
- Added a StateObserver which looks at the state of the app and determines the desired state of the lights when the user interacts with the app.
- Cleaned up IconData.
- Combine the `SingleRoutineWidget` and `MultiRoutineWidget` into a single `RoutineWidget`.
- Fixed unnecessary includes.
- Fixed miscellaneous bugs.


### **v0.18.1**
#### The Minor Fixes Update
- Fixed issues with asset rendering. 
- Split `bridge.h` into `command.h`, `schedule.h`, and `bridge.h`. 
- Fixed issues with scroll areas.
- Fixed a bug where the user could accidentally convert a group into a room or vice versa.
- Fixed miscellaneous bugs.


### **v0.18.2**
#### Bug Fixes
- `SingleColorStateWidget` defaults its routine to `singleSolid` if only hues are selected.
- Clicking on a selected page on `LeftHandMenu` closes the menu and displays the page. 
- Fixed edge cases with greyout.
- Added additional state for `DiscoveryNanoleafWidget` when some but not all nanoleafs are discovered.
- Fixed errors with Hue Bridge discovery. 
- Fixed miscellaneous bugs.


### **v0.18.3**
#### Bug Fixes
- Fixed unncessary recomputation of color circle centers in `MultiColorPicker`.
- Fixed unncessary rendering of certain widgets.
- Cleaned up the `SingleLightBrightnessWidget`.


### **v0.18.4**
#### Android Deployment Update
- Switched builds for Android to Qt 5.14.
- Updated Android assets.


### **v0.18.5**
#### Bug Fixes
- Fixed bugs with states on `PalettePage`.
- Fixed sizes of widgets on `EditableFieldWidget`.
- Fixed android launcher icons not showing.
- Moved handling of moods from `MainWindow` to `MoodPage`.


### **v0.18.6**
#### The PalettePage Update
- Refactored `PalettePage` to no longer track a `ERoutine`.
- Fixed bugs with `PaletteScrollArea`.
- Fixed palette bugs with `DataSyncArduino`.
- Fixed edge cases with the brightness states when different lights with a variety of states are selected.
- Fixed edge cases with the brightness states when you change between pages.
- Fixed bugs in getting the `ERoutine` enum from `RoutineButtonsWidget`.


### **v0.18.7**
#### Bug Fixes
- Fixed bugs with manual Hue Bridge discovery.
- Fixed bugs with displaying the state of Hue Bridges during discovery.
- Fixed bugs with `BridgeInfoWidget` not updating properly. 


### **v0.18.75**
#### Bug Fixes
- Fixed an issue with Hue color modes.
- Fixed edge cases with the `LeftHandMenu` when it is always open.


### **v0.18.8**
#### Documentation Update
- Updated autogenerated HTML pages.
- Updated class documentation on important classes.
- Moved `Palette` into `cor` namespace.


### **v0.19.0**
#### The Interaction Update
- Fixed bugs with the global on/off switch.
- Added the `TouchListener` for handling top-level mouse and touch events.
- Fixed bugs with mouse events.
- Adjusted how the `LeftHandMenu` drags in and out.
- Made light state icons brighter.
- Added an experimental feature where light updates no longer idle out while the app is still active.
- Fixed a crash when sharing files from android.


### **v0.19.1**
#### Bug Fixes
- Simplified `cor::Slider`.
- Fixed bugs with `TouchListener`.


### **v0.19.2**
#### Bug Fixes
- Fixed a discovery bug when Hue Bridges change IP addresses.
- Fixed a layout bug with sliders in the `ColorPicker`.
- Fixed lag when moving the `LeftHandMenu` in android.


### **v0.19.3**
#### Bug Fixes
- Fixed bugs with handle sizes of `cor::Slider` in certain environments.
- Fix bugs in layouts with overlapping widgets.
- Fixed a bug where `GreyoutOverlay` wasn't showing properly.


### **v0.19.4**
#### The GroupButtonsWidget Update
_This is the first of a series of updates to address design issues in the `ListRoomWidget`. In this update, the main focus was simplifying the relationship between `ListRoomWidget` and `GroupButtonsWidget`._
- Simplified `GroupButtonsWidget` and `cor::GroupButton`.
- Simplified the relationship between `ListRoomWidget` and `GroupButtonsWidget`.
- Added a system to query the last time any `CommType` received an update.


### **v0.19.5**
#### The ListRoomWidget Update
- Simplified the logic in the `ListRoomWidget`.
- Fixed miscellaneous bugs.


### **v0.19.6**
#### The Orphans, Subgroups, and Parents Update
- Removed `subgroups` from rooms.
- Added `SubgroupData` to track the relationship between groups and rooms.
- Added `OrphanData` to track lights that belong to no group or room.
- Added `ParentData` to track groups that are either a room, or are no subgroup of any other group or room.
- Refactored `ListRoomWidget` to be a `ParentGroupWidget` instead.
- Each `CommType` now signals when a light is either added or deleted.


### **v0.19.63**
#### Bug Fixes
- Removed `cor::Room` and replaced it with a `cor::EGroupType` in `cor::Group`.
- Removed separate dictionaries for rooms in `GroupData` and `hue::Bridge`.
- Refactored `ParentGroupWidget` to signal with unique IDs instead of group names.
- Added a `keysAndItems()` function to `cor::Dictionary`.


### **v0.19.67**
#### Bug Fixes
- Refactored the relationship between the `LeftHandMenu` and `ParentGroupWidget`.
- Fixed miscellaneous bugs.


### **v0.19.7**
#### The LeftHandMenuScrollArea Update
- Added `LeftHandMenuScrollArea` and `LeftHandMenuScrollTopWidget`.
- Removed `ParentGroupWidget` in favor of the new widgets.
- `SubgroupData` now stores a vector of alternative names for each subgroup in the context of its parent group.
- Fixed bugs with the `LeftHandMenu`.


### **v0.19.75**
#### The LeftHandLightMenu Update
- Added `MenuLightContainer`, `MenuParentGroupContainer`, and `MenuSubgroupContainer` as high-level widgets to display `GroupData`.
- Added `LeftHandLightMenu`.
- Added `ParentGroupWidget` as a new widget that derives from `DropdownTopWidget`.
- Simplified the logic for updating the lights in the `LeftHandMenu`.
- Fixed bugs with the size of `ListLightWidget`.
- Fixed miscellaneous bugs.
- Fixed bugs with the `LeftHandMenu`.


### **v0.19.79**
#### The EditPage Update
- Added a new `EditPage` and a `EditProgressWidget`.
- Fixed bugs with the `LefthandLightMenu`.
- Fixed miscellaneous bugs.


### **v0.19.8**
#### Bug Fixes
- Fixed a crash when wifi is not enabled.
- Fixed a bug where the "Add New Group" button on the `SettingsPage` was greyed out erroneously.
- Added a new `EditGroupPage` class.


### **v0.19.85**
#### The New EditGroupPage Update
- Added `ChooseMetadataWidget` and `ChooseLightWidget` for building groups, rooms, and moods. 
- Added `EditBottomButtons` and fleshed out the `EditPage`.
- Added the complete workflow to create a new group with the new edit pages. 
- Added an optional field for group descriptions in the `GroupData`.

