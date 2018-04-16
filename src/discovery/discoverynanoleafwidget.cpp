/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "discoverynanoleafwidget.h"
#include "comm/commnanoleaf.h"

DiscoveryNanoLeafWidget::DiscoveryNanoLeafWidget(CommLayer *comm, QWidget *parent) :
    DiscoveryWidget(parent) {
   // mScale = 0.5f;

    mComm = comm;

    mLabel = new QLabel(this);
    mLabel->setWordWrap(true);
    mLabel->setText("Looking for NanoLeaf...");
    mLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);



    mSearchWidget = new SearchWidget("192.168.0.114", this);
    connect(mSearchWidget, SIGNAL(plusClicked()), this, SLOT(plusButtonClicked()));
    connect(mSearchWidget, SIGNAL(minusClicked()), this, SLOT(minusButtonClicked()));


    mLayout = new QVBoxLayout;
    mLayout->addWidget(mLabel, 4);
    mLayout->addWidget(mSearchWidget, 28);
    mLayout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    setLayout(mLayout);

}

DiscoveryNanoLeafWidget::~DiscoveryNanoLeafWidget() {

}



void DiscoveryNanoLeafWidget::handleDiscovery(bool isCurrentCommType) {
    std::list<cor::Controller> deviceTable = mComm->discoveredList(ECommType::eNanoleaf);
    std::list<QString> discoveringList = mComm->undiscoveredList(ECommType::eNanoleaf);

    if (isCurrentCommType) {
        for (auto device : deviceTable) {
            mSearchWidget->addToConnectedList(device.name);
        }

        for (auto name : discoveringList) {
            mSearchWidget->addToSearchList(name);
        }
    }

    ENanoleafDiscoveryState discoveryState = mComm->nanoLeaf()->discoveryState();
    switch (discoveryState) {
    case ENanoleafDiscoveryState::eConnectionError:
        mLabel->setText("Connection Error");
        break;
    case ENanoleafDiscoveryState::eTestingPreviousData:
    case ENanoleafDiscoveryState::eTestingAuth:
        mLabel->setText("Testing previous connection data..");
        break;
    case ENanoleafDiscoveryState::eTestingIP:
    case ENanoleafDiscoveryState::eRunningUPnP:
        mLabel->setText("Looking for a NanoLeaf Aurora. This may take up to a minute...");
        break;
    case ENanoleafDiscoveryState::eAwaitingAuthToken:
        mLabel->setText("NanoLeaf Aurora found! Please hold the power button for around 5 seconds, until the LED to the left of it starts blinking. ");
        break;
    case ENanoleafDiscoveryState::eFullyConnected:
        mLabel->setText("NanoLeaf discovered and fully connected!");
        break;
    }

    // handle button updates
    if (mComm->discoveryErrorsExist(ECommType::eNanoleaf)) {
        emit connectionStatusChanged(EProtocolType::eNanoleaf, EConnectionState::eConnectionError);
    }  else if (deviceTable.size() > 0) {
        emit connectionStatusChanged(EProtocolType::eNanoleaf, EConnectionState::eDiscoveredAndNotInUse);
    } else {
        emit connectionStatusChanged(EProtocolType::eNanoleaf, EConnectionState::eOff);
    }

}

// ----------------------------
// Plus/Minus/Line Edit
// ----------------------------


void DiscoveryNanoLeafWidget::plusButtonClicked() {
    mComm->nanoLeaf()->attemptIP(mSearchWidget->lineEditText());
}

void DiscoveryNanoLeafWidget::minusButtonClicked() {

}


// ----------------------------
// Helpers
// ----------------------------


bool DiscoveryNanoLeafWidget::doesNanoLeafExist(QString controller) {
    bool deviceFound = false;
    for (auto&& discoveredController : mComm->discoveredList(ECommType::eNanoleaf)) {
        if (discoveredController.name.compare(controller) == 0) {
            deviceFound = true;
        }
    }

    for (auto&& unDiscoveredController : mComm->undiscoveredList(ECommType::eNanoleaf)) {
        if (unDiscoveredController.compare(controller) == 0) {
            deviceFound = true;
        }
    }

    return deviceFound;
}

