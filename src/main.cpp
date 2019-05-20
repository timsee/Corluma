/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "appsettings.h"
#include "utils/exception.h"
#include "groupdata.h"
#include "mainwindow.h"
#include "utils/qt.h"

#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QFile>
#include <QSplashScreen>
#include <QTextStream>
#include <QThread>
#include <QTimer>

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


const static QString kFirstTimeOpenKey = QString("Corluma_FirstTimeOpen");


int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    QSettings settings;

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
        THROW_EXCEPTION("Unable to set stylesheet, file not found");
    }
    f.open(QFile::ReadOnly | QFile::Text);
    QTextStream ts(&f);
    a.setStyleSheet(ts.readAll());
#endif


    //--------------------
    // Optional Data Cleaning
    //--------------------
// removes saved controllers and app settings
#ifdef WIPE_QSETTINGS
    settings.clear();
#endif

// removes saved groups of devices
#ifdef WIPE_JSON
    GroupsParser* parser = new GroupsParser();
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
    qDebug() << "CPU: " << QSysInfo::buildCpuArchitecture();
    qDebug() << "kernel type: " << QSysInfo::kernelType();
    qDebug() << "mac version: " << QSysInfo::macVersion();
    qDebug() << "host name: " << QSysInfo::machineHostName();
    qDebug() << "pretty product name: " << QSysInfo::prettyProductName();
    qDebug() << "product type: " << QSysInfo::productType();
    qDebug() << "product version: " << QSysInfo::productVersion();
    qDebug() << "windows version: " << QSysInfo::windowsVersion();
    qDebug() << "";
    qDebug() << "Default Save Path: " << GroupsParser::defaultSavePath();
    qDebug() << "";
    qDebug() << "Supports SSL:" << QSslSocket::supportsSsl();
    qDebug() << "SSL Library Build Version: " << QSslSocket::sslLibraryBuildVersionString();
    qDebug() << "SSL Library Version: " << QSslSocket::sslLibraryVersionString();
    qDebug() << "--------------------";
    qDebug() << "";
    qDebug() << "";
#endif

    //--------------------
    // Create splash screen
    //--------------------

#ifndef DISABLE_SPLASH_SCREEN
#ifdef MOBILE_BUILD
    QSplashScreen splash(QPixmap(":images/mobileLaunchScreen.png"), Qt::WindowStaysOnTopHint);
#else
    QSplashScreen splash(QPixmap(":images/splash_screen.png"), Qt::WindowStaysOnTopHint);
#endif
#endif


    //--------------------
    // check for first time opening
    //--------------------
    /*!
     * check for default value here, if its a default its the first time opening,
     * since the value function will return the default value if none is found
     */
    if (settings.value(kFirstTimeOpenKey, QVariant(127)) == QVariant(127)) {
        // add default settings
        settings.setValue(cor::kUseTimeoutKey, QString::number(int(true)));
        settings.setValue(cor::kTimeoutValue, QString::number(120));


        std::vector<QString> protocolInUseKeys = AppSettings::protocolKeys();
        for (const auto& key : protocolInUseKeys) {
            settings.setValue(key, QString::number(int(true)));
        }
        // set arducor to off
        settings.setValue(protocolInUseKeys[std::size_t(EProtocolType::arduCor)],
                          QString::number(int(false)));


        // set the value so it no longer gives a default back.
        settings.setValue(kFirstTimeOpenKey, QString::number(10));
    }

    //--------------------
    // Create app window
    //--------------------

    MainWindow window(nullptr);
    // set the icon
    window.setWindowIcon(icon);

    // center the application
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    int x = (screenGeometry.width() - window.width()) / 2;
    int y = (screenGeometry.height() - window.height()) / 2;
    window.move(x, y);

#ifndef DISABLE_SPLASH_SCREEN
    int splashScreenDelay = 3000; // in milliseconds
    QTimer::singleShot(splashScreenDelay, &splash, SLOT(close()));
    QTimer::singleShot(splashScreenDelay, &window, SLOT(show()));
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
