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

SettingsPage::SettingsPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsPage) {
    ui->setupUi(this);

    ui->singleModeButton->setHidden(true);
    ui->multiModeButton->setHidden(true);

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

    mListIndexVector = std::vector<int>(4);

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
}

SettingsPage::~SettingsPage() {
    delete ui;
}


void SettingsPage::setupUI() {
   connect(mComm, SIGNAL(hueDiscoveryStateChange(int)), this, SLOT(hueDiscoveryUpdate(int)));
   connect(mComm, SIGNAL(lightStateUpdate(int, int)), this, SLOT(lightStateChanged(int, int)));
   commTypeSelected((int)mComm->currentCommType());
}

void SettingsPage::updateUI(int type) {
    if (mData->currentRoutine() <= ELightingRoutine::eSingleSawtoothFadeOut) {
        ui->speedSlider->setSliderColorBackground(mData->mainColor());
        ui->timeoutSlider->setSliderColorBackground(mData->mainColor());
    } else {
        ui->speedSlider->setSliderColorBackground(mData->colorsAverage(mData->currentColorGroup()));
        ui->timeoutSlider->setSliderColorBackground(mData->colorsAverage(mData->currentColorGroup()));
    }
    updateConnectionList(type);
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
    mComm->sendSpeed(mComm->selectedDevice(), mData->speed());
}

void SettingsPage::timeoutChanged(int newTimeout) {
   mData->timeOut(newTimeout);
   mComm->sendTimeOut(mComm->selectedDevice(), mData->timeOut());
}

