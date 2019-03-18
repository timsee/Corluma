/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "editgrouppage.h"
#include "comm/commhue.h"

#include <QtCore>
#include <QtGui>
#include <QStyleOption>
#include <QScroller>
#include <QGraphicsOpacityEffect>
#include <QMessageBox>

EditGroupPage::EditGroupPage(QWidget *parent, CommLayer* comm, GroupData *parser) : QWidget(parent), mComm(comm), mGroups(parser) {

    mTopMenu = new EditPageTopMenu(this);
    mTopMenu->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(mTopMenu->closeButton(),  SIGNAL(clicked(bool)),        this, SLOT(closePressed(bool)));
    connect(mTopMenu->resetButton(),  SIGNAL(clicked(bool)),        this, SLOT(resetPressed(bool)));
    connect(mTopMenu->deleteButton(), SIGNAL(clicked(bool)),        this, SLOT(deletePressed(bool)));
    connect(mTopMenu->saveButton(),   SIGNAL(clicked(bool)),        this, SLOT(savePressed(bool)));
    connect(mTopMenu->nameEdit(),     SIGNAL(textChanged(QString)), this, SLOT(lineEditChanged(QString)));
    connect(mTopMenu->roomCheckBox(), SIGNAL(boxChecked(bool)),     this, SLOT(isRoomChecked(bool)));

    mIsRoomOriginal = false;

    mSimpleGroupWidget = new ListSimpleGroupWidget(this, cor::EListType::linear);
    mSimpleGroupWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mSimpleGroupWidget, SIGNAL(deviceClicked(QString)), this, SLOT(clickedDevice(QString)));
    QScroller::grabGesture(mSimpleGroupWidget->viewport(), QScroller::LeftMouseButtonGesture);
    mSimpleGroupWidget->setStyleSheet("background-color:rgba(33,32,32,255);");

    mRenderThread = new QTimer(this);
    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));

    mLayout = new QVBoxLayout(this);
    mLayout->setContentsMargins(4,4,4,4);
    mLayout->setSpacing(2);

    mLayout->addWidget(mTopMenu,     2);
    mLayout->addWidget(mSimpleGroupWidget, 8);
}

void EditGroupPage::showGroup(QString key, std::list<cor::Light> groupDevices, std::list<cor::Light> devices, bool isMood, bool isRoom) {
    mOriginalName = key;
    mNewName = key;

    mOriginalDevices = groupDevices;
    mTopMenu->nameEdit()->setText(key);
    mTopMenu->saveButton()->setEnabled(false);
    mIsMood = isMood;
    mIsRoomOriginal = isRoom;
    mIsRoomCurrent = mIsRoomOriginal;
    if (mIsMood) {
        mTopMenu->helpLabel()->setText("Edit the Mood...");
        mTopMenu->roomCheckBox()->setVisible(false);
    } else {
        mTopMenu->helpLabel()->setText("Edit the Collection...");
        mTopMenu->roomCheckBox()->setVisible(true);
        mTopMenu->roomCheckBox()->setChecked(mIsRoomOriginal);
    }
    updateDevices(groupDevices, devices);
    repaint();
}

void EditGroupPage::updateDevices(const std::list<cor::Light>& checkedDevices, const std::list<cor::Light>& devices) {
    mSimpleGroupWidget->updateDevices(devices, mSimpleGroupWidget->height() / 6, cor::EWidgetType::full, EOnOffSwitchState::hidden, true, false);
    mSimpleGroupWidget->setCheckedDevices(checkedDevices);
}

// ----------------------------
// Slots
// ----------------------------

void EditGroupPage::listDeviceWidgetClicked(QString) {}

void EditGroupPage::deletePressed(bool) {
    QMessageBox::StandardButton reply;
    QString text = "Delete the " + mNewName + " group?";
    reply = QMessageBox::question(this, "Delete?", text,
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
      mGroups->removeGroup(mNewName, mIsMood);
      // delete from hue bridge, if applicable.
      mComm->deleteHueGroup(mNewName);
      emit pressedClose();
    }
}

void EditGroupPage::closePressed(bool) {
    if (checkForChanges()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Changes", "Changes were made, save the changes?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
          saveChanges();
          mTopMenu->saveButton()->setEnabled(true);
        }
    }
    emit pressedClose();
}

void EditGroupPage::resetPressed(bool) {
    mSimpleGroupWidget->setCheckedDevices(mOriginalDevices);
}

void EditGroupPage::savePressed(bool) {
    if (checkForChanges()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Changes", "Changes were made, save the changes?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
          saveChanges();
          // make new original devices
          mOriginalDevices = mSimpleGroupWidget->checkedDevices();
          // make new original name
          mOriginalName = mNewName;
          mTopMenu->saveButton()->setEnabled(false);
        }
    }
}

void EditGroupPage::isRoomChecked(bool checked) {
      mIsRoomCurrent = checked;

      if (checkForChanges()) {
          mTopMenu->saveButton()->setEnabled(true);
      } else {
          mTopMenu->saveButton()->setEnabled(false);
      }
}

