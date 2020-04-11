/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */
#include "touchlistener.h"
#include "mainwindow.h"
#include "menu/lefthandmenu.h"
#include "topmenu.h"

TouchListener::TouchListener(MainWindow* mainWindow,
                             LeftHandMenu* leftHandMenu,
                             TopMenu* topMenu,
                             cor::LightList* data)
    : QObject(mainWindow),
      mMainWindow{mainWindow},
      mLeftHandMenu{leftHandMenu},
      mMainViewport{mainWindow->viewport()},
      mTopMenu{topMenu},
      mData{data} {}

void TouchListener::pressEvent(QMouseEvent* event) {
    mStartPoint = event->pos();
    mHandleAsMenuSpace = (mStartPoint.x() < mLeftHandMenu->width());
    mLeftHandMenu->isMoving(false);
}

void TouchListener::moveEvent(QMouseEvent* event) {
    // only care about moves when greyout is not open and lefthand menu isn't forced open
    if (!mLeftHandMenu->alwaysOpen()) {
        auto pos = event->pos();

        // if the pointer ever goes beyond the LeftHandMenu's width, make the start point the edge
        // of the LeftHandMenu
        if (pos.x() > mLeftHandMenu->width()) {
            mStartPoint = QPoint(mLeftHandMenu->width(), pos.y());
        }


        if (mHandleAsMenuSpace && pos.x() > mLeftHandMenu->width()) {
            // if a touch started in the lefthand menu space, it can move the menu. But if it
            // didn't, don't mess with it!
            pos = QPoint(mLeftHandMenu->width(), pos.y());
        } else if (!mHandleAsMenuSpace && (pos.x() <= mLeftHandMenu->width())) {
            // if a touch didn't start in the menu space but ended up in it, treat it like menu
            // space
            mStartPoint = QPoint(mLeftHandMenu->width(), pos.y());
            mHandleAsMenuSpace = true;
        }

        if (shouldMoveMenu(pos)) {
            bool isMoving =
                (std::abs(mStartPoint.x() - pos.x()) / double(mLeftHandMenu->width())) > 0.05;
            mLeftHandMenu->isMoving(isMoving);
            // get the x value based on current value
            auto xPos = pos.x();
            if (mLeftHandMenu->isIn()) {
                xPos = pos.x() - mStartPoint.x();
                if (xPos > 0) {
                    xPos = 0;
                }
            } else {
                xPos -= mLeftHandMenu->width();
            }
            mLeftHandMenu->setGeometry(xPos,
                                       mLeftHandMenu->pos().y(),
                                       mLeftHandMenu->width(),
                                       mLeftHandMenu->height());

            // partial greyout should be 100 when menu is fully out, and 0 when its fully in
            const auto greyLevel = std::uint32_t(mLeftHandMenu->showingWidth()
                                                 / double(mLeftHandMenu->width()) * 100.0);
            mMainWindow->greyOut()->partialGreyOut(greyLevel);
        }
    }
}

void TouchListener::releaseEvent(QMouseEvent* event) {
    if (!mLeftHandMenu->alwaysOpen() && !mMainWindow->isAnyWidgetAbove()) {
        mLeftHandMenu->isMoving(false);
        // check special cases for released mouse events.
        if (mLeftHandMenu->isIn() && !mHandleAsMenuSpace
            && (event->pos().x() > mLeftHandMenu->width())) {
            // hide the menu if its in and the user clicks outside of it
            mMainWindow->pushOutLeftHandMenu();
        } else if (mLeftHandMenu->isIn()
                   && mLeftHandMenu->showingWidth() < (mLeftHandMenu->width() * 2 / 3)) {
            // if the menu is in and less than 2/3 of it is showing, count that as
            // intending to close the menu.
            mMainWindow->pushOutLeftHandMenu();
        } else if (!mLeftHandMenu->isIn()
                   && mLeftHandMenu->showingWidth() > (mLeftHandMenu->width() * 1 / 3)) {
            // if the menu is in and more than 1/3 of it is showing, count that as intending to
            // open the menu.
            mMainWindow->pushInLeftHandMenu();
        } else {
            // no special cases that change the menu state have ocurred, so just reset the menu to
            // where its supposed to be (if its in, make it fully in, if its out, make it fully out)
            if (mLeftHandMenu->isIn()) {
                mMainWindow->pushInLeftHandMenu();
            } else {
                mMainWindow->pushOutLeftHandMenu();
            }
        }
    }
}

bool TouchListener::shouldMoveMenu(QPoint pos) {
    if (mMainWindow->isAnyWidgetAbove()) {
        return false;
    }
    bool inRange = (pos.x() < mLeftHandMenu->size().width());
    bool startNearLeftHand = (mStartPoint.x() < (mMainWindow->size().width() * 0.2));
    if (mLeftHandMenu->isIn()) {
        return inRange;
    }
    return inRange && startNearLeftHand;
}
