#ifndef MOBILE_BUILD
/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "discoveryserialwidget.h"

DiscoverySerialWidget::DiscoverySerialWidget(CommLayer *comm, QWidget *parent) :
    DiscoveryWidget(parent) {

    mComm = comm;

    mLabel = new QLabel(this);
    mLabel->setText("No serial devices found.");
    mLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mListWidget = new QListWidget(this);
    mListWidget->setStyleSheet("color: silver;");

    mLayout = new QVBoxLayout;
    mLayout->addWidget(mLabel, 2);
    mLayout->addWidget(mListWidget, 20);
    setLayout(mLayout);
}

DiscoverySerialWidget::~DiscoverySerialWidget() {

}

void DiscoverySerialWidget::handleDiscovery(bool isCurrentCommType) {
    std::list<SDeviceController> deviceTable = mComm->discoveredList(ECommType::eSerial);
    bool runningDiscovery = mComm->lookingForActivePorts();
    if (isCurrentCommType) {
        if (deviceTable.size() > 0) {
            mLabel->setText(QString("Serial devices found!"));
        } else {
            mLabel->setText(QString("No serial devices found."));
        }
        fillList(mListWidget, deviceTable);
    }
    // get serial state
    if (mComm->discoveryErrorsExist(ECommType::eSerial)) {
        emit connectionStatusChanged((int)ECommType::eSerial, (int)EConnectionState::eConnectionError);
    } else if (!runningDiscovery && deviceTable.size() == 0) {
        // no serial ports discovered
        emit connectionStatusChanged((int)ECommType::eSerial, (int)EConnectionState::eOff);
    } else if (!runningDiscovery && deviceTable.size() > 0) {
        // all serial ports connect to corluma lights
        emit connectionStatusChanged((int)ECommType::eSerial, (int)EConnectionState::eDiscoveredAndNotInUse);
    } else if (runningDiscovery) {
        // some did not connect to corluma lights
        emit connectionStatusChanged((int)ECommType::eSerial, (int)EConnectionState::eDiscovering);
    } else {
        qDebug() << __func__ << "unhandled case";
    }
}

#endif //MOBILE_BUILD
