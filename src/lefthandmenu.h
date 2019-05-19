#ifndef LEFTHANDMENU_H
#define LEFTHANDMENU_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>

#include "lefthandbutton.h"
#include "cor/devicelist.h"
#include "groupdata.h"
#include "comm/commlayer.h"
#include "cor/devicelist.h"
#include "listroomwidget.h"
#include "addnewgroupbutton.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The LeftHandMenu class is a left hand drawer that can either be pulled in or out when the app
 *        has a portrait aspect ratio. If it is a landscape aspect ratio, the drawer is permanently out.
 *        This menu contains buttons for all the main pages of the app, as well as the ability to choose
 *        specific lights to control.
 */
class LeftHandMenu : public QWidget
{
    Q_OBJECT
public:
    /// constructor
    LeftHandMenu(cor::DeviceList *selectedLights, CommLayer *comm, cor::DeviceList *lights, GroupData *groups, QWidget *parent);

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

    /// set the menu to be either always open or to be able to be pulled in and out
    void alwaysOpen(bool alwaysOpen) { mAlwaysOpen = alwaysOpen; }

signals:
    /// signals when a page button is pressed
    void pressedButton(EPage);

    /// called when the number of lights selected changes
    void changedDeviceCount();

    /// signals to create a new group from the left hand menu
    void createNewGroup();

public slots:

    /// called when the number of lights selected changed
    void deviceCountChanged();

    /// called when a button is pressed
    void buttonPressed(EPage);

private slots:

    /// called when a light is clicked
    void lightClicked(const QString&, const QString&);

    /// called when a group is selected or deselected
    void groupSelected(const QString& key, bool shouldSelect);

    /// called when buttons should be shown or hidden
    void shouldShowButtons(const QString&, bool);

    /// called when a group is changed
    void changedGroup(const QString&);

    /// used to render UI updates
    void renderUI();

    /// handles when the new group button is pressed
    void newGroupButtonPressed();

private:

    /// true if menu should be always open for landscape orientation, false otherwise
    bool mAlwaysOpen;

    /// gathers all light groups, as displayed in the UI
    std::list<cor::Group> gatherAllUIGroups();

    /*!
     * \brief updateDataGroupInUI using the new cor::LightGroup, update the UI assets with up-to-date light info. This function matches the dataGroup group
     *        to all UI groups that match it, then takes the up to date version from the allDevices list to display that info
     * \param dataGroup the group to update in the UI
     * \param uiGroups all UI groups
     */
    void updateDataGroupInUI(const cor::Group& dataGroup, const std::list<cor::Group>& uiGroups);

    /*!
     * \brief initRoomsWidget constructor helper for making a ListRoomWidget
     * \param group group of lights for collection
     * \param key key for collection
     * \return pointer to the newly created ListDevicesGroupWidget
     */
    ListRoomWidget* initRoomsWidget(const cor::Group& group, const QString& key);

    /// resize teh room widgets
    int resizeRoomsWidgets();

    /// update the lights in the room widgets
    void updateLights();

    /// creates a miscellaneous group for all groups not stated in standard rooms
    cor::Group makeMiscellaneousGroup(const std::list<cor::Group>& roomList);

    /// true if menu is in, false otherwise
    bool mIsIn;

    /// size of the parent
    QSize mParentSize;

    /// point to start the widget at
    QPoint mStartPoint;

    /// scroll area for the widget
    QScrollArea *mScrollArea;

    /// main widget
    QWidget *mWidget;

    /// spacer for top of widget
    QWidget *mSpacer;

    /// thread for rendering updates to the UI
    QTimer *mRenderThread;

    /// list of selected lights
    cor::DeviceList *mSelectedLights;

    /// palette that shows the currently selected devices
    cor::LightVectorWidget *mMainPalette;

    /// vector room widgets
    std::vector<ListRoomWidget*> mRoomWidgets;

    /// pointer to comm layer
    CommLayer *mComm;

    /// list of current lights, used to update UI
    cor::DeviceList *mData;

    /// pointer to group data
    GroupData *mGroups;

    /// single color button
    LeftHandButton *mSingleColorButton;

    /// multi color button
    LeftHandButton *mMultiColorButton;

    /// mood button
    LeftHandButton *mMoodButton;

    /// settings button
    LeftHandButton *mSettingsButton;

    /// button to add a new group
    AddNewGroupButton *mNewGroupButton;

    /// update the single color button based off of what is selected
    void updateSingleColorButton();

    /// stored values for last devices to prevent unnecessary renders
    std::list<cor::Light> mLastDevices;

    /// stores number of rooms to check if resize is needed
    std::size_t mNumberOfRooms;

    /// stores the number of shown lights to check if resize is needed
    std::size_t mNumberOfShownLights;

    /// used to skip rendering while scrolling
    int mLastScrollValue;
};

#endif // LEFTHANDMENU_H
