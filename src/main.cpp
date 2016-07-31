/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("Corluma");
    QCoreApplication::setApplicationName("Corluma");

    // used to override settings that are persisent
    // between sessions and clear them out, useful for debugging
//    QSettings mSettings;
//    mSettings.clear();

    // load the style sheet
    QFile f(":qdarkstyle/style.qss");
    if (!f.exists()) {
        qDebug() << "Unable to set stylesheet, file not found\n";
    } else {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }

    MainWindow w;
    // choose the icon using preprocessors
#ifdef __APPLE__
    QIcon icon(":images/icon.icns");
#else
    QIcon icon(":images/icon.ico");
#endif
    // set the icon
    w.setWindowIcon(icon);

    // show the window
    w.show();
    return a.exec();
}
