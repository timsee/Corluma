/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include <QMessageBox>

#include "editmoodpage.h"

void EditMoodPage::showMood(const cor::Mood& mood, const std::vector<cor::Light>& lights) {
    mOriginalMood = mood;
    mNewMood = mood;

    mTopMenu->nameEdit()->setText(mOriginalMood.name());
    mTopMenu->saveButton()->setEnabled(false);
    mTopMenu->helpLabel()->setText("Edit the Mood...");
    mTopMenu->roomCheckBox()->setVisible(false);

    updateDevices(mood.lights(), lights);
    update();
}

bool EditMoodPage::checkForChanges() {
    if (mNewMood.name().isEmpty()) {
        return false;
    }

    if (mNewMood.name() != mOriginalMood.name()) {
        return true;
    }

    // check all checked devices are part of original group
    for (const auto& checkedDevice : mSimpleGroupWidget->checkedDevices()) {
        bool foundDevice = false;
        for (const auto& device : mOriginalMood.lights()) {
            if (checkedDevice.uniqueID() == device.uniqueID()) {
                foundDevice = true;
            }
        }
        if (!foundDevice) {
            return true;
        }
    }

    // check all given devices are checked
    for (const auto& device : mOriginalMood.lights()) {
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

bool EditMoodPage::saveChanges() {
    //---------------------------------
    // check if group has a name, at least one device, and all valid devices.
    //---------------------------------
    QString newName = mNewMood.name();
    QString originalName = mOriginalMood.name();

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
            if (device.isValid()) {
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
    // Save if passing checks
    //---------------------------------
    // remove group
    mGroups->removeGroup(mOriginalMood.uniqueID());

    // make a new mood
    cor::Mood mood(mGroups->generateNewUniqueKey(), newName, newDevices);
    mGroups->saveNewMood(mood);

    return true;
}


void EditMoodPage::deletePressed(bool) {
    QMessageBox::StandardButton reply;
    auto name = mOriginalMood.name();
    QString text("Delete the " + name + " mood?");
    reply = QMessageBox::question(this, "Delete?", text, QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        mGroups->removeGroup(mOriginalMood.uniqueID());
        // delete from hue bridge, if applicable.
        mComm->deleteHueGroup(name);
        emit pressedClose();
    }
}
