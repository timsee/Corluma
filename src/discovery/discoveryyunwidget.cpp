/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "discoveryyunwidget.h"

DiscoveryYunWidget::DiscoveryYunWidget(CommLayer *comm, QWidget *parent) :
    DiscoveryWidget(parent) {

    mComm = comm;

    //----------
    // Top layout
    //----------

    mLineEdit = new QLineEdit(this);

    mPlusButton = new QPushButton(this);
    connect(mPlusButton, SIGNAL(clicked(bool)), this, SLOT(plusButtonClicked()));
    mPlusButton->setText("+");

    mMinusButton = new QPushButton(this);
    connect(mMinusButton, SIGNAL(clicked(bool)), this, SLOT(minusButtonClicked()));
    mMinusButton->setText("-");


    mInputLayout = new QHBoxLayout;
    mInputLayout->addWidget(mLineEdit, 10);
    mInputLayout->addWidget(mPlusButton, 2);
    mInputLayout->addWidget(mMinusButton, 2);

    //----------
    // UI assets
    //----------

    mTopLabel = new QLabel(this);
    mTopLabel->setText("Add or remove IP Addresses:");
    mTopLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mTopLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    mConnectedLabel = new QLabel(this);
    mConnectedLabel->setText("Connected Yuns:");
    mConnectedLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mConnectedLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    mConnectedListWidget = new QListWidget(this);
    mConnectedListWidget->setStyleSheet("color: silver;");
    mConnectedListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mConnectedListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(connectedListClicked(QListWidgetItem*)));

    mDiscoveringLabel = new QLabel(this);
    mDiscoveringLabel->setText("Looking For:");
    mDiscoveringLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mDiscoveringLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    mDiscoveringListWidget = new QListWidget(this);
    mDiscoveringListWidget->setStyleSheet("color: silver;");
    mDiscoveringListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mDiscoveringListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(discoveringListClicked(QListWidgetItem*)));

    //----------
    // Main Layout
    //----------

    mLayout = new QVBoxLayout;
    mLayout->addWidget(mTopLabel, 4);
    mLayout->addLayout(mInputLayout, 2);
    mLayout->addWidget(mConnectedLabel, 2);
    mLayout->addWidget(mConnectedListWidget, 10);
    mLayout->addWidget(mDiscoveringLabel, 2);
    mLayout->addWidget(mDiscoveringListWidget, 10);
    setLayout(mLayout);
}

DiscoveryYunWidget::~DiscoveryYunWidget() {

}

