#ifndef COR_UTILS_QT_H
#define COR_UTILS_QT_H

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */


#include <QApplication>
#include <QColor>
#include <QDebug>
#include <QLabel>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScreen>

#include "mainwindow.h"

#define TRANSITION_TIME_MSEC 150

namespace cor {

//-------------
// Qt Utils
//-------------

/*!
 * \brief changeLabelToTitleLabel Takes a label, and increases its font size and makes the font
 * bold. The font size provided varies from platform to platform, but will always be a factor larger
 * than the standard font size recommended by the system.
 * NOTE: This function overwrites the stylesheet for the label.
 *
 * \param label the label to change the font size on.
 */
inline void changeLabelToTitleLabel(QLabel* label) {
    // get the system preferred font size
    QLabel tempLabel("font");
    // make the size larger
    int newSize = int(tempLabel.font().pointSize() * 1.33);
    QString styleSheet =
        "background-color:rgba(0,0,0,0); font:bold; font-size:" + QString::number(newSize) + "pt;";
    label->setStyleSheet(styleSheet);
}

/// debug function for checking basic states of widget
inline void debugWhyWidgetIsNotShowing(QWidget* widget) {
    qDebug() << "----------";
    qDebug() << "Widget is Visible: " << widget->isVisible();
    qDebug() << "Geometry " << widget->geometry();
    qDebug() << "Min Size" << widget->minimumSize();
    qDebug() << "Max Size" << widget->maximumSize();
    qDebug() << "----------";
}


/// moves a widge from startPoint to endPoint, resizing to given size if necessary.
inline void moveWidget(QWidget* widget, const QPoint& startPoint, const QPoint& endPoint) {
    QPropertyAnimation* animation = new QPropertyAnimation(widget, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(startPoint);
    animation->setEndValue(endPoint);
    animation->start();
}

/*!
 * \brief isMouseEventTouchUpInside stealing terminology from iOS, a "touch up inside" is when a
 * touch is released while still within the bounds of the widget. this is meant to be ran from
 * mouseRelaseEvent functions to check if we can qualify this touch as a touch up inside
 *
 * \param mouseEvent the mouse event from a mouseRelaseEvent
 * \param widget the widget that created the mouseReleaseEvent
 * \return true if the touch is within the bounds of the widget, false otherwise
 */
inline bool isMouseEventTouchUpInside(QMouseEvent* mouseEvent,
                                      QWidget* widget,
                                      bool ignoreWidgetPos) {
    const auto& mousePos = mouseEvent->pos();
    const auto& widgetSize = widget->size();
    QPoint finalPoint;
    if (ignoreWidgetPos) {
        finalPoint = mousePos;
    } else {
        finalPoint =
            QPoint(mousePos.x() - widget->geometry().x(), mousePos.y() - widget->geometry().y());
    }
    if (finalPoint.x() < widgetSize.width() && finalPoint.x() > 0
        && finalPoint.y() < widgetSize.height() && finalPoint.y() > 0) {
        return true;
    } else {
        return false;
    }
}

/*!
 * \brief resizeIcon resize an icon in a QPushButton based off of the size of the icon.
 *
 * \param button button whose icon is going to get resized
 * \param iconPath path to the icon resource.
 * \param sizeRatio determines the ratio between icon size and button size. If none is provided,
 *        the icon takes up 66% of the button.
 */
inline void resizeIcon(QPushButton* button, QString iconPath, float sizeRatio = 0.8f) {
    QPixmap pixmap(iconPath);
    int min = std::min(button->width(), button->height());
    int finalSize = int(min * sizeRatio);
    button->setIconSize(QSize(finalSize, finalSize));
    button->setIcon(
        QIcon(pixmap.scaled(finalSize, finalSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
}

/*!
 * \brief applicationSize this returns the size of the MainWindow, in a pretty ugly but effective
 *        way.
 *
 * \return the size of the main window.
 */
inline QSize applicationSize() {
    QSize mainWindowSize(0, 0);
    for (auto widget : QApplication::topLevelWidgets()) {
        if (QString(widget->metaObject()->className()) == "MainWindow") {
            mainWindowSize = widget->size();
        }
    }
    return mainWindowSize;
}


/*!
 * \brief leftHandMenuMoving returns true if the lefthandmenu is currently in motion, false
 * otherwise. Used as a bit of an antipattern to detect the lefthand menu state to avoid edge cases
 * like buttons being pressed on the menu canceling the movement.
 *
 * \return true if left hand menu is moving, false otherwise
 */
inline bool leftHandMenuMoving() {
    for (auto widget : QApplication::topLevelWidgets()) {
        if (QString(widget->metaObject()->className()) == "MainWindow") {
            // cast to mainwindow
            auto mainWindow = qobject_cast<MainWindow*>(widget);
            return mainWindow->leftHandMenu()->geometry().x() < 0;
        }
    }
    return false;
}

} // namespace cor

#endif // COR_UTILS_QT_H