void SettingsPage::listClicked(QListWidgetItem* item) {
    if (QString::compare(item->text(), mCurrentListString)) {
        mCurrentListString = item->text();
        SSettingsListKeyData structOutput = SettingsListKey::stringToStruct(mCurrentListString);
        int controllerIndex;
        if (structOutput.type == ECommType::eHue) {
            controllerIndex = mComm->comm()->controllerIndexByName("Bridge");
        } else {
            controllerIndex = mComm->comm()->controllerIndexByName(structOutput.name);
        }


        if (structOutput.type == ECommType::eHue) {
    #ifndef MOBILE_BUILD
            // serial connections require a little more effort to properly
            // switch, they need to be closed first and then reconnected
            mComm->closeCurrentConnection();
    #endif //MOBILE_BUILD
            mComm->comm()->selectDevice(controllerIndex, structOutput.index);
            mComm->comm()->selectConnection("Bridge");
        } else if (structOutput.type == ECommType::eUDP
                   || structOutput.type == ECommType::eHTTP)  {
            mComm->comm()->selectConnection(structOutput.name);
            mComm->comm()->selectDevice(controllerIndex, structOutput.index);
            mComm->changeDeviceController(structOutput.name);
        } else {
    #ifndef MOBILE_BUILD
            if (structOutput.type == ECommType::eSerial) {
                // serial connections require a little more effort to properly
                // switch, they need to be closed first and then reconnected
                mComm->changeDeviceController(structOutput.name);
            }
    #endif //MOBILE_BUILD
        }
        ui->lineEdit->setText(mComm->comm()->currentConnection());
        SLightDevice light = (mComm->comm()->controllerDeviceList(controllerIndex))[mComm->selectedDevice() - 1];
        mComm->updateDataLayer(controllerIndex, light.index, (int)mComm->currentCommType());

        // save the current index of this version of the list
        for(int i = 0; i < ui->connectionList->count(); ++i) {
            if (!ui->connectionList->item(i)->text().compare(item->text())) {
                mListIndexVector[(int)mComm->currentCommType()] = i;
            }
        }

        updateUI((int)structOutput.type);
        emit updateMainIcons();
    }
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
    if ((ECommType)type != mComm->currentCommType()) {
        mComm->currentCommType((ECommType)type);
        highlightButton((ECommType)type);
        if (mComm->currentCommType() == ECommType::eHue) {
            if (mComm->isConnected()) {
               // theres a hue bridge already connected, show bridge MAC and all available lights
               ui->connectionListLabel->setText(QString("Hue Lights:"));
               mComm->stopDiscovery();
            } else if (!mComm->isInDiscoveryMode()){
                // no hue connected and not in discovery mode, start discovery mode
                mComm->startDiscovery();
            }
        } else {
            mComm->stopDiscovery();
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
        SLightDevice light = mComm->comm()->controllerDeviceList(0)[mComm->selectedDevice() - 1];
        if (light.isOn) {
            mData->mainColor(light.color);
        } else {
            mData->mainColor(QColor(0,0,0));
        }
        updateUI(type);
        ui->lineEdit->setText(mComm->comm()->currentConnection());
        if (ui->connectionList->count() > 0) {
            ui->connectionList->item(mListIndexVector[(int)type])->setSelected(true);
        }
        emit updateMainIcons();
    }
}


void SettingsPage::plusButtonClicked() {
    //SSettingsListKeyData listData = mKeyTool.structToString(mCurrentListString);
    bool isSuccessful = mComm->comm()->addConnection(ui->lineEdit->text());
    if (isSuccessful) {
        // adjusts the backend to the connection
        mComm->comm()->selectConnection(ui->lineEdit->text());
        // updates the connection list
        updateConnectionList((int)mComm->currentCommType());
        // selects the top connection on the GUI level
        ui->connectionList->item(0)->setSelected(true);
        mListIndexVector[(int)mComm->currentCommType()] = 0;
    }
}


void SettingsPage::minusButtonClicked() {
    SSettingsListKeyData listData = SettingsListKey::stringToStruct(mCurrentListString);
    bool isSuccessful = mComm->comm()->removeConnection(listData.name);

    if (isSuccessful) {

        mCurrentListString = ui->connectionList->item(0)->text();
        // adjusts the backend to the connection
        mComm->comm()->selectConnection(mCurrentListString);
        // update the line edit text
        ui->lineEdit->setText(mComm->comm()->currentConnection());
        // updates the connection list
        updateConnectionList((int)mComm->currentCommType());
        // selects the top connection on the GUI level
        ui->connectionList->item(0)->setSelected(true);
        mListIndexVector[(int)mComm->currentCommType()] = 0;
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
            //updateConnectionList((int)ECommType::eHue);
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
    ui->speedSlider->slider->setValue(mSliderSpeedValue);
    ui->timeoutSlider->slider->setValue(mData->timeOut());
    highlightButton(mComm->currentCommType());
    commTypeSelected((int)mComm->currentCommType());
    mCurrentListString = mComm->comm()->currentConnection();

    // default the settings bars to the current colors
    updateUI((int)mComm->currentCommType());

     if(mComm->currentCommType() == ECommType::eHue) {
         ui->lineEdit->setHidden(true);
         ui->plusButton->setHidden(true);
         ui->minusButton->setHidden(true);
         ui->connectionListLabel->setHidden(false);
     }

}

void SettingsPage::lightStateChanged(int type, int controllerIndex) {
    Q_UNUSED(controllerIndex);
    updateUI(type);
}

// ----------------------------
// Private
// ----------------------------

void SettingsPage::updateConnectionList(int type) {
   if (type == (int)mComm->currentCommType()) {
        // store currently selected index
        int tempIndex = 0;
        for (int i = 0; i < ui->connectionList->count(); i++) {
            if (ui->connectionList->item(i)->isSelected()) {
                tempIndex = i;
            }
        }

        // clear the list
        ui->connectionList->clear();
        int listIndex = 0; // used for inserting new entries on the list
        // get the pointer to the commtype of the connection list
        CommType *commPtr = mComm->commPtrByType((ECommType)type);
        // iterate through all of its controllers
        for (uint32_t i = 0; i < (*commPtr->controllerList()).size(); ++i) {
            // get the controller name and if its not empty, continue
            QString controllerName = (*commPtr->controllerList())[i];
            if (QString::compare(QString(""), controllerName)) {
                // grab the controller index
                int controllerIndex = commPtr->controllerIndexByName(controllerName);
                // iterate through all connected devices for that index
                for (int x = 0; x < commPtr->numberOfConnectedDevices(controllerIndex); ++x)
                {
                   // grab the device
                   SLightDevice light;
                   bool shouldAdd = commPtr->deviceByControllerAndIndex(light, i, x);

                   bool itemFound = false;
                   // Hues all use one name as theres only one bridge connected at a time, other controllers use their controller names.
                   if ((ECommType)type == ECommType::eHue) {
                       controllerName = "Hue Color Lamp";
                   } else {
                       // check for duplicates
                       for (int i = 0; i < ui->connectionList->count(); i++) {
                           if (!QString::compare(ui->connectionList->item(i)->text(), controllerName)) {
                               itemFound = true;
                           }
                       }
                   }

                   // if the object is found, is valid, and has a name, add it
                   if (!itemFound && shouldAdd && light.isValid) {
                       LightsListWidget *lightsItem = new LightsListWidget;
                       lightsItem->setup(controllerName, light.isOn, light.isReachable, light.color, light.index, mData);
                       SSettingsListKeyData structData;
                       structData.index = light.index;
                       structData.type = (ECommType)type;
                       structData.name = controllerName;
                       QString structString = SettingsListKey::structToString(structData);
                       ui->connectionList->addItem(structString);
                       ui->connectionList->setItemWidget(ui->connectionList->item(listIndex), lightsItem);
                       listIndex++;
                   }
                }
            }
        }

        if (ui->connectionList->count() > 0 && tempIndex < ui->connectionList->count()) {
           ui->connectionList->item(tempIndex)->setSelected(true);
        }
        emit updateMainIcons();
    }
}

