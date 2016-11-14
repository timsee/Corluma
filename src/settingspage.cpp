/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

#include "settingspage.h"
#include "ui_settingspage.h"
#include "commhue.h"
#include "lightslistwidget.h"

#include <QDebug>
#include <QSignalMapper>

#include <algorithm>

SettingsPage::SettingsPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsPage) {
    ui->setupUi(this);

    mSettings = new QSettings;

    // setup sliders
    mSliderSpeedValue = 425;
    ui->speedSlider->slider->setRange(1, 1000);
    ui->speedSlider->slider->setValue(mSliderSpeedValue);
    ui->speedSlider->setSliderHeight(0.5f);
    ui->speedSlider->slider->setTickPosition(QSlider::TicksBelow);
    ui->speedSlider->slider->setTickInterval(100);

    ui->timeoutSlider->slider->setRange(0,240);
    ui->timeoutSlider->slider->setValue(120);
    ui->timeoutSlider->setSliderHeight(0.5f);
    ui->timeoutSlider->slider->setTickPosition(QSlider::TicksBelow);
    ui->timeoutSlider->slider->setTickInterval(40);

    QSignalMapper *commTypeMapper = new QSignalMapper(this);

#ifndef MOBILE_BUILD
    ui->serialButton->setCheckable(true);
    connect(ui->serialButton, SIGNAL(clicked(bool)), commTypeMapper, SLOT(map()));
    commTypeMapper->setMapping(ui->serialButton, (int)ECommType::eSerial);
    ui->connectionListLabel->setHidden(true);
#else
    // hide PC-specific elements
    ui->serialButton->setHidden(true);
    ui->connectionListLabel->setHidden(true);
#endif //MOBILE_BUILD

    ui->httpButton->setCheckable(true);
    connect(ui->httpButton, SIGNAL(clicked(bool)), commTypeMapper, SLOT(map()));
    commTypeMapper->setMapping(ui->httpButton, (int)ECommType::eHTTP);

    ui->udpButton->setCheckable(true);
    connect(ui->udpButton, SIGNAL(clicked(bool)), commTypeMapper, SLOT(map()));
    commTypeMapper->setMapping(ui->udpButton, (int)ECommType::eUDP);

    ui->hueButton->setCheckable(true);
    connect(ui->hueButton, SIGNAL(clicked(bool)), commTypeMapper, SLOT(map()));
    commTypeMapper->setMapping(ui->hueButton, (int)ECommType::eHue);

    connect(commTypeMapper, SIGNAL(mapped(int)), this, SLOT(commTypeSelected(int)));

    connect(ui->speedSlider, SIGNAL(valueChanged(int)), this, SLOT(speedChanged(int)));
    connect(ui->timeoutSlider, SIGNAL(valueChanged(int)), this, SLOT(timeoutChanged(int)));
    connect(ui->connectionList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(listClicked(QListWidgetItem*)));
    connect(ui->plusButton, SIGNAL(clicked(bool)), this, SLOT(plusButtonClicked()));
    connect(ui->minusButton, SIGNAL(clicked(bool)), this, SLOT(minusButtonClicked()));

    ui->connectionList->setSelectionMode(QAbstractItemView::MultiSelection);
    mCommType = ECommType::eUDP;
}

SettingsPage::~SettingsPage() {
    delete ui;
}


void SettingsPage::setupUI() {
   connect(mComm, SIGNAL(hueDiscoveryStateChange(int)), this, SLOT(hueDiscoveryUpdate(int)));
   connect(mComm, SIGNAL(lightStateUpdate(int, int)), this, SLOT(lightStateChanged(int, int)));
   commTypeSelected((int)mCommType);
}

void SettingsPage::updateUI(int type) {
    if (type == (int)mCommType) {
        if (mData->currentRoutine() <= ELightingRoutine::eSingleSawtoothFadeOut) {
            ui->speedSlider->setSliderColorBackground(mData->mainColor());
            ui->timeoutSlider->setSliderColorBackground(mData->mainColor());
        } else {
            ui->speedSlider->setSliderColorBackground(mData->colorsAverage(mData->currentColorGroup()));
            ui->timeoutSlider->setSliderColorBackground(mData->colorsAverage(mData->currentColorGroup()));
        }
        updateConnectionList(type);
    }
}

// ----------------------------
// Slots
// ----------------------------