// ----------------------------
// Protected
// ----------------------------


void EditGroupPage::show() {
    mRenderThread->start(mRenderInterval);
}


void EditGroupPage::hide() {
    mRenderThread->stop();
}

void EditGroupPage::resize() {
    mSimpleGroupWidget->resizeWidgets();
}

void EditGroupPage::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(48, 47, 47)));
}


void EditGroupPage::resizeEvent(QResizeEvent *) {
    resize();
}

void EditGroupPage::lineEditChanged(const QString& newText) {
    mNewName = newText;

    if (checkForChanges()) {
        mTopMenu->saveButton()->setEnabled(true);
    } else {
        mTopMenu->saveButton()->setEnabled(false);
    }
}

void EditGroupPage::renderUI() {

}

// ----------------------------
// Private
// ----------------------------

void EditGroupPage::saveChanges() {

    //---------------------------------
    // check if group has a name, at least one device, and all valid devices.
    //---------------------------------
    bool nameIsValid = false;
    if (mNewName.size() > 0 && !(mNewName.compare("zzzAVAIALBLE_DEVICES") == 0)) {
            nameIsValid = true;
    } else {
        qDebug() << "WARNING: attempting to save a group without a valid name";
    }

    //--------------------------------
    // Create a list of devices
    //--------------------------------
    std::list<cor::Light> newDevices = mSimpleGroupWidget->checkedDevices();

    bool devicesAreValid = true;
    if (newDevices.size() > 0) {
        for (auto& device : newDevices) {
            if (device.controller().compare("") == 0
                    || device.index == 0) {
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
        return;
    }

    //---------------------------------
    // Edge case checks
    //---------------------------------
    bool passesChecks = true;

    // if its a room, check that all lights are unassigned or part of the current room
    if (mIsRoomCurrent) {
        for (const auto& device : newDevices) {
            // loop through all collections
            for (const auto& collection : mGroups->groups().itemList()) {
                // ignore the existing room and non-rooms
                if (collection.isRoom && (collection.name() != mNewName)) {
                    for (const auto& lightID : collection.lights) {
                        if (lightID == device.uniqueID()) {
                            passesChecks = false;
                            // pop up warning that it isn't saving
                            QMessageBox msgBox;
                            msgBox.setText("Trying to save the light " + device.name + " to multiple rooms.");
                            msgBox.exec();
                        }
                    }
                }
            }
        }
    }


    if (!passesChecks) {
        return;
    }

    //---------------------------------
    // Save if passing checks
    //---------------------------------
    // remove group
    mGroups->removeGroup(mOriginalName, mIsMood);

    if (mIsMood) {
        mGroups->saveNewMood(mNewName, newDevices, {});
    } else {
        mGroups->saveNewGroup(mNewName, newDevices, mIsRoomCurrent);
        // convert any group devices to Hue Lights, if applicable.
        std::list<HueLight> hueLights;
        for (auto device : newDevices) {
            if (device.commType() == ECommType::hue) {
                HueLight hue = mComm->hue()->hueLightFromLight(device);
                hueLights.push_back(hue);
            }
        }
        if (hueLights.size() > 0) {
            for (const auto& bridge : mComm->hue()->bridges().itemVector()) {
                // check if group already exists
                auto hueGroups = mComm->hue()->groups(bridge);
                bool groupExists = false;
                for (auto group : hueGroups) {
                    if (group.name().compare(mNewName) == 0) {
                        groupExists = true;
                        mComm->hue()->updateGroup(bridge, group, hueLights);
                    }
                }
                if (!groupExists) {
                    mComm->hue()->createGroup(bridge, mNewName, hueLights, mIsRoomCurrent);
                }
            }
        }
    }
}

bool EditGroupPage::checkForChanges() {
    // the data is not valid, so theres nothing to change, end early
    if (mNewName.isEmpty()) {
        return false;
    }


    if (!(mNewName.compare(mOriginalName) == 0)) {
        return true;
    }

    if (mIsRoomCurrent != mIsRoomOriginal) {
        return true;
    }

    // check all checked devices are part of original group
    for (const auto& checkedDevice : mSimpleGroupWidget->checkedDevices()) {
        bool foundDevice = false;
        for (const auto& device : mOriginalDevices) {
            if (checkedDevice.uniqueID() == device.uniqueID()) {
                foundDevice = true;
            }
        }
        if (!foundDevice) {
            return true;
        }
    }

    // check all given devices are checked
    for (const auto& device : mOriginalDevices) {
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

bool EditGroupPage::shouldSetChecked(const cor::Light& device, const std::list<cor::Light>& groupDevices) {
    for (const auto& collectionDevice : groupDevices) {
        if (device.uniqueID() == collectionDevice.uniqueID()) {
            return true;
        }
    }
    return false;
}

void EditGroupPage::clickedDevice(QString) {
   // qDebug() << " device clicked " << key << " vs" << deviceName;

    // call the highlight button
    if (checkForChanges()) {
        mTopMenu->saveButton()->setEnabled(true);
    } else {
        mTopMenu->saveButton()->setEnabled(false);
    }
}
