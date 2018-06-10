/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
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

EditGroupPage::EditGroupPage(QWidget *parent, CommLayer* comm, DataLayer* data, GroupsParser *parser) : QWidget(parent), mComm(comm), mGroups(parser) {

    mTopMenu = new EditPageTopMenu(this);
    mTopMenu->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(mTopMenu->closeButton(),  SIGNAL(clicked(bool)),       this, SLOT(closePressed(bool)));
    connect(mTopMenu->resetButton(),  SIGNAL(clicked(bool)),       this, SLOT(resetPressed(bool)));
    connect(mTopMenu->deleteButton(), SIGNAL(clicked(bool)),       this, SLOT(deletePressed(bool)));
    connect(mTopMenu->saveButton(),   SIGNAL(clicked(bool)),       this, SLOT(savePressed(bool)));
    connect(mTopMenu->nameEdit(),     SIGNAL(textChanged(QString)), this, SLOT(lineEditChanged(QString)));
    connect(mTopMenu->roomCheckBox(), SIGNAL(boxChecked(bool)),    this, SLOT(isRoomChecked(bool)));

    mIsRoomOriginal = false;

    mDevicesList = new cor::ListWidget(this);
    mDevicesList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mDevicesList->setContentsMargins(0,0,0,0);
    QScroller::grabGesture(mDevicesList->viewport(), QScroller::LeftMouseButtonGesture);

    mScrollAreaWidget = new ListEditWidget(this, mComm, data);
    connect(mScrollAreaWidget, SIGNAL(deviceClicked(QString, QString)), this, SLOT(clickedDevice(QString, QString)));
    mScrollAreaWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mDevicesList->addWidget(mScrollAreaWidget);

    mRenderThread = new QTimer(this);
    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));
    mScrollAreaWidget->setStyleSheet("background-color:rgba(33,32,32,255);");

    mLayout = new QVBoxLayout(this);
    mLayout->setContentsMargins(4,4,4,4);
    mLayout->setSpacing(2);

    mLayout->addWidget(mTopMenu, 3);
    mLayout->addWidget(mDevicesList, 8);
}

EditGroupPage::~EditGroupPage() {
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

void EditGroupPage::updateDevices(std::list<cor::Light> groupDevices, std::list<cor::Light> devices) {
    mScrollAreaWidget->updateDevices(devices, true);
    mScrollAreaWidget->setCheckedDevices(groupDevices);
    resize(false);
}

void EditGroupPage::resize(bool resizeFullWidget) {
    QSize size = qobject_cast<QWidget*>(this->parent())->size();
    if (resizeFullWidget) {
        this->setGeometry(size.width() * 0.125f,
                          size.height() * 0.125f,
                          size.width() * 0.75f,
                          size.height() * 0.75f);
    }
    mScrollAreaWidget->resize();
}


// ----------------------------
// Slots
// ----------------------------

void EditGroupPage::listDeviceWidgetClicked(QString key) {
    Q_UNUSED(key);
}

void EditGroupPage::deletePressed(bool) {
    QMessageBox::StandardButton reply;
    QString text = "Delete the " + mOriginalName + " group?";
    reply = QMessageBox::question(this, "Delete?", text,
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
      mGroups->removeGroup(mOriginalName);
      // delete from hue bridge, if applicable.
      mComm->deleteHueGroup(mOriginalName);
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
    mScrollAreaWidget->setCheckedDevices(mOriginalDevices);
}

void EditGroupPage::savePressed(bool) {
    if (checkForChanges()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Changes", "Changes were made, save the changes?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
          saveChanges();
          // make new original devices
          mOriginalDevices = mScrollAreaWidget->checkedDevices();
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


void EditGroupPage::showEvent(QShowEvent *event) {
    Q_UNUSED(event);

    mRenderThread->start(mRenderInterval);
}


void EditGroupPage::hideEvent(QHideEvent *event) {
    Q_UNUSED(event);

    mRenderThread->stop();
}

void EditGroupPage::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(48, 47, 47)));
}


void EditGroupPage::resizeEvent(QResizeEvent *) {
    mDevicesList->setMaximumWidth(this->size().width());
    mDevicesList->setMaximumHeight(this->size().height());
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
    if (mNewName.size() > 0
            && !(mNewName.compare("zzzAVAIALBLE_DEVICES") == 0)) {
            nameIsValid = true;
    } else {
        qDebug() << "WARNING: attempting to save a group without a valid name";
    }

    //--------------------------------
    // Create a list of devices
    //--------------------------------
    std::list<cor::Light> newDevices = mScrollAreaWidget->checkedDevices();

    bool devicesAreValid = true;
    if (newDevices.size() > 0) {
        for (auto& device : newDevices) {
            if (device.controller().compare("") == 0
                    || device.index() == 0) {
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
    // Save if passing checks
    //---------------------------------
    if (mIsMood) {
        mGroups->removeGroup(mOriginalName);
        mGroups->saveNewMood(mNewName, newDevices);
    } else {
        mGroups->removeGroup(mOriginalName);
        mGroups->saveNewCollection(mNewName, newDevices, mIsRoomCurrent);

        // convert any group devices to Hue Lights, if applicable.
        std::list<HueLight> hueLights;
        for (auto device : newDevices) {
            if (device.commType() == ECommType::hue) {
                HueLight hue = mComm->hue()->hueLightFromLight(device);
                hueLights.push_back(hue);
            }
        }
        if (hueLights.size() > 0) {
            // check if group already exists
            auto hueGroups = mComm->hue()->groups();
            bool groupExists = false;
            for (auto group : hueGroups) {
                if (group.name.compare(mNewName) == 0) {
                    groupExists = true;
                }
            }
            if (groupExists) {
                mComm->updateHueGroup(mNewName, hueLights);
            } else {
                mComm->hue()->createGroup(mNewName, hueLights, mIsRoomCurrent);
            }
        }
    }
}

bool EditGroupPage::checkForChanges() {
    if (!(mNewName.compare(mOriginalName) == 0)) {
        return true;
    }

    if (mIsRoomCurrent != mIsRoomOriginal) {
        return true;
    }

    // check all checked devices are part of original group
    for (auto&& checkedDevice : mScrollAreaWidget->checkedDevices()) {
        bool foundDevice = false;
        for (auto&& device : mOriginalDevices) {
            if (compareLight(checkedDevice, device)) {
                foundDevice = true;
            }
        }
        if (!foundDevice) {
            return true;
        }
    }

    // check all given devices are checked
    for (auto&& device : mOriginalDevices) {
        for (uint32_t i = 0; i < mScrollAreaWidget->widgets().size(); ++i) {
            ListDeviceWidget *existingWidget = qobject_cast<ListDeviceWidget*>(mScrollAreaWidget->widgets()[i]);
            if (compareLight(existingWidget->device(), device)) {
                if (!existingWidget->checked())  {
                    return true;
                }
            }
        }
    }

    return false;
}

bool EditGroupPage::shouldSetChecked(const cor::Light& device, const std::list<cor::Light>& groupDevices) {
    for (auto&& collectionDevice : groupDevices) {
        if (compareLight(device, collectionDevice)) {
            return true;
        }
    }
    return false;
}

void EditGroupPage::clickedDevice(QString key, QString deviceName) {
    Q_UNUSED(key);
    Q_UNUSED(deviceName);
   // qDebug() << " device clicked " << key << " vs" << deviceName;

    // call the highlight button
    if (checkForChanges()) {
        mTopMenu->saveButton()->setEnabled(true);
    } else {
        mTopMenu->saveButton()->setEnabled(false);
    }
}
