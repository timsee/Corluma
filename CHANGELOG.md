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


### Coming Soon
* Fully integrate Rooms, Groups, and Schedules stored on the Hue Bridge.



