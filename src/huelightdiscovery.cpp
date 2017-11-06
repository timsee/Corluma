/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include <QtCore>
#include <QtGui>
#include <QStyleOption>

#include "huelightdiscovery.h"

HueLightDiscovery::HueLightDiscovery(QWidget *parent) : QWidget(parent) {
    //------------
    // Top Layout
    //------------
    mTopWidget = new CorlumaTopWidget("Discover Hues", ":images/closeX.png");
    connect(mTopWidget, SIGNAL(clicked(bool)), this, SLOT(closeButtonPressed(bool)));
    mTopWidget->setFontPoint(20);

    mSearchWidget = new SearchWidget("", this);
    mSearchWidget->enableSizeChecks(6, 6, "Hue serial numbers must be 6 characters long.");
    mSearchWidget->forceUpperCase(true);
    connect(mSearchWidget, SIGNAL(plusClicked()), this, SLOT(plusButtonClicked()));
    connect(mSearchWidget, SIGNAL(minusClicked()), this, SLOT(minusButtonClicked()));

    mLayout = new QVBoxLayout(this);

    mLayout->addWidget(mTopWidget, 1);
    mLayout->addWidget(mSearchWidget, 8);

    //------------
    // Timer
    //------------
    mDiscoveryTimer = new QTimer(this);
    connect(mDiscoveryTimer, SIGNAL(timeout()), this, SLOT(discoveryRoutine()));

}


void HueLightDiscovery::resize(bool resizeFullWidget) {
    QSize size = qobject_cast<QWidget*>(this->parent())->size();
    if (resizeFullWidget) {
        this->setGeometry(size.width() * 0.125f,
                          size.height() * 0.125f,
                          size.width() * 0.75f,
                          size.height() * 0.75f);
    }
}

void HueLightDiscovery::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(48, 47, 47)));
}

void HueLightDiscovery::closeButtonPressed(bool) {
    emit closePressed();
}

void HueLightDiscovery::show() {
    mDiscoveryTimer->start(2500);
    discoveryRoutine();
    mComm->searchForHueLights();
}

void HueLightDiscovery::hide() {
    if (mDiscoveryTimer->isActive()) {
        mDiscoveryTimer->stop();
    }

}

void HueLightDiscovery::discoveryRoutine() {
    qDebug() << " discover";
    // get new lights, which also checks if a scan is active
    mComm->requestNewHueLights();

    // if a scan is not active, reactivate it

    // if any changes happen, update UI
    std::list<SHueLight> hues = mComm->newHueLights();

    // iterate through all hues found
    for (auto hue : hues) {
        mSearchWidget->addToConnectedList(hue.name);
    }
}

// ----------------------------
// Plus/Minus/Line Edit
// ----------------------------


void HueLightDiscovery::plusButtonClicked() {

}

void HueLightDiscovery::minusButtonClicked() {

}

