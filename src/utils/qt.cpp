
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "utils/qt.h"

#include "mainwindow.h"

namespace cor {

QString makePrettyTimeOutput(QTime time) {
    QString output = time.toString();
    auto timeAgo = time.msecsTo(QTime::currentTime()) / 1000;
    if (timeAgo > 300) {
        timeAgo = timeAgo / 60.0;
        return output + " (" + QString::number(timeAgo) + "min ago)";
    }
    return output + " (" + QString::number(timeAgo) + "s ago)";
}

QStringList regexSplit(const QString& input, const QString& regex) {
#ifdef USE_QT_6
    QRegularExpression rx(regex);
    return input.split(rx, Qt::SkipEmptyParts);
#else
    QRegExp rx(regex);
    return input.split(rx, QString::SkipEmptyParts);
#endif
}

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
