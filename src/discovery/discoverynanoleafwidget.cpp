/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "discoverynanoleafwidget.h"

#include "comm/commnanoleaf.h"

DiscoveryNanoLeafWidget::DiscoveryNanoLeafWidget(CommLayer* comm, QWidget* parent)
    : DiscoveryWidget(parent) {
    mComm = comm;

    mLabel = new QLabel(this);
    mLabel->setWordWrap(true);
    mLabel->setText("Looking for NanoLeaf...");
    mLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    mSpacer = new QWidget(this);
    mSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mTopLayout = new QHBoxLayout;
    mTopLayout->addWidget(mLabel, 7);
    mTopLayout->addWidget(mSpacer, 3);

    mSearchWidget = new SearchWidget("192.168.0.114", this);
    connect(mSearchWidget, SIGNAL(plusClicked()), this, SLOT(plusButtonClicked()));
    connect(mSearchWidget, SIGNAL(minusClicked()), this, SLOT(minusButtonClicked()));

    mLayout = new QVBoxLayout;
    mLayout->addLayout(mTopLayout, 4);
    mLayout->addWidget(mSearchWidget, 28);
    mLayout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    setLayout(mLayout);
}


void DiscoveryNanoLeafWidget::handleDiscovery(bool) {
    // starts discovery if its not already started
    mComm->nanoleaf()->discovery()->startDiscovery();

    const auto& foundNanoleafs = mComm->nanoleaf()->discovery()->foundLights().items();
    const auto& notFoundNanoleafs = mComm->nanoleaf()->discovery()->notFoundLights();

    for (const auto& nanoleaf : foundNanoleafs) {
        mSearchWidget->addToConnectedList(nanoleaf.name());
    }

    for (const auto& nanoleaf : notFoundNanoleafs) {
        mSearchWidget->addToSearchList(nanoleaf.name());
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
            mLabel->setText("NanoLeaf Aurora found! Please hold the power button for around 5 "
                            "seconds, until the LED to the left of it starts blinking. ");
            break;
        case ENanoleafDiscoveryState::allNanoleafsConnected:
            mLabel->setText("All NanoLeaf discovered and fully connected!");
            break;
    }

    // handle button updates
    if (mComm->discoveryErrorsExist(EProtocolType::nanoleaf)) {
        emit connectionStatusChanged(EProtocolType::nanoleaf, EConnectionState::connectionError);
    } else if (!foundNanoleafs.empty()) {
        emit connectionStatusChanged(EProtocolType::nanoleaf, EConnectionState::discovered);
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

void DiscoveryNanoLeafWidget::minusButtonClicked() {}
