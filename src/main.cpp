/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QSplashScreen>
#include <QTextStream>
#include <QThread>
#include <QTimer>

#include "appsettings.h"
#include "cor/widgets/loadingscreen.h"
#include "mainwindow.h"
#include "utils/exception.h"
#include "utils/qt.h"

#ifdef BUILD_STATIC_CORLUMA
#include <QtPlugin>

#ifdef CORLUMA_QT_STATIC_LINK_LINUX
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
#endif

#endif

/// uncomment to wipe out all QSettings Data.
//#define WIPE_QSETTINGS 1
/// uncomment to print system info in debug statement
//#define PRINT_SYSINFO 1


/// uncomment to disable the splash screen.
//#define DISABLE_SPLASH_SCREEN 1
/// uncomment to disable the style sheet.
//#define DISABLE_STYLE_SHEET 1
/// uncomment to force portrait on desktop
//#define FORCE_PORTRAIT 1

const static QString kFirstTimeOpenKey = QString("Corluma_FirstTimeOpen");


int main(int argc, char* argv[]) {
    QCoreApplication::setOrganizationName("Corluma");
    QCoreApplication::setApplicationName("Corluma");

#if defined(Q_OS_IOS)
    // band-aid hack to fix font scaling on iOS devices.
    qputenv("QT_FONT_DPI", "48");
#endif

    QApplication a(argc, argv);
    QSettings settings;
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

#ifdef MOBILE_BUILD
    QScreen* screen = QApplication::screens().at(0);
    QSize size = screen->size();
#elif FORCE_PORTRAIT
    QSize size(400, 600);
#else
    QSize size(700, 600);
#endif


#ifndef DISABLE_SPLASH_SCREEN
    QSplashScreen splash(LoadingScreen::splashScreenPixmap(size), Qt::WindowStaysOnTopHint);
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
        AppSettings::setToDefaults();

        // set the value so it no longer gives a default back.
        settings.setValue(kFirstTimeOpenKey, QString::number(10));
        settings.sync();
    }

    //--------------------
    // Create app window
    //--------------------

#ifdef Q_OS_ANDROID
    qputenv("QT_QPA_NO_TEXT_HANDLES", "1");
#endif
    MainWindow window(nullptr, size, size);
    // set the icon
    window.setWindowIcon(icon);

    // center the application
    auto screens = QGuiApplication::screens();
    if (!screens.empty()) {
        auto screenGeometry = screens[0]->geometry();
        int x = (screenGeometry.width() - window.width()) / 2;
        int y = (screenGeometry.height() - window.height()) / 2;
        window.move(x, y);
    }


#ifndef DISABLE_SPLASH_SCREEN
#ifndef Q_OS_IOS
    int splashScreenDelay = 2000; // in milliseconds
    QTimer::singleShot(splashScreenDelay, &splash, SLOT(close()));
    QTimer::singleShot(splashScreenDelay, &window, SLOT(show()));
    splash.show();
#else
    window.show();
#endif

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
