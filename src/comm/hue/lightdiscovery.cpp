/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include <QStyleOption>
#include <QtCore>
#include <QtGui>

#include "comm/commhue.h"
#include "comm/hue/lightdiscovery.h"

namespace hue {

LightDiscovery::LightDiscovery(QWidget* parent, CommLayer* comm) : QWidget(parent), mComm(comm) {
    //------------
    // Top Layout
    //------------
    mTopWidget = new cor::TopWidget("Discover Hues", ":images/closeX.png", this);
    connect(mTopWidget, SIGNAL(clicked(bool)), this, SLOT(closeButtonPressed(bool)));
    mTopWidget->setFontPoint(20);

    mSearchButton = new QPushButton(this);
    mSearchButton->setText("Search");
    mSearchButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mSearchButton, SIGNAL(clicked(bool)), this, SLOT(searchButtonPressed(bool)));

    mSearchWidget = new SearchWidget(
        "", this, 10, QString("You may only search for 10 Hues manually at a time."));
    mSearchWidget->enableSizeChecks(6, 6, "Hue serial numbers must be 6 characters long.");
    mSearchWidget->forceUpperCase(true);
    connect(mSearchWidget, SIGNAL(plusClicked()), this, SLOT(plusButtonClicked()));
    connect(mSearchWidget, SIGNAL(minusClicked()), this, SLOT(minusButtonClicked()));

    mLayout = new QVBoxLayout(this);
    mLayout->addWidget(mTopWidget, 1);
    mLayout->addWidget(mSearchButton, 1);
    mLayout->addWidget(mSearchWidget, 8);

    //------------
    // Timer
    //------------
    mDiscoveryTimer = new QTimer(this);
    connect(mDiscoveryTimer, SIGNAL(timeout()), this, SLOT(discoveryRoutine()));
}


void LightDiscovery::resize(bool resizeFullWidget) {
    QSize size = qobject_cast<QWidget*>(this->parent())->size();
    if (resizeFullWidget) {
        this->setGeometry(int(size.width() * 0.125f),
                          int(size.height() * 0.125f),
                          int(size.width() * 0.75f),
                          int(size.height() * 0.75f));
    }
}

void LightDiscovery::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(48, 47, 47)));
}

void LightDiscovery::closeButtonPressed(bool) {
    emit closePressed();
}

void LightDiscovery::show(const hue::Bridge& bridge) {
    mBridge = bridge;
    mDiscoveryTimer->start(2500);
    discoveryRoutine();
}

void LightDiscovery::hide() {
    if (mDiscoveryTimer->isActive()) {
        mDiscoveryTimer->stop();
    }
}

void LightDiscovery::discoveryRoutine() {
    // get new lights, which also checks if a scan is active
    mComm->hue()->requestNewLights(mBridge);
    // see if any new lights have been added to UI
    bool newLightsAdded = false;
    for (const auto& serial : mSearchWidget->searchingFor()) {
        bool foundSerial = false;
        for (const auto& searchingSerial : mComm->hue()->searchingLights()) {
            if (serial.compare(searchingSerial) == 0) {
                foundSerial = true;
            }
        }
        if (!foundSerial) {
            newLightsAdded = true;
        }
    }

    // if scan is not active or if new lights have been added, restart scan
    if (!mComm->hue()->scanIsActive() || newLightsAdded) {
        qDebug() << " search for new lights! active: " << mComm->hue()->scanIsActive()
                 << " new lgihts added" << newLightsAdded;
        mComm->hue()->searchForNewLights(mBridge, mSearchWidget->searchingFor());
    }

    // if any changes happen, update UI
    std::list<HueLight> hues = mComm->hue()->newLights();

    // iterate through all hues found
    for (auto hue : hues) {
        mSearchWidget->addToConnectedList(hue.name);
    }
}

void LightDiscovery::searchButtonPressed(bool) {
    mComm->hue()->searchForNewLights(mBridge, mSearchWidget->searchingFor());
}

// ----------------------------
// Plus/Minus/Line Edit
// ----------------------------


void LightDiscovery::plusButtonClicked() {}

void LightDiscovery::minusButtonClicked() {}

} // namespace hue