void SettingsPage::speedChanged(int newSpeed) {
    mSliderSpeedValue = newSpeed;
    int finalSpeed;
    // first half of slider is going linearly between 20 FPS down to 1 FPS
    if (newSpeed < 500) {
        float percent = newSpeed / 500.0f;
        finalSpeed = (int)((1.0f - percent) * 2000.0f);
    } else {
        // second half maps 1FPS to 0.01FPS
        float percent = newSpeed - 500.0f;
        finalSpeed = (500 - percent) / 5.0f;
        if (finalSpeed < 2.0f) {
            finalSpeed = 2.0f;
        }
    }
    mData->speed((int)finalSpeed);
    mComm->sendSpeed(mData->currentDevices(), mData->speed());
}

void SettingsPage::timeoutChanged(int newTimeout) {
   mData->timeOut(newTimeout);
   mComm->sendTimeOut(mData->currentDevices(), mData->timeOut());
}

void SettingsPage::listClicked(QListWidgetItem* item) {
    SLightDevice output = CommType::stringToStruct(item->text());
    SLightDevice device;
    int controllerIndex = mComm->controllerIndexByName(output.type, output.name);
    mComm->deviceByControllerAndIndex(output.type, device, controllerIndex, output.index - 1);
    if (mData->doesDeviceExist(output)) {
        mData->removeDevice(device);
    } else {
        mData->addDevice(device);
    }
    // set the line edit to be last connection name clicked
    if (device.type == ECommType::eHTTP
            || device.type == ECommType::eUDP) {
            ui->lineEdit->setText(device.name);
    }

    ui->lineEdit->setText(device.name);
    mCurrentListString = item->text();
    // update UI
    updateUI((int)device.type);
    emit updateMainIcons();
}


void SettingsPage::highlightButton(ECommType currentCommType) {
    ui->serialButton->setChecked(false);
    ui->httpButton->setChecked(false);
    ui->udpButton->setChecked(false);
    ui->hueButton->setChecked(false);
    if (currentCommType == ECommType::eHTTP) {
        ui->httpButton->setChecked(true);
    } else if (currentCommType == ECommType::eUDP) {
        ui->udpButton->setChecked(true);
    } else if (currentCommType == ECommType::eHue) {
        ui->hueButton->setChecked(true);
    }
#ifndef MOBILE_BUILD
    else if (currentCommType == ECommType::eSerial) {
        ui->serialButton->setChecked(true);
    }
#endif //MOBILE_BUILD
}


void SettingsPage::commTypeSelected(int type) {
    if ((ECommType)type != mCommType) {
        mCommType = (ECommType)type;

        if ((ECommType)type == ECommType::eHue) {
           // theres a hue bridge already connected, show bridge MAC and all available lights
           ui->connectionListLabel->setText(QString("Hue Lights:"));
        }
#ifndef MOBILE_BUILD
        if ((ECommType)type == ECommType::eSerial) {
            ui->lineEdit->setHidden(true);
            ui->plusButton->setHidden(true);
            ui->minusButton->setHidden(true);
            ui->connectionListLabel->setHidden(false);
            ui->connectionListLabel->setText(QString("Available Serial Ports"));
        }
#endif //MOBILE_BUILD
        if ((ECommType)type == ECommType::eUDP
                ||(ECommType)type == ECommType::eHTTP ) {
            ui->lineEdit->setHidden(false);
            ui->plusButton->setHidden(false);
            ui->minusButton->setHidden(false);
            ui->connectionListLabel->setHidden(true);
        } else if ((ECommType)type == ECommType::eHue) {
            ui->lineEdit->setHidden(true);
            ui->plusButton->setHidden(true);
            ui->minusButton->setHidden(true);
            ui->connectionListLabel->setHidden(false);
        }

        highlightButton((ECommType)type);
        updateUI(type);
        emit updateMainIcons();

        // save setting to persistent memory
        mSettings->setValue(DataLayer::kCommDefaultType, QString::number((int)type));
        mSettings->sync();
    }
}


void SettingsPage::plusButtonClicked() {
    bool isSuccessful = mComm->addConnection(mCommType, ui->lineEdit->text());
    if (isSuccessful) {
        // updates the connection list
        updateConnectionList((int)mCommType);
    }
}


void SettingsPage::minusButtonClicked() {
    SLightDevice listData = CommType::stringToStruct(mCurrentListString);
    bool isSuccessful = mComm->removeConnection(mData->currentCommType(), listData.name);

    if (isSuccessful) {
        SLightDevice output = CommType::stringToStruct(mCurrentListString);
        SLightDevice device;
        int controllerIndex = mComm->controllerIndexByName(output.type, output.name);
        mComm->deviceByControllerAndIndex(output.type, device, controllerIndex, output.index - 1);
        isSuccessful = mData->removeDevice(device);

        // update the line edit text
        ui->lineEdit->setText("");
        // updates the connection list
        updateConnectionList((int)device.type);
    }
}

