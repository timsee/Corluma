#-------------------------------------------------
#
# Corluma
# Copyright (C) 2015 - 2019.
# Released under the GNU General Public License.
# Full license in root of git repo.
#
# Project created by QtCreator 2015-12-26T19:10:52
#
#-------------------------------------------------

TARGET = Corluma
TEMPLATE = app
VERSION = 1.0.0


# Define these paths to include libcrypto and libssl from your machine. This is
# required for android 7.0 and later but is optional for other builds.
# For more info on how to build and install: http://doc.qt.io/qt-5/opensslsupport.html
LIB_CRYPTO_ANDROID = $$PWD/../../../Libraries/openssl/libcrypto.so
LIB_SSL_ANDROID = $$PWD/../../../Libraries/openssl/libssl.so

#----------
# Minimum Requirements Check
#----------

# check for proper version of Qt
message("DEBUG: Qt Version: $$QT_MAJOR_VERSION _ $$QT_MINOR_VERSION arch: $$QT_ARCH " )
equals (QT_MAJOR_VERSION, 5)  {
  !greaterThan(QT_MINOR_VERSION, 12) {
    error(ERROR: Qt5 is installed, but it is not a recent enough version. This project uses QT5.13 or later)
  }
}
!equals(QT_MAJOR_VERSION, 5) {
    error(ERROR: Qt5 is not installed. This project uses QT5.13 or later)
}

CONFIG += c++11 #adds C++11 support

#----------
# Dependencies check
#----------

# openSSL is not included in Qt due to legal restrictions
# in some countries. This links windows against an openSSL
# library downloaded from this project:
# http://slproweb.com/products/Win32OpenSSL.html
#
# NOTE: This dependency is currently only used for discovering
#       Philips Hues, It is an optional dependency for discovery
#       although it is recommended as our testing shows it to be
#       the quickest method of discovery.

win32 {
    # uses default path for openSSL in 32 and 64 bit
    contains(QT_ARCH, i386) {
        # message("Using windows 32 bit libraries")
        LIBS += -LC:/OpenSSL-Win32/lib -lubsec
        INCLUDEPATH += C:/OpenSSL-Win32/include
    } else {
        # message("Using windows 64 bit libraries")
        LIBS += -LC:/OpenSSL-Win64/lib -lubsec
        INCLUDEPATH += C:/OpenSSL-Win64/include
    }
}


#----------
# Qt Linking
#----------
QT   += core gui widgets network

# Does not set up the qt serial port on mobile devices
# since they can't support it.
!android:!ios {
  QT +=  serialport
}

# MOBILE_BUILD is a flag that gets set for only MOBILE targets.
# This is useful for things liek QSerialPort, which don't translate
# well to a mobile device and are not supported by Qt.
android {
   DEFINES += MOBILE_BUILD=1
   # Android Manifests are the top level global xml for things like
   # app name, icons, screen orientations, etc.
   OTHER_FILES += android/AndroidManifest.xml
   ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
    # adds prebuilt libcrypto and libssl for android versions 6 and later
    contains(ANDROID_TARGET_ARCH, armeabi-v7a) {
        ANDROID_EXTRA_LIBS = \
            $$LIB_CRYPTO_ANDROID \
            $$LIB_SSL_ANDROID
    }

    # Android Play Store requires different version numbers for armv7 and arm64
    # https://www.qt.io/blog/2019/06/28/comply-upcoming-requirements-google-play
    defineReplace(droidVersionCode) {
            segments = $$split(1, ".")
            for (segment, segments): vCode = "$$first(vCode)$$format_number($$segment, width=3 zeropad)"
            contains(ANDROID_TARGET_ARCH, arm64-v8a): \
                suffix = 1

            else:contains(ANDROID_TARGET_ARCH, armeabi-v7a): \
                suffix = 0
            return($$first(vCode)$$first(suffix))
    }

    ANDROID_VERSION_NAME = $$VERSION
    ANDROID_VERSION_CODE = $$droidVersionCode($$ANDROID_VERSION_NAME)
}

ios {
   DEFINES += MOBILE_BUILD=1
   # Info.plist is the top level global configuration file for iOS
   # for things like app name, icons, screen orientations, etc.
   QMAKE_INFO_PLIST = ios/Info.plist
   # adds the icon files to the iOS application
   ios_icon.files = $$files($$PWD/ios/icon/AppIcon*.png)
   QMAKE_BUNDLE_DATA += ios_icon
}

#----------
# Sources
#----------

