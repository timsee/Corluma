/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include <QMessageBox>

#include "editgrouppage.h"

void EditGroupPage::showGroup(const cor::Group& group,
                              const std::vector<cor::Light>& groupLights,
                              const std::vector<cor::Light>& lights,
                              bool isRoom) {
    mOriginalGroup = group;
    mNewGroup = group;
    mOriginalLights = groupLights;
    mIsRoomOriginal = isRoom;
    mIsRoom = isRoom;

    mTopMenu->nameEdit()->setText(mOriginalGroup.name());
    mTopMenu->saveButton()->setEnabled(false);
    mTopMenu->helpLabel()->setText("Edit the Group...");
    mTopMenu->roomCheckBox()->setVisible(true);
    mTopMenu->roomCheckBox()->setChecked(mIsRoom);

    updateDevices(groupLights, lights);
    update();
}

bool EditGroupPage::checkForChanges() {
    if (mNewGroup.name().isEmpty()) {
        return false;
    }

    if (mNewGroup.name() != mOriginalGroup.name()) {
        return true;
    }

    if (mIsRoom != mIsRoomOriginal) {
        return true;
    }

    // check all checked devices are part of original group
    for (const auto& checkedDevice : mSimpleGroupWidget->checkedDevices()) {
        bool foundDevice = false;
        for (const auto& device : mOriginalLights) {
            if (checkedDevice.uniqueID() == device.uniqueID()) {
                foundDevice = true;
            }
        }
        if (!foundDevice) {
            return true;
        }
    }

    // check all given devices are checked
    for (const auto& device : mOriginalLights) {
        bool foundDevice = false;
        for (const auto& uiLight : mSimpleGroupWidget->checkedDevices()) {
            if (uiLight.uniqueID() == device.uniqueID()) {
                foundDevice = true;
            }
        }
        if (!foundDevice) {
            return true;
        }
    }

    return false;
}

void EditGroupPage::isRoomChecked(bool checked) {
    mIsRoom = checked;

    if (checkForChanges()) {
        mTopMenu->saveButton()->setEnabled(true);
    } else {
        mTopMenu->saveButton()->setEnabled(false);
    }
}

bool EditGroupPage::saveChanges() {
    //---------------------------------
    // check if group has a name, at least one device, and all valid devices.
    //---------------------------------
    QString newName = mNewGroup.name();
    QString originalName = mOriginalGroup.name();

    bool nameIsValid = false;
    if (!newName.isEmpty() && (newName != "zzzAVAIALBLE_DEVICES")) {
        nameIsValid = true;
    } else {
        qDebug() << "WARNING: attempting to save a group without a valid name";
    }
    // TODO: verify only a name has changed, act accordingly

    //--------------------------------
    // Create a list of devices
    //--------------------------------
    std::vector<cor::Light> newDevices = mSimpleGroupWidget->checkedDevices();
    bool devicesAreValid = true;
    if (!newDevices.empty()) {
        for (auto& device : newDevices) {
            if (device.controller() == "") {
                devicesAreValid = false;
            }
        }
    } else {
        devicesAreValid = false;
    }

    if (!nameIsValid || !devicesAreValid) {
        qDebug() << "Not saving this group: " << newName;
        qDebug() << "---------------------";
        for (auto& device : newDevices) {
            qDebug() << device;
        }
        qDebug() << "---------------------";

        // pop up warning that it isn't saving
        QMessageBox msgBox;
        msgBox.setText("Trying to save invalid group, no changes will be made.");
        msgBox.exec();
        // close edit page anyway.
        emit pressedClose();
        return false;
    }

    //---------------------------------
    // Edge case checks
    //---------------------------------
    bool passesChecks = true;

    // if its a room, check that all lights are unassigned or part of the current room
    if (mIsRoom) {
        for (const auto& device : newDevices) {
            // loop through all collections
            for (const auto& collection : mGroups->rooms().items()) {
                // ignore the existing room and non-rooms
                if (collection.name() != newName) {
                    for (const auto& lightID : collection.lights()) {
                        if (lightID == device.uniqueID()) {
                            passesChecks = false;
                            // pop up warning that it isn't saving
                            QMessageBox msgBox;
                            msgBox.setText("Trying to save the light " + device.name()
                                           + " to multiple rooms.");
                            msgBox.exec();
                        }
                    }
                }
            }
        }
    }


    if (!passesChecks) {
        return false;
    }

    //---------------------------------
    // Save if passing checks
    //---------------------------------
    // remove group
    mGroups->removeGroup(originalName);

    if (mIsRoom) {
        mGroups->saveNewRoom(newName, newDevices);
    } else {
        mGroups->saveNewGroup(newName, newDevices);
    }
    // convert any group devices to Hue Lights, if applicable.
    std::vector<HueLight> hueLights;
    for (const auto& device : newDevices) {
        if (device.commType() == ECommType::hue) {
            HueLight hue = mComm->hue()->hueLightFromLight(device);
            hueLights.push_back(hue);
        }
    }
    if (!hueLights.empty()) {
        for (const auto& bridge : mComm->hue()->bridges().items()) {
            // check if group already exists
            auto hueGroups = mComm->hue()->groups(bridge);
            bool groupExists = false;
            for (const auto& group : hueGroups) {
                if (group.name() == newName) {
                    groupExists = true;
                    mComm->hue()->updateGroup(bridge, group, hueLights);
                }
            }
            if (!groupExists) {
                mComm->hue()->createGroup(bridge, newName, hueLights, mIsRoom);
            }
        }
    }

    return true;
}
