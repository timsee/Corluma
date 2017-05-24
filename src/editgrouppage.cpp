/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "editgrouppage.h"
#include "ui_editcollectionpage.h"

#include <QtCore>
#include <QtGui>
#include <QStyleOption>
#include <QScroller>
#include <QGraphicsOpacityEffect>
#include <QMessageBox>

EditGroupPage::EditGroupPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EditCollectionPage) {
    ui->setupUi(this);

    connect(ui->closeButton, SIGNAL(clicked(bool)), this, SLOT(closePressed(bool)));
    connect(ui->resetButton, SIGNAL(clicked(bool)), this, SLOT(resetPressed(bool)));
    connect(ui->deleteButton, SIGNAL(clicked(bool)), this, SLOT(deletePressed(bool)));
    connect(ui->saveButton, SIGNAL(clicked(bool)), this, SLOT(savePressed(bool)));

    connect(ui->nameEdit, SIGNAL(textEdited(QString)), this, SLOT(lineEditChanged(QString)));

    connect(ui->deviceList, SIGNAL(itemPressed(QListWidgetItem*)), this, SLOT(deviceListClicked(QListWidgetItem*)));

    QScroller::grabGesture(ui->deviceList->viewport(), QScroller::LeftMouseButtonGesture);

    mRenderThread = new QTimer(this);
    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));
}

EditGroupPage::~EditGroupPage() {
    delete ui;
}

void EditGroupPage::showGroup(QString key, std::list<SLightDevice> groupDevices, std::list<SLightDevice> devices, bool isMood) {
    mOriginalName = key;
    mNewName = key;
    mOriginalDevices = groupDevices;
    ui->nameEdit->setText(key);
    ui->saveButton->setEnabled(false);
    mIsMood = isMood;
    if (mIsMood) {
        ui->helpLabel->setText("Edit the Mood...");
    } else {
        ui->helpLabel->setText("Edit the Collection...");
    }
    updateDevices(groupDevices, devices);
    repaint();
}

void EditGroupPage::updateDevices(std::list<SLightDevice> groupDevices, std::list<SLightDevice> devices) {

    for (auto&& device : devices) {
        bool widgetFound = false;
        for (auto&& widget : mWidgets) {
            SLightDevice widgetDevice = widget->device();
            if (compareLightDevice(device, widgetDevice)) {
                widgetFound = true;
                widget->updateWidget(device, mData->colorGroup(device.colorGroup));
                widget->setHighlightChecked(shouldSetChecked(device, groupDevices));
            }
        }

        // no widget found for this device,
        if (!widgetFound) {
            QString name;
            bool shouldAddWidget = true;
            if (device.type == ECommType::eHue) {
                SHueLight hue = mComm->hueLightFromLightDevice(device);
                if (hue.name.compare("") == 0) shouldAddWidget = false;
                name = hue.name;
            } else {
                name = device.name;
            }

            if (shouldAddWidget) {
                ListDeviceWidget *widget = new ListDeviceWidget(device, name, mData->colorGroup(device.colorGroup));
                widget->setHighlightChecked(shouldSetChecked(device, groupDevices));

                int index = ui->deviceList->count();
                ui->deviceList->addItem(widget->key());
                ui->deviceList->setItemWidget(ui->deviceList->item(index), widget);
                mWidgets.push_back(widget);
            }
        }
    }
    ui->deviceList->sortItems();
}

void EditGroupPage::resize() {
    QSize size = qobject_cast<QWidget*>(this->parent())->size();
    this->setGeometry(size.width() * 0.125f, size.height() * 0.125f, size.width() * 0.75f, size.height() * 0.75f);
    QSize widgetSize(this->width(), this->height() / 8);
    for (int i = 0; i < ui->deviceList->count(); ++i) {
        QListWidgetItem *item = ui->deviceList->item(i);
        ListDeviceWidget *widget = qobject_cast<ListDeviceWidget*>(ui->deviceList->itemWidget(item));
        Q_ASSERT(widget);
        widget->setMinimumSize(widgetSize);
        widget->setMaximumSize(widgetSize);
        item->setSizeHint(widgetSize);
    }
}




// ----------------------------
// Slots
// ----------------------------


void EditGroupPage::deviceListClicked(QListWidgetItem* item) {
     ListDeviceWidget *widget = qobject_cast<ListDeviceWidget*>(ui->deviceList->itemWidget(item));
     widget->setHighlightChecked(!widget->checked());

     if (checkForChanges()) {
         ui->saveButton->setEnabled(true);
     } else {
         ui->saveButton->setEnabled(false);
     }
}