SOURCES += main.cpp \
    comm/arducor/arducordiscovery.cpp \
    comm/arducor/arducorpacketparser.cpp \
    comm/arducor/controller.cpp \
    comm/arducor/arducorinfowidget.cpp \
    comm/arducor/crccalculator.cpp \
    comm/commarducor.cpp \
    comm/commhttp.cpp \
    comm/commudp.cpp \
    comm/commtype.cpp \
    comm/commhue.cpp \
    comm/commnanoleaf.cpp \
    comm/commlayer.cpp \
    comm/datasync.cpp \
    comm/datasynchue.cpp \
    comm/datasyncarduino.cpp \
    comm/datasyncnanoleaf.cpp \
    comm/upnpdiscovery.cpp \
    colorpicker/colorpicker.cpp \
    colorpicker/rgbsliders.cpp \
    colorpicker/tempbrightsliders.cpp \
    colorpicker/colorschemecircles.cpp \
    colorpicker/swatchvectorwidget.cpp \
    colorpicker/colorschemechooser.cpp \
    colorpicker/colorschemebutton.cpp \
    colorpicker/hsvsliders.cpp \
    colorpicker/colorwheel.cpp \
    colorpicker/singlecolorpicker.cpp \
    colorpicker/multicolorpicker.cpp \
    colorpicker/schemegenerator.cpp \
    discovery/discoverywidget.cpp \
    discovery/discoveryhuewidget.cpp \
    discovery/discoverynanoleafwidget.cpp \
    discovery/hardwareconnectionwidget.cpp \
    discovery/discoveryarducorwidget.cpp \
    cor/widgets/slider.cpp \
    cor/widgets/listwidget.cpp \
    cor/presetpalettes.cpp \
    cor/jsonsavedata.cpp \
    cor/widgets/lightvectorwidget.cpp \
    cor/listlayout.cpp \
    cor/devicelist.cpp \
    cor/widgets/groupbutton.cpp \
    comm/hue/bridgeinfowidget.cpp \
    comm/hue/lightdiscovery.cpp \
    comm/hue/bridgediscovery.cpp \
    comm/hue/bridge.cpp \
    comm/hue/hueinfowidget.cpp \
    comm/hue/bridgegroupswidget.cpp \
    comm/hue/bridgescheduleswidget.cpp \
    comm/hue/huegroupwidget.cpp \
    comm/hue/hueschedulewidget.cpp \
    comm/nanoleaf/leafcontroller.cpp \
    comm/nanoleaf/leafdiscovery.cpp \
    comm/nanoleaf/leafcontrollerinfowidget.cpp \
    comm/syncstatus.cpp \
    mainwindow.cpp \
    settingspage.cpp \
    icondata.cpp \
    floatinglayout.cpp \
    discoverypage.cpp \
    presetgroupwidget.cpp \
    greyoutoverlay.cpp \
    listmoodgroupwidget.cpp \
    editgrouppage.cpp \
    routinebuttonswidget.cpp \
    colorpage.cpp \
    topmenu.cpp \
    settingsbutton.cpp \
    globalsettingswidget.cpp \
    editablefieldwidget.cpp \
    searchwidget.cpp \
    editpagetopmenu.cpp \
    palettepage.cpp \
    moodpage.cpp \
    lightinfolistwidget.cpp \
    appsettings.cpp \
    listsimplegroupwidget.cpp \
    dropdowntopwidget.cpp \
    listroomwidget.cpp \
    groupbuttonswidget.cpp \
    nowifiwidget.cpp \
    listmooddetailedwidget.cpp \
    listmoodpreviewwidget.cpp \
    groupdata.cpp \
    mooddetailswidget.cpp \
    lefthandmenu.cpp \
    selectlightsbutton.cpp \
    mainviewport.cpp \
    lefthandbutton.cpp \
    listlightwidget.cpp \
    addnewgroupbutton.cpp \
    palettescrollarea.cpp \
    syncwidget.cpp \
    singlecolorstatewidget.cpp \
    multicolorstatewidget.cpp \
    timeoutwidget.cpp \
    lightinfoscrollarea.cpp

