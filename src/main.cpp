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

/// uncomment to wipe out all QSettings Data.
//#define WIPE_QSETTINGS 1
/// uncomment to wipe out all json data, even in local directory
//#define WIPE_JSON 1
/// uncomment to print system info in debug statement
//#define PRINT_SYSINFO 1

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

    QFile f(":qdarkstyle/style.qss");
    if (!f.exists()) {
        qDebug() << "Unable to set stylesheet, file not found\n";
    } else {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }

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
    // Create app window
    //--------------------

    MainWindow w;
    // set the icon
    w.setWindowIcon(icon);
    // show the window
    w.show();
    return a.exec();
}