void EditGroupPage::deletePressed(bool) {
    QMessageBox::StandardButton reply;
    QString text = "Delete the " + mOriginalName + " group?";
    reply = QMessageBox::question(this, "Delete?", text,
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
      mGroups->removeGroup(mOriginalName);
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
          ui->saveButton->setEnabled(true);
        }
    }
    emit pressedClose();
}

void EditGroupPage::resetPressed(bool) {
    for (auto&& widget : mWidgets) {
        bool deviceFound = false;
        for (auto&& device : mOriginalDevices) {
            SLightDevice widgetDevice = widget->device();
            if (compareLightDevice(device, widgetDevice)) {
                deviceFound = true;
                widget->setHighlightChecked(true);
            }
        }
        if (!deviceFound) widget->setHighlightChecked(false);
    }
}

void EditGroupPage::savePressed(bool) {
    if (checkForChanges()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Changes", "Changes were made, save the changes?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
          saveChanges();
          // make new original devices
          mOriginalDevices = createCollection();
          // make new original name
          mOriginalName = mNewName;
          ui->saveButton->setEnabled(false);
        }
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


void EditGroupPage::lineEditChanged(const QString& newText) {
    mNewName = newText;

    if (checkForChanges()) {
        ui->saveButton->setEnabled(true);
    } else {
        ui->saveButton->setEnabled(false);
    }
}

void EditGroupPage::renderUI() {
    std::list<SLightDevice> allDevices = mComm->allDevices();
   // updateDevices(std::list<SLightDevice>(), allDevices);
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
            && !(mNewName.compare("zzzAVAIALBLE_DEVICES") == 0
                || mNewName.compare("zzzAVAIALBLE_DEVICES") == 0
                || mNewName.compare("zzzAVAIALBLE_DEVICES") == 0
                || mNewName.compare("zzzAVAIALBLE_DEVICES") == 0)) {
            nameIsValid = true;
    } else {
        qDebug() << "WARNING: attempting to save a group without a valid name";
    }

    //--------------------------------
    // Create a list of devices
    //--------------------------------
    std::list<SLightDevice> newDevices;
    if (mIsMood) {
        newDevices = createMood();
    } else {
        newDevices = createCollection();
    }

    bool devicesAreValid = true;
    if (newDevices.size() > 0) {
        for (auto& device : newDevices) {
            if (device.name.compare("") == 0
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
            device.PRINT_DEBUG();
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
        mGroups->saveNewCollection(mNewName, newDevices);
    }
}

std::list<SLightDevice> EditGroupPage::createCollection() {
   std::list<SLightDevice> list;
   for (int i = 0; i < ui->deviceList->count(); ++i) {
       QListWidgetItem *item = ui->deviceList->item(i);
       ListDeviceWidget *widget = qobject_cast<ListDeviceWidget*>(ui->deviceList->itemWidget(item));
       Q_ASSERT(widget);
       if (widget->checked()) {
           list.push_back(widget->device());
       }
   }
   return list;
}

std::list<SLightDevice> EditGroupPage::createMood() {
    std::list<SLightDevice> list;
    return list;
}

bool EditGroupPage::checkForChanges() {
    if (!(mNewName.compare(mOriginalName) == 0)) {
        return true;
    }

    // check all checked devices are part of original group
    for (int i = 0; i < ui->deviceList->count(); ++i) {
        QListWidgetItem *item = ui->deviceList->item(i);
        ListDeviceWidget *widget = qobject_cast<ListDeviceWidget*>(ui->deviceList->itemWidget(item));
        Q_ASSERT(widget);
        if (widget->checked()) {
            bool foundDevice = false;
            for (auto&& device : mOriginalDevices) {
                if (compareLightDevice(widget->device(), device)) {
                    foundDevice = true;
                }
            }
            if (!foundDevice) {
                qDebug() << "all devices are part of original group";
                return true;
            }
        }
    }
    // check all given devices are checked
    for (auto&& device : mOriginalDevices) {
        for (int i = 0; i < ui->deviceList->count(); ++i) {
            QListWidgetItem *item = ui->deviceList->item(i);
            ListDeviceWidget *widget = qobject_cast<ListDeviceWidget*>(ui->deviceList->itemWidget(item));
            Q_ASSERT(widget);
            if (compareLightDevice(widget->device(), device)) {
                if (!widget->checked())  {
                    qDebug() << "all given deviecs are checked";
                    return true;
                }
            }
        }
    }

    return false;
}

bool EditGroupPage::shouldSetChecked(const SLightDevice& device, const std::list<SLightDevice>& groupDevices) {
    for (auto&& collectionDevice : groupDevices) {
        if (compareLightDevice(device, collectionDevice)) {
            return true;
        }
    }
    return false;
}
