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

/// uncomment to wipe out all QSettings Data.
//#define WIPE_QSETTINGS 1
/// uncomment to wipe out all json data, even in local directory
//#define WIPE_JSON 1
/// uncomment to print system info in debug statement
//#define PRINT_SYSINFO 1


/// uncomment to disable the splash screen.
#define DISABLE_SPLASH_SCREEN 1
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
    // create app icon
    //--------------------

#ifdef __APPLE__
    QIcon icon(":images/icon.icns");
#else
    QIcon icon(":images/icon.ico");
#endif


    //--------------------
    // optional macros
    //--------------------

    // used to override settings that are persisent
    // between sessions and clear them out, useful for debugging
#ifdef WIPE_QSETTINGS
    QSettings mSettings;
    mSettings.clear();
#endif

    //--------------------
    // Wipe JSON data
    //--------------------

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
    QSplashScreen *splash = new QSplashScreen;
    splash->setPixmap(QPixmap(":images/color_wheel.png"));
    splash->show();
#endif

    //--------------------
    // Create app window
    //--------------------

    MainWindow window;
    // set the icon
    window.setWindowIcon(icon);

#ifndef DISABLE_SPLASH_SCREEN
    // create single shot timer to add a delay before hiding the splash screen.
    int splashScreenDelay = 2500; // in milliseconds
    QTimer::singleShot(splashScreenDelay, splash, SLOT(close()));
    QTimer::singleShot(splashScreenDelay, &window,SLOT(show()));
#else
    window.show();
#endif

    //splash.finish(*window);
    return a.exec();
}
