/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "discoveryyunwidget.h"

DiscoveryYunWidget::DiscoveryYunWidget(CommLayer *comm, QWidget *parent) :
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

DiscoveryYunWidget::~DiscoveryYunWidget() {

}

void DiscoveryYunWidget::handleDiscovery(bool isCurrentCommType) {
    std::list<cor::Controller> deviceTableUDP = mComm->discoveredList(ECommType::eUDP);
    std::list<cor::Controller> deviceTableHTTP = mComm->discoveredList(ECommType::eHTTP);

    std::list<QString> discoveringUDP = mComm->undiscoveredList(ECommType::eUDP);
    std::list<QString> discoveringHTTP = mComm->undiscoveredList(ECommType::eHTTP);

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

        // compare discovered list of UDP against HTTP, removing those that are discovering in HTTP
        for (auto&& discovered : deviceTableUDP) {
            for (auto&& undiscovered : discoveringHTTP) {
                if (discovered.name.compare(undiscovered) == 0) {
                    mComm->removeController(ECommType::eHTTP, discovered);
                }
            }
        }

        // compare discovered list of HTTP against UDP, removing those that are discovering in UDP
        for (auto&& discovered : deviceTableHTTP) {
            for (auto&& undiscovered : discoveringUDP) {
                if (discovered.name.compare(undiscovered) == 0) {
                    mComm->removeController(ECommType::eUDP, discovered);
                }
            }
        }
    }

    // handle button updates
    if (mComm->discoveryErrorsExist(ECommType::eUDP)) {
        emit connectionStatusChanged(EProtocolType::eArduCor, EConnectionState::eConnectionError);
    } else if (discoveringHTTP.size() == 0
            && discoveringUDP.size() == 0
            && (deviceTableHTTP.size() > 0
                || deviceTableUDP.size() > 0)) {
       emit connectionStatusChanged(EProtocolType::eArduCor, EConnectionState::eDiscoveredAndNotInUse);
    } else if (discoveringHTTP.size() > 0
               || discoveringUDP.size() > 0) {
        emit connectionStatusChanged(EProtocolType::eArduCor, EConnectionState::eDiscovering);
    } else {
        emit connectionStatusChanged(EProtocolType::eArduCor, EConnectionState::eOff);
    }

}

// ----------------------------
// Plus/Minus/Line Edit
// ----------------------------


void DiscoveryYunWidget::plusButtonClicked() {
    if (!doesYunControllerExistAlready(mSearchWidget->lineEditText())) {
        QString controller = mSearchWidget->lineEditText();
        bool isSuccessful = mComm->startDiscoveringController(ECommType::eUDP, controller);
        if (!isSuccessful) qDebug() << "WARNING: failure adding" << controller << "to UDP discovery list";
        isSuccessful = mComm->startDiscoveringController(ECommType::eHTTP, controller);
        if (!isSuccessful) qDebug() << "WARNING: failure adding" << controller << "to HTTP discovery list";
    } else {
        qDebug() << "WARNING: trying to add controller that already exists: " << mSearchWidget->lineEditText();
    }
}

void DiscoveryYunWidget::minusButtonClicked() {
    if (doesYunControllerExistAlready(mSearchWidget->lineEditText())) {
        cor::Controller controller;
        controller.name =  mSearchWidget->lineEditText();
        bool isSuccessful = mComm->removeController(ECommType::eUDP, controller);
        if (!isSuccessful) qDebug() << "WARNING: failure removing" << controller.name << "from UDP discovery list";
        isSuccessful = mComm->removeController(ECommType::eHTTP, controller);
        if (!isSuccessful) qDebug() << "WARNING: failure removing" << controller.name << "from HTTP discovery list";
    } else {
        qDebug() << "WARNING: trying to remove controller that doesn't exist: " << mSearchWidget->lineEditText();
    }
}


// ----------------------------
// Helpers
// ----------------------------


bool DiscoveryYunWidget::doesYunControllerExistAlready(QString controller) {
    bool deviceFound = false;
    for (auto&& discoveredController : mComm->discoveredList(ECommType::eHTTP)) {
        if (discoveredController.name.compare(controller) == 0) {
            deviceFound = true;
        }
    }

    for (auto&& discoveredController : mComm->discoveredList(ECommType::eUDP)) {
        if (discoveredController.name.compare(controller) == 0) {
            deviceFound = true;
        }
    }

    for (auto&& unDiscoveredController : mComm->undiscoveredList(ECommType::eHTTP)) {
        if (unDiscoveredController.compare(controller) == 0) {
            deviceFound = true;
        }
    }

    for (auto&& unDiscoveredController : mComm->undiscoveredList(ECommType::eUDP)) {
        if (unDiscoveredController.compare(controller) == 0) {
            deviceFound = true;
        }
    }
    return deviceFound;
}