void SettingsPage::hueDiscoveryUpdate(int newState) {
    switch((EHueDiscoveryState)newState)
    {
        case EHueDiscoveryState::eNoBridgeFound:
            qDebug() << "Hue Update: no bridge found";
            break;
        case EHueDiscoveryState::eFindingIpAddress:
            ui->connectionListLabel->setText(QString("Looking for Bridge IP..."));
            qDebug() << "Hue Update: Finding IP Address";
            break;
        case EHueDiscoveryState::eTestingIPAddress:
            ui->connectionListLabel->setText(QString("Looking for Bridge IP..."));
            qDebug() << "Hue Update: Found IP, waiting for response";
            break;
        case EHueDiscoveryState::eFindingDeviceUsername:
            ui->connectionListLabel->setText(QString("Bridge Found! Please press Link button..."));
            qDebug() << "Hue Update: Bridge is waiting for link button to be pressed.";
            break;
        case EHueDiscoveryState::eTestingFullConnection:
            ui->connectionListLabel->setText(QString("Bridge button pressed! Testing connection..."));
            qDebug() << "Hue Update: IP and Username received, testing combination. ";
            break;
        case EHueDiscoveryState::eBridgeConnected:
            ui->connectionListLabel->setText(QString("Hue Lights:"));
            qDebug() << "Hue Update: Bridge Connected";
            break;
        default:
            qDebug() << "Not a recognized state...";
            break;
    }
}

// ----------------------------
// Protected
// ----------------------------


void SettingsPage::showEvent(QShowEvent *event) {
    Q_UNUSED(event);

    updateUI((int)mCommType);
    highlightButton(mCommType);
    // default the settings bars to the current colors
    commTypeSelected((int)mCommType);

     if(mCommType == ECommType::eHue) {
         ui->lineEdit->setHidden(true);
         ui->plusButton->setHidden(true);
         ui->minusButton->setHidden(true);
         ui->connectionListLabel->setHidden(false);
     }
     ui->speedSlider->slider->setValue(mSliderSpeedValue);
     ui->timeoutSlider->slider->setValue(mData->timeOut());
}

void SettingsPage::lightStateChanged(int type, int controllerIndex) {
    Q_UNUSED(controllerIndex);
    if (type == (int)mCommType) {
        updateUI(type);
    }
}

// ----------------------------
// Private
// ----------------------------

void SettingsPage::updateConnectionList(int type) {
   if ((ECommType)type == mCommType) {
        // clear the list
        ui->connectionList->clear();
        int listIndex = 0; // used for inserting new entries on the list
        // get the pointer to the commtype of the connection list
        // iterate through all of its controllers

        //TODO: refactor and simplify section
        //================================================================
        for (uint32_t i = 0; i < (*mComm->controllerList((ECommType)type)).size(); ++i) {
            // get the controller name and if its not empty, continue
            QString controllerName = (*mComm->controllerList((ECommType)type))[i];
            if (QString::compare(QString(""), controllerName)) {
                // grab the controller index
                int controllerIndex = mComm->controllerIndexByName((ECommType)type, controllerName);
                // iterate through all connected devices for that index
                for (int x = 0; x < mComm->numberOfConnectedDevices((ECommType)type, controllerIndex); ++x)
                {
         //================================================================

                   // grab the device
                   SLightDevice light;
                   bool shouldAdd = mComm->deviceByControllerAndIndex((ECommType)type, light, i, x);
                   bool itemFound = false;
                   for (int i = 0; i < ui->connectionList->count(); i++) {
                       if (!QString::compare(ui->connectionList->item(i)->text(), controllerName)) {
                           itemFound = true;
                       }
                   }

                   // if the object is found, is valid, and has a name, add it
                   if (!itemFound && shouldAdd && light.isValid) {
                       LightsListWidget *lightsItem = new LightsListWidget;
                       lightsItem->setup(light, mData);

                       QString structString = CommType::structToString(light);
                       ui->connectionList->addItem(structString);

                       int minimumHeight = ui->connectionList->height() / 5;
                       ui->connectionList->item(listIndex)->setSizeHint(QSize(ui->connectionList->item(listIndex)->sizeHint().width(),
                                                                              minimumHeight));

                       ui->connectionList->setItemWidget(ui->connectionList->item(listIndex), lightsItem);
                       // if it exists in current devices, set it as selected
                       if (mData->doesDeviceExist(light)) {
                          ui->connectionList->item(listIndex)->setSelected(true);
                       }

                       listIndex++;
                   }                   
                }
            }
        }

        emit updateMainIcons();
    }
}

