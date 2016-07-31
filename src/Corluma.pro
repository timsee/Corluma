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

# check for Qt5
!greaterThan(QT_MAJOR_VERSION, 4){
#todo handle the case of 5.2 or greater, or 6.0 or greater
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

#message("DEBUG: QT_ARCH = $$QT_ARCH")
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

SOURCES += main.cpp\
    mainwindow.cpp \
    singlecolorpage.cpp \
    settingspage.cpp \
    datalayer.cpp \
    commlayer.cpp \
    icondata.cpp \
    lightsslider.cpp \
    colorpicker.cpp \
    lightsbutton.cpp \
    customcolorspage.cpp \
    presetcolorspage.cpp \
    commserial.cpp \
    commhttp.cpp \
    commudp.cpp \
    commtype.cpp \
    commhue.cpp \
    huebridgediscovery.cpp \
    lightslistwidget.cpp \
    lightingroutines.cpp

HEADERS  += mainwindow.h \
    singlecolorpage.h \
    settingspage.h \
    datalayer.h \
    commlayer.h \
    icondata.h \
    lightsslider.h \
    lightingpage.h \
    colorpicker.h \
    lightsbutton.h \
    lightingprotocols.h \
    customcolorspage.h \
    presetcolorspage.h \
    commtype.h \
    commserial.h \
    commhttp.h \
    commudp.h \
    commhue.h \
    huebridgediscovery.h \
    lightslistwidget.h \
    lightingroutines.h

FORMS    += mainwindow.ui \
    singlecolorpage.ui \
    settingspage.ui \
    customcolorspage.ui \
    presetcolorspage.ui

#----------
# Resources
#----------

RESOURCES     = qdarkstyle/style.qrc \
    extraresources.qrc

RC_ICONS = images/icon.ico # Windows icon
ICON = images/icon.icns    # Mac OS X icon




