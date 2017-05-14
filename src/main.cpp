/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "mainwindow.h"
#include "groupsparser.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QSplashScreen>
#include <QTimer>
#include <QThread>
#include <QDesktopWidget>

/// uncomment to wipe out all QSettings Data.
//#define WIPE_QSETTINGS 1
/// uncomment to wipe out all json data, even in local directory
//#define WIPE_JSON 1
/// uncomment to print system info in debug statement
//#define PRINT_SYSINFO 1


/// uncomment to disable the splash screen.
//#define DISABLE_SPLASH_SCREEN 1
/// uncomment to disable the style sheet.
//#define DISABLE_STYLE_SHEET 1


int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    //--------------------
    // set app name
    //--------------------

    QCoreApplication::setOrganizationName("Corluma");
    QCoreApplication::setApplicationName("Corluma");

    //--------------------
    // create app icon
    //--------------------

#ifdef __APPLE__
    QIcon icon(":images/icon.icns");
#else
    QIcon icon(":images/icon.ico");
#endif

    //--------------------
    // load app style sheet
    //--------------------

#ifndef DISABLE_STYLE_SHEET
    QFile f(":stylesheet/corluma.qss");
    if (!f.exists()) {
        throw "Unable to set stylesheet, file not found\n";
    } else {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }
#endif


    //--------------------
    // Optional Data Cleaning
    //--------------------

// removes saved controllers and app settings
#ifdef WIPE_QSETTINGS
    QSettings mSettings;
    mSettings.clear();
#endif

// removes saved groups of devices
#ifdef WIPE_JSON
    GroupsParser *parser = new GroupsParser();
    parser->removeAppData();
    delete parser;
#endif

    //--------------------
    // Get SysInfo
    //--------------------

#ifdef PRINT_SYSINFO
    qDebug() << "";
    qDebug() << "";
    qDebug() << "--------------------";
    qDebug() << "CPU: "                 << QSysInfo::buildCpuArchitecture();
    qDebug() << "kernel type: "         << QSysInfo::kernelType();
    qDebug() << "mac version: "         << QSysInfo::macVersion();
    qDebug() << "host name: "           << QSysInfo::machineHostName();
    qDebug() << "pretty product name: " << QSysInfo::prettyProductName();
    qDebug() << "product type: "        << QSysInfo::productType();
    qDebug() << "product version: "     << QSysInfo::productVersion();
    qDebug() << "windows version: "     << QSysInfo::windowsVersion();
    qDebug() << "--------------------";
    qDebug() << "";
    qDebug() << "";
#endif

    //--------------------
    // Create splash screen
    //--------------------

#ifndef DISABLE_SPLASH_SCREEN
#ifdef MOBILE_BUILD
    QSplashScreen splash(QPixmap(":images/color_wheel.png"), Qt::WindowStaysOnTopHint);
#else
    QSplashScreen splash(QPixmap(":images/splash_screen.png"), Qt::WindowStaysOnTopHint);
#endif
#endif

    //--------------------
    // Create app window
    //--------------------

    MainWindow window;
    // set the icon
    window.setWindowIcon(icon);

    // center the application
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    int x = (screenGeometry.width()-window.width()) / 2;
    int y = (screenGeometry.height()-window.height()) / 2;
    window.move(x, y);

#ifndef DISABLE_SPLASH_SCREEN
    int splashScreenDelay = 3000; // in milliseconds
    QTimer::singleShot(splashScreenDelay, &splash, SLOT(close()));
    QTimer::singleShot(splashScreenDelay, &window,SLOT(show()));
    splash.show();
#else
    window.show();
#endif

    // bring window to front in some environments
    window.activateWindow();

    //--------------------
    // Load Backend Data
    //--------------------
    return a.exec();
}
