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


