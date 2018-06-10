/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "discoveryarducorwidget.h"

DiscoveryArduCorWidget::DiscoveryArduCorWidget(CommLayer *comm, QWidget *parent) :
    DiscoveryWidget(parent) {

    mComm = comm;

    mSearchWidget = new SearchWidget("192.168.0.101", this);
    connect(mSearchWidget, SIGNAL(plusClicked()), this, SLOT(plusButtonClicked()));
    connect(mSearchWidget, SIGNAL(minusClicked()), this, SLOT(minusButtonClicked()));

    mTopLabel = new QLabel(this);
    mTopLabel->setText("Add or remove IP Addresses:");
    mTopLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mTopLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    //----------
    // Main Layout
    //----------

    mLayout = new QVBoxLayout;
    mLayout->addWidget(mTopLabel, 4);
    mLayout->addWidget(mSearchWidget, 28);
    setLayout(mLayout);
}

DiscoveryArduCorWidget::~DiscoveryArduCorWidget() {

}

void DiscoveryArduCorWidget::handleDiscovery(bool isCurrentCommType) {
    std::list<cor::Controller> deviceTableUDP = mComm->discoveredList(ECommType::UDP);
    std::list<cor::Controller> deviceTableHTTP = mComm->discoveredList(ECommType::HTTP);
#ifndef MOBILE_BUILD
    std::list<cor::Controller> deviceTableSerial = mComm->discoveredList(ECommType::serial);
#endif

    std::list<QString> discoveringUDP = mComm->undiscoveredList(ECommType::UDP);
    std::list<QString> discoveringHTTP = mComm->undiscoveredList(ECommType::HTTP);

    if (isCurrentCommType) {
        for (auto device : deviceTableUDP) {
            mSearchWidget->addToConnectedList(device.name);
        }

        for (auto device : deviceTableHTTP) {
            mSearchWidget->addToConnectedList(device.name);
        }

        for (auto name : discoveringUDP) {
            mSearchWidget->addToSearchList(name);
        }

        for (auto name : discoveringHTTP) {
            mSearchWidget->addToSearchList(name);
        }

#ifndef MOBILE_BUILD
        for (auto device : deviceTableSerial) {
            mSearchWidget->addToConnectedList(device.name);
        }
#endif
        // compare discovered list of UDP against HTTP, removing those that are discovering in HTTP
        for (auto&& discovered : deviceTableUDP) {
            for (auto&& undiscovered : discoveringHTTP) {
                if (discovered.name.compare(undiscovered) == 0) {
                    mComm->removeController(ECommType::HTTP, discovered);
                }
            }
        }

        // compare discovered list of HTTP against UDP, removing those that are discovering in UDP
        for (auto&& discovered : deviceTableHTTP) {
            for (auto&& undiscovered : discoveringUDP) {
                if (discovered.name.compare(undiscovered) == 0) {
                    mComm->removeController(ECommType::UDP, discovered);
                }
            }
        }
    }

    // handle button updates
    if (mComm->discoveryErrorsExist(ECommType::UDP)) {
        emit connectionStatusChanged(EProtocolType::arduCor, EConnectionState::connectionError);
    } else if (discoveringHTTP.size() == 0
            && discoveringUDP.size() == 0
            && (deviceTableHTTP.size() > 0
                || deviceTableUDP.size() > 0)) {
       emit connectionStatusChanged(EProtocolType::arduCor, EConnectionState::discoveredAndNotInUse);
    } else if (discoveringHTTP.size() > 0
               || discoveringUDP.size() > 0) {
        emit connectionStatusChanged(EProtocolType::arduCor, EConnectionState::discovering);
    } else {
        emit connectionStatusChanged(EProtocolType::arduCor, EConnectionState::off);
    }

}

// ----------------------------
// Plus/Minus/Line Edit
// ----------------------------


void DiscoveryArduCorWidget::plusButtonClicked() {
    if (!doesYunControllerExistAlready(mSearchWidget->lineEditText())) {
        QString controller = mSearchWidget->lineEditText();
        bool isSuccessful = mComm->startDiscoveringController(ECommType::UDP, controller);
        if (!isSuccessful) qDebug() << "WARNING: failure adding" << controller << "to UDP discovery list";
        isSuccessful = mComm->startDiscoveringController(ECommType::HTTP, controller);
        if (!isSuccessful) qDebug() << "WARNING: failure adding" << controller << "to HTTP discovery list";
    } else {
        qDebug() << "WARNING: trying to add controller that already exists: " << mSearchWidget->lineEditText();
    }
}

void DiscoveryArduCorWidget::minusButtonClicked() {
    if (doesYunControllerExistAlready(mSearchWidget->lineEditText())) {
        cor::Controller controller;
        controller.name =  mSearchWidget->lineEditText();
        bool isSuccessful = mComm->removeController(ECommType::UDP, controller);
        if (!isSuccessful) qDebug() << "WARNING: failure removing" << controller.name << "from UDP discovery list";
        isSuccessful = mComm->removeController(ECommType::HTTP, controller);
        if (!isSuccessful) qDebug() << "WARNING: failure removing" << controller.name << "from HTTP discovery list";
    } else {
        qDebug() << "WARNING: trying to remove controller that doesn't exist: " << mSearchWidget->lineEditText();
    }
}


// ----------------------------
// Helpers
// ----------------------------


bool DiscoveryArduCorWidget::doesYunControllerExistAlready(QString controller) {
    bool deviceFound = false;
    for (auto&& discoveredController : mComm->discoveredList(ECommType::HTTP)) {
        if (discoveredController.name.compare(controller) == 0) {
            deviceFound = true;
        }
    }

    for (auto&& discoveredController : mComm->discoveredList(ECommType::UDP)) {
        if (discoveredController.name.compare(controller) == 0) {
            deviceFound = true;
        }
    }

    for (auto&& unDiscoveredController : mComm->undiscoveredList(ECommType::HTTP)) {
        if (unDiscoveredController.compare(controller) == 0) {
            deviceFound = true;
        }
    }

    for (auto&& unDiscoveredController : mComm->undiscoveredList(ECommType::UDP)) {
        if (unDiscoveredController.compare(controller) == 0) {
            deviceFound = true;
        }
    }
    return deviceFound;
}
