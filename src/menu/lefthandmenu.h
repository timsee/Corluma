#ifndef LEFTHANDMENU_H
#define LEFTHANDMENU_H

#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

#include "comm/commlayer.h"
#include "cor/lightlist.h"
#include "data/appdata.h"
#include "globalstatewidget.h"
#include "lefthandbutton.h"
#include "standardlightsmenu.h"
#include "timeoutbutton.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The LeftHandMenu class is a left hand drawer that can either be pulled in or out when the
 * app has a portrait aspect ratio. If it is a landscape aspect ratio, the drawer is permanently
 * out. This menu contains buttons for all the main pages of the app, as well as the ability to
 * choose specific lights to control.
 */
class LeftHandMenu : public QWidget {
    Q_OBJECT
public:
    /// constructor
    LeftHandMenu(bool alwaysOpen,
                 cor::LightList* selectedLights,
                 CommLayer* comm,
                 cor::LightList* lights,
                 AppData* appData,
                 QWidget* parent);

    /// change the height of each row
    void changeRowHeight(int height) { mRowHeight = height; }

    /// resize programmatically
    void resize();

    /// push in the menu for display
    void pushIn();

    /// move out the menu from display
    void pushOut();

    /// true if in, false otherwise
    bool isIn() { return mIsIn; }

    /// true if menu is always open due to the app being in landscape, false otherwise
    bool alwaysOpen() { return mAlwaysOpen; }

    /// used during complete reloads, this deletes all existing widgets
    void clearWidgets();

    /// update the lights in the scroll area
    void updateLights();

    /// true if the menu is currently moving
    bool isMoving() const noexcept { return mIsMoving; }

    /// sets the flag for whether or not the menu is moving
    void isMoving(bool moving) { mIsMoving = moving; }

    /// getter for how many pixels of the menu are currently showing
    int showingWidth();

    /// update the state of the timeout button
    void updateTimeoutButton(bool timeoutEnabled, std::uint32_t timeoutValue);

    /// certain buttons are only enabled if a light is connected. this enables and disables those
    /// buttons.
    void enableButtons(bool enable);

signals:
    /// signals when a page button is pressed
    void pressedButton(EPage);

    /// called when the number of lights selected changes
    void changedLightCount();

    /// signals to create a new group from the left hand menu
    void createNewGroup();

public slots:

    /// called when a button is pressed
    void buttonPressed(EPage);

private slots:

    /// select all lights is toggled from a group
    void selectAllToggled(const cor::UUID&, bool);

    /// a light is clicked from the LeftHandMenuScrollArea
    void lightClicked(const cor::LightID& lightID);

    /// used to render UI updates
    void renderUI();

    /// handles when the new group button is pressed
    void newGroupButtonPressed();

private:
    /// true if menu should be always open for landscape orientation, false otherwise
    bool mAlwaysOpen;

    /// certain buttons that require any lights to be connected are disabled by default. This is set
    /// to false if they are disabled, and true if they are enabled.
    bool mButtonsEnabled;

    /// returns a pointer to the currently selected button
    QWidget* selectedButton();

    /// true if menu is in, false otherwise
    bool mIsIn;

    /// point to start the widget at
    QPoint mStartPoint;

    /// spacer for top of widget
    QWidget* mSpacer;

    /// displays the state of the selected lights.
    GlobalStateWidget* mStateWidget;

    /// the menu that allows the user to select lights or groups of lights
    StandardLightsMenu* mLightMenu;

    /// thread for rendering updates to the UI
    QTimer* mRenderThread;

    /// list of selected lights
    cor::LightList* mSelectedLights;

    /// pointer to comm layer
    CommLayer* mComm;

    /// list of current lights, used to update UI
    cor::LightList* mData;

    /// pointer to group data
    AppData* mAppData;

    /// lights button
    LeftHandButton* mLightsButton;

    /// single color button
    LeftHandButton* mSingleColorButton;

    /// multi color button
    LeftHandButton* mMultiColorButton;

    /// mood button
    LeftHandButton* mMoodButton;

    /// settings button
    LeftHandButton* mSettingsButton;

#ifdef USE_EXPERIMENTAL_FEATURES
    /// timeout button
    TimeoutButton* mTimeoutButton;
#endif // USE_EXPERIMENTAL_FEATURES

    /// last time lights were rendered.
    QTime mLastRenderTime;

    /// height of each row, used for buttons.
    int mRowHeight;

    /// true if moving, false otherwise
    bool mIsMoving;
};

#endif // LEFTHANDMENU_H
