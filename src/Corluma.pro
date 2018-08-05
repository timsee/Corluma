#-------------------------------------------------
#
# Corluma
# Copyright (C) 2015 - 2018.
# Released under the GNU General Public License.
# Full license in root of git repo.
#
# Project created by QtCreator 2015-12-26T19:10:52
#
#-------------------------------------------------

TARGET = Corluma
TEMPLATE = app


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
  !greaterThan(QT_MINOR_VERSION, 1) {
    error(ERROR: Qt5 is installed, but it is not a recent enough version. This project uses QT5.2 or later)
  }
}
!greaterThan(QT_MAJOR_VERSION, 4) {
    error(ERROR: Qt5 is not installed. This project uses QT5.4 or later)
}

#todo: check for C++11
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
   OTHER_FILES += android-sources/AndroidManifest.xml
   ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android-sources
    # adds prebuilt libcrypto and libssl for android versions 6 and later
    contains(ANDROID_TARGET_ARCH, armeabi-v7a) {
        ANDROID_EXTRA_LIBS = \
            $$LIB_CRYPTO_ANDROID \
            $$LIB_SSL_ANDROID
    }
}

ios {
   DEFINES += MOBILE_BUILD=1
   # Info.plist is the top level global configuration file for iOS
   # for things like app name, icons, screen orientations, etc.
   QMAKE_INFO_PLIST = ios-sources/Info.plist
   # adds the icon files to the iOS application
   ios_icon.files = $$files($$PWD/ios-sources/icon/AppIcon*.png)
   QMAKE_BUNDLE_DATA += ios_icon
}

#----------
# Sources
#----------

SOURCES += main.cpp \
    arducor/arducordiscovery.cpp \
    arducor/arducorpacketparser.cpp \
    arducor/controller.cpp \
    arducor/arducorinfowidget.cpp \
    arducor/crccalculator.cpp \
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
    comm/datasyncsettings.cpp \
    comm/upnpdiscovery.cpp \
    colorpicker/colorpicker.cpp \
    colorpicker/rgbsliders.cpp \
    colorpicker/tempbrightsliders.cpp \
    colorpicker/brightnessslider.cpp \
    colorpicker/colorschemecircles.cpp \
    colorpicker/customcolorpicker.cpp \
    colorpicker/swatchvectorwidget.cpp \
    discovery/discoverywidget.cpp \
    discovery/discoveryhuewidget.cpp \
    discovery/discoverynanoleafwidget.cpp \
    discovery/hardwareconnectionwidget.cpp \
    discovery/discoveryarducorwidget.cpp \
    cor/light.cpp \
    cor/button.cpp \
    cor/checkbox.cpp \
    cor/slider.cpp \
    cor/listwidget.cpp \
    cor/statusicon.cpp \
    cor/topwidget.cpp \
    cor/webview.cpp \
    cor/switch.cpp \
    cor/lightgroup.cpp \
    cor/presetpalettes.cpp \
    cor/palette.cpp \
    cor/jsonsavedata.cpp \
    cor/lightvectorwidget.cpp \
    cor/devicelist.cpp \
    hue/bridgeinfowidget.cpp \
    hue/lightdiscovery.cpp \
    hue/bridgediscovery.cpp \
    hue/bridge.cpp \
    hue/huelight.cpp \
    hue/hueinfowidget.cpp \
    hue/bridgegroupswidget.cpp \
    hue/bridgescheduleswidget.cpp \
    hue/huegroupwidget.cpp \
    hue/hueschedulewidget.cpp \
    nanoleaf/panels.cpp \
    nanoleaf/rhythmcontroller.cpp \
    nanoleaf/leafcontroller.cpp \
    nanoleaf/leafdiscovery.cpp \
    nanoleaf/leafcontrollerinfowidget.cpp \
    mainwindow.cpp \
    settingspage.cpp \
    icondata.cpp \
    floatinglayout.cpp \
    groupsparser.cpp \
    discoverypage.cpp \
    listdevicewidget.cpp \
    presetgroupwidget.cpp \
    listdevicesgroupwidget.cpp \
    listcollectionwidget.cpp \
    greyoutoverlay.cpp \
    listmoodwidget.cpp \
    listmoodgroupwidget.cpp \
    editgrouppage.cpp \
    routinebuttonswidget.cpp \
    colorpage.cpp \
    topmenu.cpp \
    settingsbutton.cpp \
    globalsettingswidget.cpp \
    editablefieldwidget.cpp \
    searchwidget.cpp \
    listeditwidget.cpp \
    editpagetopmenu.cpp \
    lightpage.cpp \
    palettepage.cpp \
    moodpage.cpp \
    lightinfolistwidget.cpp \
    appsettings.cpp

