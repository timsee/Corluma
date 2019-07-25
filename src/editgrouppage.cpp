/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "editgrouppage.h"

#include <QGraphicsOpacityEffect>
#include <QMessageBox>
#include <QScroller>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

#include "comm/commhue.h"
#include "utils/qt.h"

EditGroupPage::EditGroupPage(QWidget* parent, CommLayer* comm, GroupData* parser)
    : QWidget(parent), mComm(comm), mGroups(parser), mIsMood{false}, mIsRoomCurrent{false} {
    mTopMenu = new EditPageTopMenu(this);
    mTopMenu->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(mTopMenu->closeButton(), SIGNAL(clicked(bool)), this, SLOT(closePressed(bool)));
    connect(mTopMenu->resetButton(), SIGNAL(clicked(bool)), this, SLOT(resetPressed(bool)));
    connect(mTopMenu->deleteButton(), SIGNAL(clicked(bool)), this, SLOT(deletePressed(bool)));
    connect(mTopMenu->saveButton(), SIGNAL(clicked(bool)), this, SLOT(savePressed(bool)));
    connect(mTopMenu->nameEdit(),
            SIGNAL(textChanged(QString)),
            this,
            SLOT(lineEditChanged(QString)));
    connect(mTopMenu->roomCheckBox(), SIGNAL(boxChecked(bool)), this, SLOT(isRoomChecked(bool)));

    mIsRoomOriginal = false;

    mSimpleGroupWidget = new ListSimpleGroupWidget(this, cor::EListType::linear);
    mSimpleGroupWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mSimpleGroupWidget, SIGNAL(deviceClicked(QString)), this, SLOT(clickedDevice(QString)));
    QScroller::grabGesture(mSimpleGroupWidget->viewport(), QScroller::LeftMouseButtonGesture);
    mSimpleGroupWidget->setStyleSheet("background-color:rgba(33,32,32,255);");

    mRenderThread = new QTimer(this);
    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));
}

void EditGroupPage::showGroup(const QString& key,
                              const std::list<cor::Light>& groupDevices,
                              const std::list<cor::Light>& devices,
                              bool isMood,
                              bool isRoom) {
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
    update();
}

void EditGroupPage::updateDevices(const std::list<cor::Light>& checkedDevices,
                                  const std::list<cor::Light>& devices) {
    mSimpleGroupWidget->updateDevices(devices,
                                      cor::EWidgetType::full,
                                      EOnOffSwitchState::hidden,
                                      true,
                                      false);
    mSimpleGroupWidget->setCheckedDevices(checkedDevices);
}

// ----------------------------
// Slots
// ----------------------------

void EditGroupPage::deletePressed(bool) {
    QMessageBox::StandardButton reply;
    QString text = "Delete the " + mNewName + " group?";
    reply = QMessageBox::question(this, "Delete?", text, QMessageBox::Yes | QMessageBox::No);
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
        reply = QMessageBox::question(this,
                                      "Changes",
                                      "Changes were made, save the changes?",
                                      QMessageBox::Yes | QMessageBox::No);
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
        reply = QMessageBox::question(this,
                                      "Changes",
                                      "Changes were made, save the changes?",
                                      QMessageBox::Yes | QMessageBox::No);
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


void EditGroupPage::resize() {
    auto yPos = 0;
    mTopMenu->setGeometry(0, yPos, this->width(), this->height() / 5);
    yPos += mTopMenu->height();

    mSimpleGroupWidget->setGeometry(0, yPos, this->width(), this->height() * 4 / 5);

    mSimpleGroupWidget->resizeWidgets();
}

void EditGroupPage::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(48, 47, 47)));
}

void EditGroupPage::lineEditChanged(const QString& newText) {
    mNewName = newText;

    if (checkForChanges()) {
        mTopMenu->saveButton()->setEnabled(true);
    } else {
        mTopMenu->saveButton()->setEnabled(false);
    }
}

void EditGroupPage::renderUI() {}

// ----------------------------
// Private
// ----------------------------

void EditGroupPage::saveChanges() {
    //---------------------------------
    // check if group has a name, at least one device, and all valid devices.
    //---------------------------------
    bool nameIsValid = false;
    if (!mNewName.isEmpty() && (mNewName != "zzzAVAIALBLE_DEVICES")) {
        nameIsValid = true;
    } else {
        qDebug() << "WARNING: attempting to save a group without a valid name";
    }

    //--------------------------------
    // Create a list of devices
    //--------------------------------
    std::list<cor::Light> newDevices = mSimpleGroupWidget->checkedDevices();

    bool devicesAreValid = true;
    if (!newDevices.empty()) {
        for (auto& device : newDevices) {
            if (device.controller() == "" || device.index == 0) {
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
                            msgBox.setText("Trying to save the light " + device.name
                                           + " to multiple rooms.");
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
        for (const auto& device : newDevices) {
            if (device.commType() == ECommType::hue) {
                HueLight hue = mComm->hue()->hueLightFromLight(device);
                hueLights.push_back(hue);
            }
        }
        if (!hueLights.empty()) {
            for (const auto& bridge : mComm->hue()->bridges().itemVector()) {
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


    if (mNewName != mOriginalName) {
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

bool EditGroupPage::shouldSetChecked(const cor::Light& device,
                                     const std::list<cor::Light>& groupDevices) {
    for (const auto& collectionDevice : groupDevices) {
        if (device.uniqueID() == collectionDevice.uniqueID()) {
            return true;
        }
    }
    return false;
}

void EditGroupPage::clickedDevice(const QString&) {
    // call the highlight button
    if (checkForChanges()) {
        mTopMenu->saveButton()->setEnabled(true);
    } else {
        mTopMenu->saveButton()->setEnabled(false);
    }
}

void EditGroupPage::pushIn() {
    moveWidget(this,
               QPoint(int(this->parentWidget()->width() * 0.125f),
                      int(-1 * this->parentWidget()->height())),
               QPoint(int(this->parentWidget()->width() * 0.125f),
                      int(this->parentWidget()->height() * 0.125f)));
    this->raise();
    this->setVisible(true);
    mRenderThread->start(mRenderInterval);
}

void EditGroupPage::pushOut() {
    moveWidget(this,
               QPoint(int(this->parentWidget()->width() * 0.125f),
                      int(this->parentWidget()->height() * 0.125f)),
               QPoint(int(this->parentWidget()->width() * 0.125f),
                      int(-1 * this->parentWidget()->height())));
    mRenderThread->stop();
}
