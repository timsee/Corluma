#ifndef TOUCHLISTENER_H
#define TOUCHLISTENER_H

#include <QMouseEvent>
#include <QObject>

#include "cor/lightlist.h"

class MainWindow;
class LeftHandMenu;
class MainViewport;
class TopMenu;

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 *
 * \brief The TouchListener class handles the logic for listening for touch and mouse events. The
 * general design pattern used by Corluma for touch events is if a touch event should be consumed by
 * a widget and not propagate through the rest of the system, then its the widget's responsibility
 * to handle its own touch events. Otherwise, all touch events that can get handled by the
 * MainWindow, which then sends its touch events to this class for interpeting what to do wtih them.
 *
 * A major portion of this class' function is to handle how the LeftHandMenu is pulled in and out in
 * versions of the app in portait mode.
 */
class TouchListener : public QObject {
    Q_OBJECT
public:
    /// constructor
    explicit TouchListener(MainWindow* mainWindow,
                           LeftHandMenu* leftHandMenu,
                           TopMenu* topMenu,
                           cor::LightList* data);

    /// handles when the mouse is pressed down
    void pressEvent(QMouseEvent* event);

    /// handles when the mouse moves
    void moveEvent(QMouseEvent* event);

    /// handles when the mouse is released
    void releaseEvent(QMouseEvent* event);

private:
    /// helper that looks at  the state of various widgets and the current touch location and
    /// determines if the LeftHandMenu should move.
    bool shouldMoveMenu(QPoint pos);

    /// start point for a mouse move event
    QPoint mStartPoint;

    /// true if the touch should be handled as having entered the menu space, so it should be moving
    /// the menu horizontally as the touch moves. False if the menu should remain static.
    bool mHandleAsMenuSpace;

    /// true if left hand menu is being moved, false otherwise
    bool mMovingMenu;

    /// pointer to MainWindow
    MainWindow* mMainWindow;

    /// pointer to LeftHandMenu
    LeftHandMenu* mLeftHandMenu;

    /// pointer to MainViewPort
    MainViewport* mMainViewport;

    /// pointer to TopMenu
    TopMenu* mTopMenu;

    /// pointer to app date
    cor::LightList* mData;
};

#endif // TOUCHLISTENER_H