HEADERS  +=  arducor/arducordiscovery.h \
    arducor/arducorpacketparser.h \
    arducor/controller.h \
    arducor/arducorinfowidget.h \
    arducor/crccalculator.h \
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
    comm/datasyncsettings.h \
    comm/datasyncnanoleaf.h \
    comm/upnpdiscovery.h \
    colorpicker/colorpicker.h \
    colorpicker/rgbsliders.h \
    colorpicker/tempbrightsliders.h \
    colorpicker/brightnessslider.h \
    colorpicker/colorschemecircles.h \
    colorpicker/customcolorpicker.h \
    colorpicker/swatchvectorwidget.h \
    discovery/discoverywidget.h \
    discovery/discoveryhuewidget.h \
    discovery/discoverynanoleafwidget.h \
    discovery/hardwareconnectionwidget.h \
    discovery/discoveryarducorwidget.h \
    cor/light.h \
    cor/protocols.h \
    cor/utils.h \
    cor/button.h \
    cor/checkbox.h \
    cor/slider.h \
    cor/listwidget.h \
    cor/statusicon.h \
    cor/topwidget.h \
    cor/webview.h \
    cor/switch.h \
    cor/lightgroup.h \
    cor/range.h \
    cor/page.h \
    cor/presetpalettes.h \
    cor/palette.h \
    cor/jsonsavedata.h \
    cor/lightvectorwidget.h \
    cor/devicelist.h \
    hue/lightdiscovery.h \
    hue/bridgediscovery.h \
    hue/hueprotocols.h \
    hue/huelight.h \
    hue/bridge.h \
    hue/hueinfowidget.h \
    hue/bridgeinfowidget.h \
    hue/bridgegroupswidget.h \
    hue/bridgescheduleswidget.h \
    hue/huegroupwidget.h \
    hue/hueschedulewidget.h \
    nanoleaf/panels.h \
    nanoleaf/rhythmcontroller.h \
    nanoleaf/leafcontroller.h \
    nanoleaf/leafdiscovery.h \
    nanoleaf/leafcontrollerinfowidget.h \
    mainwindow.h \
    settingspage.h \
    icondata.h \
    floatinglayout.h \
    groupsparser.h \
    discoverypage.h \
    listdevicewidget.h \
    presetgroupwidget.h \
    listdevicesgroupwidget.h \
    listcollectionwidget.h \
    greyoutoverlay.h \
    listmoodwidget.h \
    listmoodgroupwidget.h \
    editgrouppage.h \
    listcollectionsubwidget.h \
    routinebuttonswidget.h \
    colorpage.h \
    topmenu.h \
    settingsbutton.h \
    globalsettingswidget.h \
    editablefieldwidget.h \
    searchwidget.h \
    listeditwidget.h \
    editpagetopmenu.h \
    lightpage.h \
    palettepage.h \
    moodpage.h \
    lightinfolistwidget.h \
    appsettings.h

#----------
# Desktop builds only
#----------

!android:!ios {
HEADERS  +=  comm/commserial.h
SOURCES += comm/commserial.cpp
}

#----------
# Resources
#----------

RESOURCES  = resources.qrc

RC_ICONS = images/icon.ico # Windows icon
ICON = images/icon.icns    # Mac OS X icon

