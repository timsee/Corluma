/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "comm/hue/lightdiscovery.h"

#include <QStyleOption>
#include <QtCore>
#include <QtGui>

#include "comm/commhue.h"

namespace hue {

LightDiscovery::LightDiscovery(QWidget* parent, CommLayer* comm) : QWidget(parent), mComm(comm) {
    //------------
    // Top Layout
    //------------
    mTopWidget = new cor::TopWidget("Discover New Hues", ":images/closeX.png", this);
    connect(mTopWidget, SIGNAL(clicked(bool)), this, SLOT(closeButtonPressed(bool)));
    mTopWidget->setFontPoint(20);

    mSearchHelpLabel = new QLabel(
        "Add the serial number of your Hue to the text field and then press the \"+\" button.",
        this);
    mSearchHelpLabel->setWordWrap(true);
    mSearchHelpLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mSearchWidget =
        new SearchWidget("",
                         this,
                         10,
                         QString("You may only search for 10 Hues manually at a time."));
    mSearchWidget->enableSizeChecks(6, 6, "Hue serial numbers must be 6 characters long.");
    mSearchWidget->forceUpperCase(true);
    connect(mSearchWidget, SIGNAL(plusClicked()), this, SLOT(plusButtonClicked()));
    connect(mSearchWidget, SIGNAL(minusClicked()), this, SLOT(minusButtonClicked()));

    //------------
    // Timer
    //------------
    mDiscoveryTimer = new QTimer(this);
    connect(mDiscoveryTimer, SIGNAL(timeout()), this, SLOT(discoveryRoutine()));
}


void LightDiscovery::resize() {
    QSize size = parentWidget()->size();
    setGeometry(int(size.width() * 0.125f),
                int(size.height() * 0.125f),
                int(size.width() * 0.75f),
                int(size.height() * 0.75f));

    int yPos = 0;
    mTopWidget->setGeometry(0, yPos, width(), int(height() * 0.1));
    yPos += mTopWidget->height();

    mSearchHelpLabel->setGeometry(0, yPos, width(), int(height() * 0.1));
    yPos += mSearchHelpLabel->height();

    mSearchWidget->setGeometry(0, yPos, width(), int(height() * 0.8));
    yPos += mSearchWidget->height();
}

void LightDiscovery::resizeEvent(QResizeEvent*) {
    resize();
}

void LightDiscovery::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
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
            if (serial == searchingSerial) {
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
    auto hues = mComm->hue()->newLights();

    // iterate through all hues found
    for (auto hue : hues) {
        mSearchWidget->addToConnectedList(hue.name());
    }
}


// ----------------------------
// Plus/Minus/Line Edit
// ----------------------------


void LightDiscovery::plusButtonClicked() {
    mComm->hue()->searchForNewLights(mBridge, mSearchWidget->searchingFor());
}

void LightDiscovery::minusButtonClicked() {}

} // namespace hue
