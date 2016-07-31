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

#ifndef MOBILE_BUILD
    ui->serialButton->setCheckable(true);
    connect(ui->serialButton, SIGNAL(clicked(bool)), commTypeMapper, SLOT(map()));
    commTypeMapper->setMapping(ui->serialButton, (int)ECommType::eSerial);
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
   connect(mComm, SIGNAL(lightStateUpdate()), this, SLOT(lightStateChanged()));
   commTypeSelected((int)mComm->currentCommType());   
}

void SettingsPage::updateUI() {
    if (mData->currentRoutine() <= ELightingRoutine::eSingleSawtoothFadeOut) {
        ui->speedSlider->setSliderColorBackground(mData->mainColor());
        ui->timeoutSlider->setSliderColorBackground(mData->mainColor());
    } else {
        ui->speedSlider->setSliderColorBackground(mData->colorsAverage(mData->currentColorGroup()));
        ui->timeoutSlider->setSliderColorBackground(mData->colorsAverage(mData->currentColorGroup()));
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
    mComm->sendSpeed(mComm->selectedDevice(), mData->speed());
}

void SettingsPage::timeoutChanged(int newTimeout) {
   mData->timeOut(newTimeout);
   mComm->sendTimeOut(mComm->selectedDevice(), mData->timeOut());
}

void SettingsPage::listClicked(QListWidgetItem* item) {
    if (QString::compare(item->text(), mCurrentListString)) {
        mCurrentListString = item->text();
        if (mComm->currentCommType() == ECommType::eHue) {
    #ifndef MOBILE_BUILD
            // serial connections require a little more effort to properly
            // switch, they need to be closed first and then reconnected
            mComm->closeCurrentConnection();
    #endif //MOBILE_BUILD
            int index = item->text().toInt();
            mComm->comm()->selectDevice(index);
            mComm->comm()->selectConnection(mCurrentListString);
        } else if (mComm->currentCommType() == ECommType::eUDP) {
            int index = item->text().toInt();
            mComm->comm()->selectConnection(mCurrentListString);
            mComm->comm()->selectDevice(index);
            mComm->changeDeviceController(mCurrentListString);
        } else {
            mCurrentListString = item->text();
    #ifndef MOBILE_BUILD
            if (mComm->currentCommType() == ECommType::eSerial) {
                // serial connections require a little more effort to properly
                // switch, they need to be closed first and then reconnected
                mComm->changeDeviceController(mCurrentListString);
            }
    #endif //MOBILE_BUILD
            ui->lineEdit->setText(mComm->comm()->currentConnection());
        }
        SLightDevice light = mComm->deviceList()[mComm->selectedDevice() - 1];
        mComm->updateDataLayer(light.index, (int)mComm->currentCommType());
        updateUI();
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
        updateConnectionList();
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
        if (ui->connectionList->count() > 0 && ui->connectionList->count() > mComm->comm()->listIndex()) {
            ui->connectionList->item(mComm->comm()->listIndex())->setSelected(true);
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
        SLightDevice light = mComm->deviceList()[mComm->selectedDevice() - 1];
        if (light.isOn) {
            mData->mainColor(light.color);
        } else {
            mData->mainColor(QColor(0,0,0));
        }
        updateUI();
        emit updateMainIcons();
    }
}


void SettingsPage::plusButtonClicked() {
    bool isSuccessful = mComm->comm()->addConnection(ui->lineEdit->text());
    if (isSuccessful) {
        // adjusts the backend to the connection
        mComm->comm()->selectConnection(ui->lineEdit->text());
        // updates the connection list
        updateConnectionList();
        // selects the top connection on the GUI level
        ui->connectionList->item(0)->setSelected(true);
    }
}


void SettingsPage::minusButtonClicked() {
    bool isSuccessful = mComm->comm()->removeConnection(mCurrentListString);
    if (isSuccessful) {
        mCurrentListString = ui->connectionList->item(0)->text();
        // adjusts the backend to the connection
        mComm->comm()->selectConnection(mCurrentListString);
        // update the line edit text
        ui->lineEdit->setText(mCurrentListString);
        // updates the connection list
        updateConnectionList();
        // selects the top connection on the GUI level
        ui->connectionList->item(0)->setSelected(true);
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
            updateConnectionList();
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

    // default the settings bars to the current colors
    updateUI();
    updateConnectionList();

#ifndef MOBILE_BUILD
    if (mComm->currentCommType() == ECommType::eSerial && mComm->isConnected()) {
        ui->connectionList->item(mComm->comm()->listIndex())->setSelected(true);
    }
#endif //MOBILE_BUILD

     if(mComm->currentCommType() == ECommType::eHue) {
         ui->lineEdit->setHidden(true);
         ui->plusButton->setHidden(true);
         ui->minusButton->setHidden(true);
         ui->connectionListLabel->setHidden(false);\
     }
}

void SettingsPage::lightStateChanged() {
    updateConnectionList();
    updateUI();
}

// ----------------------------
// Private
// ----------------------------

void SettingsPage::updateConnectionList() {
    // store currently selected index
    int tempIndex = 0;
    for (int i = 0; i < ui->connectionList->count(); i++) {
        if (ui->connectionList->item(i)->isSelected()) {
            tempIndex = i;
        }
    }
    ui->connectionList->clear();
    int i = 0;
    for (QString connectionName : (*mComm->comm()->controllerList().get())) {
        if (QString::compare(QString(""), connectionName)) {
            for (SLightDevice light : mComm->deviceList()) {
                QString object;
                // Hues all use one name, other objects use their connection names.
                if (mComm->currentCommType() == ECommType::eHue) {
                    object = "Hue Color Lamp";
                } else {
                    object = connectionName;
                }
                // TODO: compare with something better than hidden text...
                bool itemFound = false;
                for (int i = 0; i < ui->connectionList->count(); i++) {
                    if (!QString::compare(ui->connectionList->item(i)->text(), object)) {
                        itemFound = true;
                    }
                }
                // add item to list
                if (!itemFound && (QString::compare(QString(""), object))) {
                    LightsListWidget *lightsItem = new LightsListWidget;
                    lightsItem->setup(object, light.isOn, light.isReachable, light.color, light.index, mData);
                    ui->connectionList->addItem(QString::number(light.index));
                    ui->connectionList->setItemWidget(ui->connectionList->item(i), lightsItem);
                }
                i++;
            }
        }
    }
    ui->lineEdit->setText(mComm->comm()->currentConnection());
    if (ui->connectionList->count() > 0 && tempIndex < ui->connectionList->count()) {
       ui->connectionList->item(tempIndex)->setSelected(true);
    }
    emit updateMainIcons();
}
