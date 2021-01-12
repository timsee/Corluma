
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "utils/qt.h"

#include "mainwindow.h"

namespace cor {

QSize applicationSize() {
    QSize mainWindowSize(0, 0);
    for (auto widget : QApplication::topLevelWidgets()) {
        if (QString(widget->metaObject()->className()) == "MainWindow") {
            mainWindowSize = widget->size();
        }
    }
    return mainWindowSize;
}

bool leftHandMenuMoving() {
    for (auto widget : QApplication::topLevelWidgets()) {
        if (QString(widget->metaObject()->className()) == "MainWindow") {
            // cast to mainwindow
            auto mainWindow = qobject_cast<MainWindow*>(widget);
            return (mainWindow->leftHandMenu()->geometry().x() < 0)
                   && (mainWindow->leftHandMenu()->geometry().width() * -1
                       > mainWindow->leftHandMenu()->geometry().x());
        }
    }
    return false;
}

bool leftHandMenuAlwaysOpen() {
    for (auto widget : QApplication::topLevelWidgets()) {
        if (QString(widget->metaObject()->className()) == "MainWindow") {
            // cast to mainwindow
            auto mainWindow = qobject_cast<MainWindow*>(widget);
            return mainWindow->leftHandMenu()->alwaysOpen();
        }
    }
    return false;
}

MainWindow* mainWindow() {
    for (auto widget : QApplication::topLevelWidgets()) {
        if (QString(widget->metaObject()->className()) == "MainWindow") {
            // cast to mainwindow
            return qobject_cast<MainWindow*>(widget);
        }
    }
    return nullptr;
}

#ifdef USE_SHARE_UTILS
ShareUtils* shareUtils() {
    return mainWindow()->shareUtils();
}
#endif

} // namespace cor
