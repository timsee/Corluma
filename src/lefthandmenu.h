#ifndef LEFTHANDMENU_H
#define LEFTHANDMENU_H

#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

#include "comm/commlayer.h"
#include "cor/lightlist.h"
#include "cor/widgets/lightvectorwidget.h"
#include "data/groupdata.h"
#include "lefthandbutton.h"
#include "lefthandmenuscrollarea.h"
#include "lefthandmenutoplightwidget.h"

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
                 GroupData* groups,
                 QWidget* parent);

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

    /// called when the number of lights selected changed
    void lightCountChanged();

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

    /// changes the state of the LeftHandMenu to showing parent groups
    void changeStateToParentGroups();

    /// changes the state of the LeftHandMenu to showing subgroups
    void changeStateToSubgroups();

    /// select all lights is toggled from a group
    void selectAllToggled(QString, bool);

    /// a light is clicked from the LeftHandMenuScrollArea
    void lightClicked(const QString& lightID);

    /// called when a group is selected or deselected
    void groupSelected(std::uint64_t key, bool shouldSelect);

    /// called when buttons should be shown or hidden
    void shouldShowButtons(std::uint64_t, bool);

    /// called when a parent group is changed
    void parentGroupClicked(std::uint64_t ID);

    /// transition from the subgroups to the lights, by showing the lights for a specific group
    void showSubgroupLights(std::uint64_t);

    /// used to render UI updates
    void renderUI();

    /// handles when the new group button is pressed
    void newGroupButtonPressed();

protected:
    /// handles when the mouse releases from the LeftHandMenu.
    void mouseReleaseEvent(QMouseEvent*) override;

private:
    /// true if menu should be always open for landscape orientation, false otherwise
    bool mAlwaysOpen;

    /// returns a pointer to the currently selected button
    QWidget* selectedButton();

    /// highlights the lights and groups
    void highlightLightsAndGroups();

    /// true if menu is in, false otherwise
    bool mIsIn;

    /// point to start the widget at
    QPoint mStartPoint;

    /// scroll area for the widget
    LeftHandMenuScrollArea* mScrollArea;

    /// top widget that shows the parent group and subgroup, if either is necessary
    LeftHandMenuTopLightWidget* mScrollTopWidget;

    /// spacer for top of widget
    QWidget* mSpacer;

    /// thread for rendering updates to the UI
    QTimer* mRenderThread;

    /// list of selected lights
    cor::LightList* mSelectedLights;

    /// palette that shows the currently selected devices
    cor::LightVectorWidget* mMainPalette;

    /// pointer to comm layer
    CommLayer* mComm;

    /// list of current lights, used to update UI
    cor::LightList* mData;

    /// pointer to group data
    GroupData* mGroups;

    /// single color button
    LeftHandButton* mSingleColorButton;

    /// multi color button
    LeftHandButton* mMultiColorButton;

    /// mood button
    LeftHandButton* mMoodButton;

    /// settings button
    LeftHandButton* mSettingsButton;

    /// update the single color button based off of what is selected
    void updateSingleColorButton();

    /// stored values for last devices to prevent unnecessary renders
    std::vector<cor::Light> mLastDataLights;

    /// last time lights were rendered.
    QTime mLastRenderTime;

    /// used to skip rendering while scrolling
    int mLastScrollValue;

    /// true if moving, false otherwise
    bool mIsMoving;
};

#endif // LEFTHANDMENU_H
