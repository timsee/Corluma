/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "discoveryarducorwidget.h"

DiscoveryArduCorWidget::DiscoveryArduCorWidget(CommLayer* comm, QWidget* parent)
    : DiscoveryWidget(parent) {
    mComm = comm;

    mSearchWidget = new SearchWidget("192.168.0.101", this);
    connect(mSearchWidget, SIGNAL(plusClicked()), this, SLOT(plusButtonClicked()));
    connect(mSearchWidget, SIGNAL(minusClicked()), this, SLOT(minusButtonClicked()));

    mTopLabel = new QLabel(this);
    mTopLabel->setText("Add or remove IP Addresses:");
    mTopLabel->setWordWrap(true);
    mTopLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mTopLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    mSpacer = new QWidget(this);
    mSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mTopLayout = new QHBoxLayout;
    mTopLayout->addWidget(mTopLabel, 7);
    mTopLayout->addWidget(mSpacer, 3);


    //----------
    // Main Layout
    //----------

    mLayout = new QVBoxLayout;
    mLayout->addLayout(mTopLayout, 4);
    mLayout->addWidget(mSearchWidget, 28);
    setLayout(mLayout);
}


void DiscoveryArduCorWidget::handleDiscovery(bool isCurrentCommType) {
    const auto& controllers = mComm->arducor()->discovery()->controllers().itemVector();
    const auto& undiscoveredControllers = mComm->arducor()->discovery()->undiscoveredControllers();

    if (isCurrentCommType) {
        for (const auto& controller : controllers) {
            mSearchWidget->addToConnectedList(controller.name);
        }

        for (auto undiscoveredController : undiscoveredControllers) {
            mSearchWidget->addToSearchList(undiscoveredController.name);
        }
    }

    // handle button updates
    if (mComm->discoveryErrorsExist(EProtocolType::arduCor)) {
        emit connectionStatusChanged(EProtocolType::arduCor, EConnectionState::connectionError);
    } else if (undiscoveredControllers.empty() && !controllers.empty()) {
        emit connectionStatusChanged(EProtocolType::arduCor, EConnectionState::discovered);
    } else if (!controllers.empty()) {
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
        mComm->arducor()->discovery()->addManualIP(controller);
        //        if (!isSuccessful) qDebug() << "WARNING: failure adding" << controller << "to HTTP
        //        discovery list";
    } else {
        qDebug() << "WARNING: trying to add controller that already exists: "
                 << mSearchWidget->lineEditText();
    }
}

void DiscoveryArduCorWidget::minusButtonClicked() {
    if (doesYunControllerExistAlready(mSearchWidget->lineEditText())) {
        cor::Controller controller;
        controller.name = mSearchWidget->lineEditText();
        bool isSuccessful = mComm->removeController(ECommType::UDP, controller);
        if (!isSuccessful) {
            qDebug() << "WARNING: failure removing" << controller.name << "from UDP discovery list";
        }
        isSuccessful = mComm->removeController(ECommType::HTTP, controller);
        if (!isSuccessful) {
            qDebug() << "WARNING: failure removing" << controller.name
                     << "from HTTP discovery list";
        }
    } else {
        qDebug() << "WARNING: trying to remove controller that doesn't exist: "
                 << mSearchWidget->lineEditText();
    }
}


// ----------------------------
// Helpers
// ----------------------------


bool DiscoveryArduCorWidget::doesYunControllerExistAlready(const QString& name) {
    bool deviceFound = mComm->arducor()->discovery()->controllers().item(name.toStdString()).second;
    if (deviceFound) {
        return true;
    }

    for (const auto& undiscoveredController :
         mComm->arducor()->discovery()->undiscoveredControllers()) {
        if (undiscoveredController.name == name) {
            deviceFound = true;
        }
    }
    return deviceFound;
}
