#-------------------------------------------------
#
# Corluma
# Copyright (C) 2015 - 2016.
# Released under the GNU General Public License.
# Full license in root of git repo.
#
# Project created by QtCreator 2015-12-26T19:10:52
#
#-------------------------------------------------

TARGET = Corluma
TEMPLATE = app

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
    error(ERROR: Qt5 is not installed. This project uses QT5.2 or later)
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
#       Phillips Hues, It is an optional dependency for discovery
#       although it is recommended as our testing shows it to be
#       the quickest method of discovery.

win32{
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
    comm/commserial.cpp \
    comm/commhttp.cpp \
    comm/commudp.cpp \
    comm/commtype.cpp \
    comm/commhue.cpp \
    comm/commpacketparser.cpp \
    comm/commtypesettings.cpp \
    comm/commlayer.cpp \
    comm/huebridgediscovery.cpp \
    comm/crccalculator.cpp \
    colorpicker/colorpicker.cpp \
    colorpicker/rgbsliders.cpp \
    colorpicker/tempbrightsliders.cpp \
    colorpicker/brightnessslider.cpp \
    colorpicker/colorgrid.cpp \
    discovery/discoveryserialwidget.cpp \
    discovery/discoverywidget.cpp \
    discovery/discoveryyunwidget.cpp \
    discovery/discoveryhuewidget.cpp \
    mainwindow.cpp \
    settingspage.cpp \
    datalayer.cpp \
    icondata.cpp \
    presetcolorspage.cpp \
    lightingroutines.cpp \
    connectionpage.cpp \
    floatinglayout.cpp \
    groupsparser.cpp \
    datasync.cpp \
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
    corlumabutton.cpp \
    corlumaslider.cpp \
    corlumautils.cpp \
    topmenu.cpp

HEADERS  +=  comm/commtype.h \
    comm/commserial.h \
    comm/commhttp.h \
    comm/commudp.h \
    comm/commhue.h \
    comm/commtypesettings.h \
    comm/commpacketparser.h \
    comm/commlayer.h \
    comm/huebridgediscovery.h \
    comm/hueprotocols.h \
    comm/crccalculator.h \
    colorpicker/colorpicker.h \
    colorpicker/rgbsliders.h \
    colorpicker/tempbrightsliders.h \
    colorpicker/brightnessslider.h \
    colorpicker/colorgrid.h \
    discovery/discoveryserialwidget.h \
    discovery/discoverywidget.h \
    discovery/discoveryyunwidget.h \
    discovery/discoveryhuewidget.h \
    mainwindow.h \
    settingspage.h \
    datalayer.h \
    icondata.h \
    lightingpage.h \
    lightingprotocols.h \
    lightingroutines.h \
    lightdevice.h \
    connectionpage.h \
    floatinglayout.h \
    groupsparser.h \
    datasync.h \
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
    grouppage.h \
    corlumabutton.h \
    corlumaslider.h \
    corlumautils.h \
    topmenu.h

FORMS    += settingspage.ui \
    connectionpage.ui \
    discoverypage.ui \
    editcollectionpage.ui \
    grouppage.ui

#----------
# Resources
#----------

RESOURCES  = resources.qrc

RC_ICONS = images/icon.ico # Windows icon
ICON = images/icon.icns    # Mac OS X icon