HEADERS  +=  comm/arducor/arducordiscovery.h \
    comm/arducor/arducorpacketparser.h \
    comm/arducor/controller.h \
    comm/arducor/arducorinfowidget.h \
    comm/arducor/crccalculator.h \
    comm/commtype.h \
    comm/commarducor.h \
    comm/commhttp.h \
    comm/commudp.h \
    comm/commhue.h \
    comm/commnanoleaf.h \
    comm/commlayer.h \
    comm/datasync.h \
    comm/datasynchue.h \
    comm/datasyncarduino.h \
    comm/datasyncnanoleaf.h \
    comm/upnpdiscovery.h \
    colorpicker/colorpicker.h \
    colorpicker/rgbsliders.h \
    colorpicker/tempbrightsliders.h \
    colorpicker/colorschemecircles.h \
    colorpicker/swatchvectorwidget.h \
    colorpicker/colorschemechooser.h \
    colorpicker/colorschemebutton.h \
    colorpicker/hsvsliders.h \
    colorpicker/colorwheel.h \
    colorpicker/singlecolorpicker.h \
    colorpicker/multicolorpicker.h \
    colorpicker/schemegenerator.h \
    discovery/discoverywidget.h \
    discovery/discoveryhuewidget.h \
    discovery/discoverynanoleafwidget.h \
    discovery/hardwareconnectionwidget.h \
    discovery/discoveryarducorwidget.h \
    cor/protocols.h \
    cor/range.h \
    cor/presetpalettes.h \
    cor/jsonsavedata.h \
    cor/devicelist.h \
    cor/listlayout.h \
    cor/dictionary.h \
    utils/exception.h \
    comm/hue/lightdiscovery.h \
    comm/hue/bridgediscovery.h \
    comm/hue/hueprotocols.h \
    comm/hue/huelight.h \
    comm/hue/bridge.h \
    comm/hue/hueinfowidget.h \
    comm/hue/bridgeinfowidget.h \
    comm/hue/bridgegroupswidget.h \
    comm/hue/bridgescheduleswidget.h \
    comm/hue/huegroupwidget.h \
    comm/hue/hueschedulewidget.h \
    comm/nanoleaf/panels.h \
    comm/nanoleaf/rhythmcontroller.h \
    comm/nanoleaf/leafcontroller.h \
    comm/nanoleaf/leafdiscovery.h \
    comm/nanoleaf/leafcontrollerinfowidget.h \
    comm/nanoleaf/leafdate.h \
    comm/nanoleaf/leafschedule.h \
    comm/nanoleaf/leafaction.h \
    comm/syncstatus.h \
    mainwindow.h \
    settingspage.h \
    icondata.h \
    floatinglayout.h \
    discoverypage.h \
    presetgroupwidget.h \
    greyoutoverlay.h \
    listmoodgroupwidget.h \
    editgrouppage.h \
    routinebuttonswidget.h \
    colorpage.h \
    topmenu.h \
    settingsbutton.h \
    globalsettingswidget.h \
    editablefieldwidget.h \
    searchwidget.h \
    editpagetopmenu.h \
    palettepage.h \
    moodpage.h \
    lightinfolistwidget.h \
    appsettings.h \
    listsimplegroupwidget.h \
    dropdowntopwidget.h \
    listroomwidget.h \
    groupbuttonswidget.h \
    nowifiwidget.h \
    listmooddetailedwidget.h \
    listmoodpreviewwidget.h \
    groupdata.h \
    mooddetailswidget.h \
    utils/reachability.h \
    utils/color.h \
    utils/qt.h \
    lefthandmenu.h \
    selectlightsbutton.h \
    mainviewport.h \
    lefthandbutton.h \
    listlightwidget.h \
    addnewgroupbutton.h \
    palettescrollarea.h \
    syncwidget.h \
    singlecolorstatewidget.h \
    multicolorstatewidget.h \
    utils/cormath.h \
    timeoutwidget.h \
    lightinfoscrollarea.h


HEADERS  += cor/objects/light.h \
    cor/objects/group.h \
    cor/objects/palette.h \
    cor/objects/page.h \
    cor/objects/mood.h \
    cor/widgets/slider.h \
    cor/widgets/button.h \
    cor/widgets/checkbox.h \
    cor/widgets/switch.h \
    cor/widgets/statusicon.h \
    cor/widgets/webview.h \
    cor/widgets/listwidget.h \
    cor/widgets/topwidget.h \
    cor/widgets/groupbutton.h \
    cor/widgets/lightvectorwidget.h \
    cor/widgets/listitemwidget.h

#----------
# Desktop builds only
#----------

!android:!ios {
HEADERS  +=  comm/commserial.h
SOURCES += comm/commserial.cpp
}

#--------
# Native Share Sources
#--------

# shareutils.hpp contains all the C++ code needed for calling the native shares on iOS and android
HEADERS += shareutils/shareutils.hpp

ios {
    # Objective-C++ files are needed for code that mixes objC and C++
    OBJECTIVE_SOURCES += shareutils/iosshareutils.mm

    # Headers for objC classes must be separate from a C++ header.
    HEADERS += shareutils/docviewcontroller.hpp
}
android {
    # used for JNI calls
    QT += androidextras

    # the source file that contains the JNI and the android share implementation
    SOURCES += shareutils/androidshareutils.cpp

    # this contains the java code for and parsing and sending android intents, and
    # the android activity required to read incoming intents.
    OTHER_FILES += android/src/org/corluma/utils/QShareUtils.java \
        android/src/org/corluma/utils/QSharePathResolver.java \
        android/src/org/corluma/activity/QShareActivity.java
}


#----------
# Resources
#----------

RESOURCES  = resources.qrc

RC_ICONS = images/icon.ico # Windows icon
ICON = images/icon.icns    # Mac OS X icon

