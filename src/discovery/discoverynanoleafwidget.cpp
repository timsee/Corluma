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
    Q_UNUSED(isCurrentCommType);
    // starts discovery if its not already started
    mComm->nanoleaf()->discovery()->startDiscovery();

    std::list<cor::Controller> deviceTable = mComm->discoveredList(ECommType::nanoleaf);
    std::list<QString> discoveringList = mComm->undiscoveredList(ECommType::nanoleaf);

    for (auto device : deviceTable) {
        mSearchWidget->addToConnectedList(device.name);
    }

    for (auto name : discoveringList) {
        mSearchWidget->addToSearchList(name);
    }

    ENanoleafDiscoveryState discoveryState = mComm->nanoleaf()->discovery()->state();
    switch (discoveryState) {
    case ENanoleafDiscoveryState::connectionError:
    case ENanoleafDiscoveryState::discoveryOff:
        mLabel->setText("Connection Error");
        break;
    case ENanoleafDiscoveryState::lookingForPreviousNanoleafs:
        mLabel->setText("Testing previous connection data..");
        break;
    case ENanoleafDiscoveryState::nothingFound:
        mLabel->setText("Looking for a NanoLeaf Aurora. This may take up to a minute...");
        break;
    case ENanoleafDiscoveryState::unknownNanoleafsFound:
        mLabel->setText("NanoLeaf Aurora found! Please hold the power button for around 5 seconds, until the LED to the left of it starts blinking. ");
        break;
    case ENanoleafDiscoveryState::allNanoleafsConnected:
        mLabel->setText("All NanoLeaf discovered and fully connected!");
        break;
    }

    // handle button updates
    if (mComm->discoveryErrorsExist(ECommType::nanoleaf)) {
        emit connectionStatusChanged(EProtocolType::nanoleaf, EConnectionState::connectionError);
    }  else if (deviceTable.size() > 0) {
        emit connectionStatusChanged(EProtocolType::nanoleaf, EConnectionState::discoveredAndNotInUse);
    } else {
        emit connectionStatusChanged(EProtocolType::nanoleaf, EConnectionState::off);
    }
}

// ----------------------------
// Plus/Minus/Line Edit
// ----------------------------


void DiscoveryNanoLeafWidget::plusButtonClicked() {
    mComm->nanoleaf()->discovery()->addIP(mSearchWidget->lineEditText());
}

void DiscoveryNanoLeafWidget::minusButtonClicked() {

}


// ----------------------------
// Helpers
// ----------------------------


bool DiscoveryNanoLeafWidget::doesNanoLeafExist(QString controller) {
    bool deviceFound = false;
    for (auto&& discoveredController : mComm->discoveredList(ECommType::nanoleaf)) {
        if (discoveredController.name.compare(controller) == 0) {
            deviceFound = true;
        }
    }

    for (auto&& unDiscoveredController : mComm->undiscoveredList(ECommType::nanoleaf)) {
        if (unDiscoveredController.compare(controller) == 0) {
            deviceFound = true;
        }
    }

    return deviceFound;
}