void DiscoveryYunWidget::handleDiscovery(bool isCurrentCommType) {
    std::list<SDeviceController> deviceTableUDP = mComm->discoveredList(ECommType::eUDP);
    std::list<SDeviceController> deviceTableHTTP = mComm->discoveredList(ECommType::eHTTP);

    std::list<QString> discoveringUDP = mComm->undiscoveredList(ECommType::eUDP);
    std::list<QString> discoveringHTTP = mComm->undiscoveredList(ECommType::eHTTP);

    if (isCurrentCommType) {
        fillList(mConnectedListWidget, deviceTableUDP);
        fillList(mConnectedListWidget, deviceTableHTTP);

        fillList(mDiscoveringListWidget, discoveringUDP);
        fillList(mDiscoveringListWidget, discoveringHTTP);

        // compare the two lists against each other
        for (int i = 0; i < mConnectedListWidget->count(); ++i) {
            QListWidgetItem *connectedItem = mConnectedListWidget->item(i);
            for (int j = 0; j < mDiscoveringListWidget->count(); ++j) {
                QListWidgetItem *discoveringItem = mDiscoveringListWidget->item(j);
                if (connectedItem->text().compare(discoveringItem->text()) == 0) {
                    mDiscoveringListWidget->takeItem(j);
                }
            }
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
        emit connectionStatusChanged((int)ECommType::eUDP, (int)EConnectionState::eConnectionError);
    } else if (discoveringHTTP.size() == 0
            && discoveringUDP.size() == 0
            && (deviceTableHTTP.size() > 0
                || deviceTableUDP.size() > 0)) {
       emit connectionStatusChanged((int)ECommType::eUDP, (int)EConnectionState::eDiscoveredAndNotInUse);
    } else if (discoveringHTTP.size() > 0
               || discoveringUDP.size() > 0) {
        emit connectionStatusChanged((int)ECommType::eUDP, (int)EConnectionState::eDiscovering);
    } else {
        emit connectionStatusChanged((int)ECommType::eUDP, (int)EConnectionState::eOff);
    }

}

// ----------------------------
// Plus/Minus/Line Edit
// ----------------------------


void DiscoveryYunWidget::plusButtonClicked() {
    if (!doesYunControllerExistAlready(mLineEdit->text())) {
        QString controller =  mLineEdit->text();
        bool isSuccessful = mComm->startDiscoveringController(ECommType::eUDP, controller);
        if (!isSuccessful) qDebug() << "WARNING: failure adding" << controller << "to UDP discovery list";
        isSuccessful = mComm->startDiscoveringController(ECommType::eHTTP, controller);
        if (!isSuccessful) qDebug() << "WARNING: failure adding" << controller << "to HTTP discovery list";
    } else {
        qDebug() << "WARNING: trying to add controller that already exists: " << mLineEdit->text();
    }
}

void DiscoveryYunWidget::minusButtonClicked() {
    if (doesYunControllerExistAlready(mLineEdit->text())) {
        SDeviceController controller;
        controller.name =  mLineEdit->text();
        bool isSuccessful = mComm->removeController(ECommType::eUDP, controller);
        if (!isSuccessful) qDebug() << "WARNING: failure removing" << controller.name << "from UDP discovery list";
        isSuccessful = mComm->removeController(ECommType::eHTTP, controller);
        if (!isSuccessful) qDebug() << "WARNING: failure removing" << controller.name << "from HTTP discovery list";

        for (int i = 0; i < mConnectedListWidget->count(); ++i) {
            QListWidgetItem *item = mConnectedListWidget->item(i);
            if (item->text().compare(controller.name) == 0) {
                mConnectedListWidget->takeItem(i);
            }
        }


        for (int i = 0; i < mDiscoveringListWidget->count(); ++i) {
            QListWidgetItem *item = mDiscoveringListWidget->item(i);
            if (item->text().compare(controller.name) == 0) {
                mDiscoveringListWidget->takeItem(i);
            }
        }
    } else {
        qDebug() << "WARNING: trying to remove controller that doesn't exist: " << mLineEdit->text();
    }
}


// ----------------------------
// Discovery Lists
// ----------------------------


void DiscoveryYunWidget::connectedListClicked(QListWidgetItem *item) {
    mLineEdit->setText(item->text());
    mLastIP = item->text();
    for (int i = 0; i < mDiscoveringListWidget->count(); ++i) {
        mDiscoveringListWidget->item(i)->setSelected(false);
    }
    yunLineEditHelper();
}


void DiscoveryYunWidget::discoveringListClicked(QListWidgetItem *item) {
    mLineEdit->setText(item->text());
    mLastIP = item->text();
    for (int i = 0; i < mConnectedListWidget->count(); ++i) {
       mConnectedListWidget->item(i)->setSelected(false);
    }
    yunLineEditHelper();
}



// ----------------------------
// Helpers
// ----------------------------



void DiscoveryYunWidget::yunLineEditHelper() {
    // check last IP
    bool foundLastIP = false;
    if (mLastIP.compare("") != 0) {
        for (int i = 0; i < mDiscoveringListWidget->count(); ++i) {
            if (mLastIP.compare(mDiscoveringListWidget->item(i)->text()) == 0) {
                mDiscoveringListWidget->item(i)->setSelected(true);
                foundLastIP = true;
            }
        }
        for (int i = 0; i < mConnectedListWidget->count(); ++i) {
            if (!mConnectedListWidget->item(i)->text().isNull()) {
                if (mLastIP.compare(mConnectedListWidget->item(i)->text()) == 0) {
                    mConnectedListWidget->item(i)->setSelected(true);
                    foundLastIP = true;
                }
            }
        }
    }

    if (!foundLastIP) {
        bool anySelected = false;
        // check if any are selected
        for (int i = 0; i < mDiscoveringListWidget->count(); ++i) {
            if (mDiscoveringListWidget->item(i)->isSelected()) anySelected = true;
        }
        for (int i = 0; i < mConnectedListWidget->count(); ++i) {
            if (mConnectedListWidget->item(i)->isSelected()) anySelected = true;
        }

        // if none selected but some exist, select first one.
        if (!anySelected && ((mDiscoveringListWidget->count() > 0) || (mConnectedListWidget->count() > 0))) {
            if (mConnectedListWidget->count() > 0) {
                mConnectedListWidget->item(0)->setSelected(true);
                mLineEdit->setText(mConnectedListWidget->item(0)->text());
            } else {
                mDiscoveringListWidget->item(0)->setSelected(true);
                mLineEdit->setText(mDiscoveringListWidget->item(0)->text());
            }
        } else {
            mLineEdit->setText("192.168.0.101");
        }
    }
}


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
