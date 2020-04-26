/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include <QMessageBox>

#include "oldeditgrouppage.h"

void OldEditGroupPage::showGroup(const cor::Group& group,
                                 const std::vector<cor::Light>& groupLights,
                                 const std::vector<cor::Light>& lights,
                                 bool isRoom) {
    mOriginalGroup = group;
    mNewName = group.name();
    mNewLights = groupLights;
    mOriginalLights = groupLights;
    mIsRoomOriginal = isRoom;
    mIsRoom = isRoom;

    mTopMenu->nameEdit()->setText(mOriginalGroup.name());
    mTopMenu->saveButton()->setEnabled(false);
    mTopMenu->helpLabel()->setText("Edit the Group...");
    mTopMenu->roomCheckBox()->setVisible(true);
    mTopMenu->roomCheckBox()->setChecked(mIsRoom);
    // only have this option enabled for new groups
    mTopMenu->roomCheckBox()->setEnabled((group.name() == "New Group"));

    updateDevices(groupLights, lights);
    update();
}

bool OldEditGroupPage::checkForChanges() {
    if (mNewName.isEmpty()) {
        return false;
    }

    if (mNewName != mOriginalGroup.name()) {
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

void OldEditGroupPage::isRoomChecked(bool checked) {
    mIsRoom = checked;

    if (checkForChanges()) {
        mTopMenu->saveButton()->setEnabled(true);
    } else {
        mTopMenu->saveButton()->setEnabled(false);
    }
}

bool OldEditGroupPage::saveChanges() {
    //---------------------------------
    // check if group has a name, at least one device, and all valid devices.
    //---------------------------------
    QString originalName = mOriginalGroup.name();

    bool nameIsValid = false;
    if (!mNewName.isEmpty() && (mNewName != "zzzAVAIALBLE_DEVICES")) {
        nameIsValid = true;
    } else {
        qDebug() << "WARNING: attempting to save a group without a valid name";
    }

    //--------------------------------
    // Create a list of devices
    //--------------------------------
    std::vector<cor::Light> newDevices = mSimpleGroupWidget->checkedDevices();
    bool devicesAreValid = true;
    if (!newDevices.empty()) {
        for (const auto& device : newDevices) {
            if (!device.isValid()) {
                devicesAreValid = false;
            }
        }
    } else {
        devicesAreValid = false;
    }

    if (!nameIsValid || !devicesAreValid) {
        qDebug() << "Not saving this group: " << mNewName;
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
            for (const auto& collection : mGroups->rooms()) {
                // ignore the existing room and non-rooms
                if (collection.name() != mNewName) {
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
    mGroups->removeGroup(mOriginalGroup.uniqueID());

    // split hues from group
    std::vector<QString> nonHueLights;
    for (const auto& light : newDevices) {
        if (light.protocol() != EProtocolType::hue) {
            nonHueLights.push_back(light.uniqueID());
        }
    }

    // make a group to save into app data that isn't hue
    if (mIsRoom) {
        cor::Group nonHueRoom(mGroups->generateNewUniqueKey(),
                              mNewName,
                              cor::EGroupType::room,
                              nonHueLights);
        mGroups->saveNewGroup(nonHueRoom);
    } else {
        cor::Group nonHueGroup(mGroups->generateNewUniqueKey(),
                               mNewName,
                               cor::EGroupType::group,
                               nonHueLights);
        mGroups->saveNewGroup(nonHueGroup);
    }

    // convert any group devices to Hue Lights, if applicable.
    std::vector<HueMetadata> hueLights;
    for (const auto& device : newDevices) {
        if (device.commType() == ECommType::hue) {
            HueMetadata hue = mComm->hue()->hueLightFromLight(device);
            hueLights.push_back(hue);
        }
    }
    if (!hueLights.empty()) {
        for (const auto& bridge : mComm->hue()->bridges().items()) {
            // check if group already exists
            auto hueGroups = mComm->hue()->groups(bridge);
            bool groupExists = false;
            for (const auto& group : hueGroups) {
                if (group.name() == mNewName) {
                    groupExists = true;
                    mComm->hue()->updateGroup(bridge, group, hueLights);
                }
            }
            if (!groupExists) {
                mComm->hue()->createGroup(bridge, mNewName, hueLights, mIsRoom);
            }
        }
    }

    return true;
}

void OldEditGroupPage::deletePressed(bool) {
    QMessageBox::StandardButton reply;
    auto name = mOriginalGroup.name();
    QString text("Delete the " + name + " group?");
    reply = QMessageBox::question(this, "Delete?", text, QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        mGroups->removeGroup(mOriginalGroup.uniqueID());
        // delete from hue bridge, if applicable.
        mComm->deleteHueGroup(name);
        emit pressedClose();
    }
}
