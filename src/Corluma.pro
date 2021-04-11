#-------------------------------------------------
#
# Corluma
# Copyright (C) 2015 - 2020.
# Released under the GNU General Public License.
# Full license in root of git repo.
#
# Project created by QtCreator 2015-12-26T19:10:52
#
#-------------------------------------------------

TARGET = Corluma
linux:!android {
    TARGET = corluma
}
TEMPLATE = app
VERSION = 0.21.89

#----------
# Build flags
#----------

# flag to use experimental features that may not be part of the standard release.
SHOULD_USE_EXPERIMENTAL_FEATURES = 1
# flag to use serial ArduCor devices. Not all versions of Qt nor do all supported devices have serial.
SHOULD_USE_SERIAL = 1
# flag to build in support for shareutils. This allows sharing on mobile devices, but can conflict in mobile updates
SHOULD_USE_SHARE_UTILS = 1

#----------
# Build flag edge case handling
#----------

equals (QT_MAJOR_VERSION, 6) {
    SHOULD_USE_SERIAL = 0
    message("DEBUG: Overriding serial setting, QT 6.0.0 does not have qserialport.")
}
#todo: why isnt this getting caught in android
android {
    SHOULD_USE_SERIAL = 0
    message("DEBUG: Overriding serial setting, android devices do not have serial ports.")
}
ios {
    SHOULD_USE_SERIAL = 0
    message("DEBUG: Overriding serial setting, ios devices do not have serial ports.")
}
#----------
# Minimum requirements Check
#----------

# check for proper version of Qt
message("DEBUG: Qt Version: $$QT_MAJOR_VERSION _ $$QT_MINOR_VERSION arch: $$QT_ARCH " )
!greaterThan(QT_MAJOR_VERSION, 4) {
    !greaterThan(QT_MINOR_VERSION, 11) {
        !greaterThan(QT_PATCH_VERSION, 4) {
            error(ERROR: This project uses Qt 5.12.5 or later.)
        }
    }
}

#----------
# Variable definition
#----------

# settings defines
equals(SHOULD_USE_EXPERIMENTAL_FEATURES, 1) {
    DEFINES += USE_EXPERIMENTAL_FEATURES=1
}
equals(SHOULD_USE_SERIAL, 1) {
    DEFINES += USE_SERIAL=1
}
equals(SHOULD_USE_SHARE_UTILS, 1) {
    DEFINES += USE_SHARE_UTILS=1
}

# qt version defines
equals (QT_MAJOR_VERSION, 6) {
    DEFINES += USE_QT_6=1
}

# environment defines
android {
    DEFINES += MOBILE_BUILD=1
}
ios {
    DEFINES += MOBILE_BUILD=1
}

# static build defines
equals(BUILD_STATIC_CORLUMA, 1) {
    # If building a static build, it is easiest to make a unique define for each environment.
    # This way, you can load the proper plugins per environment using macros in code.
    DEFINES += BUILD_STATIC_CORLUMA=1
    linux {
        DEFINES += CORLUMA_QT_STATIC_LINK_LINUX=1
    }
}

# stores the app version for display in app
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

#----------
# Config
#----------

CONFIG += c++17 #adds C++17 support
equals(BUILD_STATIC_CORLUMA, 1) {
    message("DEBUG: Building static Qt.")
    CONFIG += static
}

#----------
# Dependencies
#----------

QT += core gui widgets network
equals(SHOULD_USE_SERIAL, 1) {
  QT += serialport
}


#----------
# Desktop settings
#----------

# openSSL is not included in Qt due to legal restrictions
# in some countries. This links windows against an openSSL
# library downloaded from this project:
# http://slproweb.com/products/Win32OpenSSL.html
#
# NOTE: This dependency is currently only used for discovering
#       Philips Hues, It is an optional dependency for discovery
#       although it is recommended  since it is typically the
#       quickest method of discovery.

win32:win64 {
    message("DEBUG: Using OpenSSL installed on windows.")
    # uses default path for openSSL in 32 and 64 bit
    contains(QT_ARCH, i386) {
        message("Using windows 32 bit libraries")
        LIBS += -LC:/Program\ Files\ (x86)/OpenSSL-Win32/lib -lubsec
        INCLUDEPATH += C:/OpenSSL-Win32/include
    } else {
        message("Using windows 64 bit libraries")
        LIBS += -LC:/Program\ Files/OpenSSL-Win64/lib -lubsec
        INCLUDEPATH += C:/OpenSSL-Win64/include
    }
}


#----------
# mobile settings
#----------

