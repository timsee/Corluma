# Corluma

Corluma is a cross-platform GUI designed to control Phillips Hue lights, Corsair RGB Keyboards, and arduino sketches from the [RGB-LED-Routines project](https://github.com/timsee/RGB-LED-Routines). It supports Windows, Linux, Mac OS X, Android, and iOS. It requires on Qt5.2 or later and C++11.

## <a name="toc"></a>Table of Contents

* [Progress](#progress)
* [Mobile Builds](#mobile-support)
* [Frontend Overview](https://timsee.github.io/Corluma/html/front_overview.html)
* [Backend Overview](https://timsee.github.io/Corluma/html/backend_overview.html)
* [Credits](#credits)
* [Version Notes](CHANGELOG.md)
* [Contributing](#contributing)
* [License](#license)

##<a name="progress"></a>Progress

| Hardware     | Progress        | Notes            |
| ------------- |  ------------- |  ------------- |
| RGB-LED-Routines (UDP) |  Complete |   |
| RGB-LED-Routines (HTTP) |  Complete |  |
| RGB-LED-Routines (Serial) |  Complete |  PC Builds only |
| Phillips Hue |  Complete |   |
| Corsair RGB Keyboards |  Not Yet Implemented |  |


##<a name="mobile-support"></a>Mobile Builds

The GUI makes use of Qt5's support for Android and iOS. To use it, set up your Qt Development environment
to work with either android or iOS, attach the device, and build to that target. You may need to update
your user specific settings in Qt in order to recognize the new targets.

The mobile application can currently support RGB-LED-Routines arduino yun sketches and Phillips Hues.

The screen orientation for mobile devices will support portrait and landscape for tablets with an aspect 
ratio of 4:3, such as an iPad. For phones and thinner tablets, only portrait will be supported.

There are a couple of known issues with our current mobile support:

* In Qt versions before iOS 5.5.2, there is a compatibility issue with Apple's bitcode setting. 
To get around this, either install a more recent version of Qt, or go into the Xcodeproj generated by 
Qt Creator and change the enable bitcode setting to off. For more information, [check here](http://lists.qt-project.org/pipermail/interest/2015-October/019393.html). 
* On android screens with extremely high pixel density, some of the stylesheet and Qt resizeEvents are
still a bit buggy and images may appear smaller than they are intended to be. 

##<a name="credits"></a>Credits

* The style sheet of the project is heavily based on [QDarkStyleSheet](https://github.com/ColinDuquesnoy/QDarkStyleSheet) 
  by Colin Duquesnoy. The major differences between our version and the original are that we handle some of the background 
  drawing as QPaintEvents and some of the fixed heights and widths have been removed. 
* The color wheel's image asset was created by a modified version of the [Python Color Gamut Generator](https://github.com/jacksongabbard/Python-Color-Gamut-Generator) by Jackson Gabbard.
* Code solutions taken from stackoverflow contain links to the original solutions in the source code. 

## <a name="contributing"></a>Contributing

1. Fork it!
2. Create your feature branch: `git checkout -b my-new-feature`
3. Commit your changes: `git commit -am 'Add some feature'`
4. Push to the branch: `git push origin my-new-feature`
5. Submit a pull request.


## <a name="license"></a>License

GNU General Public License, provided [here](LICENSE).


