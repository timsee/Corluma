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


### **v0.9.7**
#### Connection Page Update
* Moved connecting to devices from Settings Page to the new Connection Page.
* When no devices are connected, assets that can't be used now grey themselves out.
* Rewrote system for turning on and off CommTypes.