android {
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
    OPEN_SSL = $$OPENSSL_PATH
    !isEmpty(OPEN_SSL) {
      message("DEBUG: looking Android openssl at: $$OPENSSL_PATH")
      include($$OPEN_SSL)
    }
    isEmpty(OPEN_SSL) {
      message("INFO: Not using OPENSSL_PATH for openssl libraries, make sure that openSSL is included from the environment")
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

     ANDROID_ABIS = armeabi-v7a arm64-v8a
}

ios {
   # Info.plist is the top level global configuration file for iOS
   # for things like app name, icons, screen orientations, etc.
   QMAKE_INFO_PLIST = ios/Info.plist
   QMAKE_TARGET_BUNDLE_PREFIX = com.cor.corluma
   # adds the icon files to the iOS application
   ios_icon.files = $$files($$PWD/ios/icon/AppIcon*.png)
   QMAKE_BUNDLE_DATA += ios_icon

   # add the launch images
   app_launch_images.files = $$files($$PWD/ios/launch_image/LaunchImage*.png)
   QMAKE_BUNDLE_DATA += app_launch_images
}


#----------
# Resources
#----------

RESOURCES  = resources.qrc

# Windows icon
RC_ICONS = images/icon.ico
# macOS icon
ICON = images/icon.icns

#----------
# Sources
#----------

SOURCES += main.cpp \
    comm/arducor/arducordiscovery.cpp \
    comm/arducor/arducorpacketparser.cpp \
    comm/arducor/controller.cpp \
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
    comm/datasynctimeout.cpp \
    comm/nanoleaf/leafeffectcontainer.cpp \
    comm/nanoleaf/leafmetadata.cpp \
    comm/nanoleaf/leafpanelimage.cpp \
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
    controllerwidget.cpp \
    cor/lightlist.cpp \
    cor/widgets/palettewidget.cpp \
    debugconnectionspoofer.cpp \
    discovery/discoveryhuewidget.cpp \
    discovery/discoverynanoleafwidget.cpp \
    discovery/discoverytypewidget.cpp \
    discovery/discoveryarducorwidget.cpp \
    discoverywidget.cpp \
    display/displaypreviewbridgewidget.cpp \
    cor/widgets/slider.cpp \
    cor/widgets/listwidget.cpp \
    cor/presetpalettes.cpp \
    cor/jsonsavedata.cpp \
    cor/widgets/lightvectorwidget.cpp \
    cor/listlayout.cpp \
    cor/widgets/groupbutton.cpp \
    display/rotatelightwidget.cpp \
    edit/chooseeditpage.cpp \
    edit/choosegroupwidget.cpp \
    edit/choosemoodwidget.cpp \
    edit/editpage.cpp \
    edit/editprogresswidget.cpp \
    groupstatewidget.cpp \
    lightspage.cpp \
    listmoodwidget.cpp \
    menu/choosestatewidget.cpp \
    menu/displaygroupmetadata.cpp \
    menu/displaymoodmetadata.cpp \
    menu/groupstatelistmenu.cpp \
    menu/kitchentimerwidget.cpp \
    menu/lightslistmenu.cpp \
    menu/menugroupcontainer.cpp \
    menu/menugroupstatecontainer.cpp \
    menu/menulightcontainer.cpp \
    menu/menumoodcontainer.cpp \
    menu/menuparentgroupcontainer.cpp \
    menu/menusubgroupcontainer.cpp \
    menu/standardlightsmenu.cpp \
    menu/standardmoodsmenu.cpp \
    menu/statelesslightslistmenu.cpp \
    routines/routinecontainer.cpp \
    mooddetailedwidget.cpp \
    moodsyncwidget.cpp \
    parentgroupwidget.cpp \
    stateobserver.cpp \
    data/subgroupdata.cpp \
    storedpalettewidget.cpp \
    timeoutpage.cpp \
    utils/qt.cpp \
    utils/cormath.cpp \
    comm/hue/lightdiscovery.cpp \
    comm/hue/bridgediscovery.cpp \
    comm/hue/bridge.cpp \
    comm/hue/hueinfowidget.cpp \
    comm/hue/bridgegroupswidget.cpp \
    comm/hue/bridgescheduleswidget.cpp \
    comm/hue/huegroupwidget.cpp \
    comm/hue/hueschedulewidget.cpp \
    comm/nanoleaf/leafdiscovery.cpp \
    comm/syncstatus.cpp \
    globalbrightnesswidget.cpp \
    mainwindow.cpp \
    settingspage.cpp \
    icondata.cpp \
    floatinglayout.cpp \
    greyoutoverlay.cpp \
    colorpage.cpp \
    singlelightbrightnesswidget.cpp \
    topmenu.cpp \
    settingsbutton.cpp \
    globalsettingswidget.cpp \
    searchwidget.cpp \
    palettepage.cpp \
    moodpage.cpp \
    lightinfolistwidget.cpp \
    appsettings.cpp \
    listsimplegroupwidget.cpp \
    dropdowntopwidget.cpp \
    nowifiwidget.cpp \
    listmoodpreviewwidget.cpp \
    data/groupdata.cpp \
    menu/lefthandmenu.cpp \
    selectlightsbutton.cpp \
    mainviewport.cpp \
    menu/lefthandbutton.cpp \
    listlightwidget.cpp \
    addnewgroupbutton.cpp \
    palettescrollarea.cpp \
    syncwidget.cpp \
    singlecolorstatewidget.cpp \
    multicolorstatewidget.cpp \
    lightinfoscrollarea.cpp \
    touchlistener.cpp

HEADERS  +=  comm/arducor/arducordiscovery.h \
    comm/arducor/arducormetadata.h \
    comm/arducor/arducorpacketparser.h \
    comm/arducor/controller.h \
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
    comm/datasynctimeout.h \
    comm/hue/bridgebutton.h \
    comm/hue/command.h \
    comm/hue/huemetadata.h \
    comm/hue/schedule.h \
    comm/nanoleaf/leafeffect.h \
    comm/nanoleaf/leafeffectcontainer.h \
    comm/nanoleaf/leafeffectpage.h \
    comm/nanoleaf/leafeffectscrollarea.h \
    comm/nanoleaf/leafeffectwidget.h \
    comm/nanoleaf/leafmetadata.h \
    comm/nanoleaf/leafpacketparser.h \
    comm/nanoleaf/leafpanelimage.h \
    comm/nanoleaf/leafprotocols.h \
    comm/nanoleaf/leafschedulewidget.h \
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
    connectionbutton.h \
    controllerwidget.h \
    cor/lightlist.h \
    cor/objects/groupstate.h \
    cor/widgets/expandingtextscrollarea.h \
    cor/widgets/palettewidget.h \
    data/moodparentdata.h \
    debugconnectionspoofer.h \
    discovery/discoveryhuewidget.h \
    discovery/discoverynanoleafwidget.h \
    discovery/discoverytopmenu.h \
    discovery/discoverytypewidget.h \
    discovery/discoveryarducorwidget.h \
    cor/protocols.h \
    cor/range.h \
    cor/presetpalettes.h \
    cor/jsonsavedata.h \
    cor/listlayout.h \
    cor/dictionary.h \
    discoverywidget.h \
    display/displayarducorcontrollerwidget.h \
    display/displayhuebridgewidget.h \
    display/displaynanoleafcontrollerwidget.h \
    display/displaynanoleafscheduleswidget.h \
    display/displaypreviewarducorwidget.h \
    display/displaypreviewnanoleafwidget.h \
    display/displaypreviewbridgewidget.h \
    display/rotatelightwidget.h \
    edit/chooseeditpage.h \
    edit/choosegroupwidget.h \
    edit/chooselightsgroupwidget.h \
    edit/choosemetadatawidget.h \
    edit/choosemoodgroupstateswidget.h \
    edit/choosemoodlightstateswidget.h \
    edit/choosemoodwidget.h \
    edit/editbottombuttons.h \
    edit/editgrouppage.h \
    edit/editmoodpage.h \
    edit/editpage.h \
    edit/editpagechildwidget.h \
    edit/editprogressstate.h \
    edit/editprogresswidget.h \
    edit/grouproomcheckboxwidget.h \
    edit/reviewgroupwidget.h \
    edit/reviewmoodwidget.h \
    globalbrightnesswidget.h \
    data/orphandata.h \
    data/parentdata.h \
    groupstatewidget.h \
    lightspage.h \
    listmoodwidget.h \
    listplaceholderwidget.h \
    menu/choosestatewidget.h \
    menu/displaygroupmetadata.h \
    menu/displaygroupwidget.h \
    menu/displaymoodmetadata.h \
    menu/displaymoodwidget.h \
    menu/groupstatelistmenu.h \
    menu/kitchentimerwidget.h \
    menu/lefthandmenutoplightwidget.h \
    menu/lightslistmenu.h \
    menu/lightstimeoutmenu.h \
    menu/menugroupcontainer.h \
    menu/menugroupstatecontainer.h \
    menu/menulightcontainer.h \
    menu/menumoodcontainer.h \
    menu/menuparentgroupcontainer.h \
    menu/menusubgroupcontainer.h \
    menu/standardlightsmenu.h \
    menu/standardmoodsmenu.h \
    menu/statelesslightslistmenu.h \
    menu/timeoutbutton.h \
    routines/fadebutton.h \
    routines/routinecontainer.h \
    mooddetailedwidget.h \
    moodsyncwidget.h \
    parentgroupwidget.h \
    routines/routinewidget.h \
    routines/multiglimmerroutinewidget.h \
    routines/multibarsroutinewidget.h \
    routines/multifaderoutinewidget.h \
    routines/multirandomroutinewidget.h \
    routines/singlefaderoutinewidget.h \
    routines/singleglimmerroutinewidget.h \
    routines/singlesolidroutinewidget.h \
    routines/singlewaveroutinewidget.h \
    routines/speedslider.h \
    singlelightbrightnesswidget.h \
    speedwidget.h \
    stateobserver.h \
    data/subgroupdata.h \
    storedpalettewidget.h \
    timeobserver.h \
    timeoutpage.h \
    utils/exception.h \
    comm/hue/lightdiscovery.h \
    comm/hue/bridgediscovery.h \
    comm/hue/hueprotocols.h \
    comm/hue/bridge.h \
    comm/hue/hueinfowidget.h \
    comm/hue/bridgegroupswidget.h \
    comm/hue/bridgescheduleswidget.h \
    comm/hue/huegroupwidget.h \
    comm/hue/hueschedulewidget.h \
    comm/nanoleaf/panels.h \
    comm/nanoleaf/rhythmcontroller.h \
    comm/nanoleaf/leafdiscovery.h \
    comm/nanoleaf/leafdate.h \
    comm/nanoleaf/leafschedule.h \
    comm/nanoleaf/leafaction.h \
    comm/syncstatus.h \
    mainwindow.h \
    settingspage.h \
    icondata.h \
    floatinglayout.h \
    greyoutoverlay.h \
    colorpage.h \
    topmenu.h \
    settingsbutton.h \
    globalsettingswidget.h \
    searchwidget.h \
    palettepage.h \
    moodpage.h \
    lightinfolistwidget.h \
    appsettings.h \
    listsimplegroupwidget.h \
    dropdowntopwidget.h \
    nowifiwidget.h \
    listmoodpreviewwidget.h \
    data/groupdata.h \
    utils/reachability.h \
    utils/color.h \
    utils/qt.h \
    menu/lefthandmenu.h \
    selectlightsbutton.h \
    mainviewport.h \
    menu/lefthandbutton.h \
    listlightwidget.h \
    addnewgroupbutton.h \
    palettescrollarea.h \
    syncwidget.h \
    singlecolorstatewidget.h \
    multicolorstatewidget.h \
    utils/cormath.h \
    lightinfoscrollarea.h \
    touchlistener.h


HEADERS  += cor/objects/light.h \
    cor/objects/lightstate.h \
    cor/objects/group.h \
    cor/objects/palette.h \
    cor/objects/page.h \
    cor/objects/mood.h \
    cor/widgets/slider.h \
    cor/widgets/button.h \
    cor/widgets/checkbox.h \
    cor/widgets/switch.h \
    cor/widgets/webview.h \
    cor/widgets/listwidget.h \
    cor/widgets/topwidget.h \
    cor/widgets/groupbutton.h \
    cor/widgets/lightvectorwidget.h \
    cor/widgets/listitemwidget.h \
    cor/widgets/textinputwidget.h


#----------
# Build specific sources
#----------

equals(SHOULD_USE_SERIAL, 1) {
    HEADERS += comm/commserial.h
    SOURCES += comm/commserial.cpp
}

#--------
# ShareUtils setup
#--------

equals(SHOULD_USE_SHARE_UTILS, 1) {
    # shareutils.hpp contains all the C++ code needed for calling the native shares on iOS and android
    HEADERS += shareutils/shareutils.hpp

    ios {
        # Objective-C++ files are needed for code that mixes objC and C++
        OBJECTIVE_SOURCES += shareutils/iosshareutils.mm

        # Headers for objC classes must be separate from a C++ header.
        HEADERS += shareutils/docviewcontroller.hpp
    }
    android {
        # Android Manifests are the top level global xml for things like
        # app name, icons, screen orientations, etc.
        OTHER_FILES += android/AndroidManifest.xml

        # used for JNI calls
        QT += androidextras

        # the source file that contains the JNI and the android share implementation
        SOURCES += shareutils/androidshareutils.cpp

        # this contains the java code for and parsing and sending android intents, and
        # the android activity required to read incoming intents.
        OTHER_FILES += android/src/org/cor/corluma/utils/QShareUtils.java \
            android/src/org/cor/corluma/utils/QSharePathResolver.java \
            android/src/org/cor/corluma/activity/QShareActivity.java
    }
}
