#include "lightslistmenu.h"
#include <QScrollBar>
#include <QScroller>

LightsListMenu::LightsListMenu(QWidget* parent, CommLayer* comm, bool displayState)
    : QWidget(parent),
      mComm{comm},
      mScrollArea{new QScrollArea(this)},
      mLightContainer{new MenuLightContainer(mScrollArea, false, displayState)} {
    mLightContainer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mScrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QScroller::grabGesture(mScrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    mScrollArea->setWidget(mLightContainer);
    mScrollArea->setFrameStyle(QFrame::NoFrame);
    mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mScrollArea->horizontalScrollBar()->setEnabled(false);
    mScrollArea->horizontalScrollBar()->setVisible(false);
}


void LightsListMenu::resize(const QRect& inputRect, int buttonHeight) {
    setGeometry(inputRect);
    int offsetY = 0u;
    mRowHeight = buttonHeight;

    QRect rect = QRect(0, offsetY, this->width(), this->height() - offsetY);
    int scrollAreaWidth = int(rect.width() * 1.2);
    mScrollArea->setGeometry(rect.x(), rect.y(), scrollAreaWidth, rect.height());
    mLightContainer->setFixedWidth(rect.width());
    mLightContainer->moveLightWidgets(QSize(this->width(), buttonHeight), QPoint(0, 0));
}


void LightsListMenu::updateLights() {
    mLightContainer->updateLightWidgets(mComm->lightsByIDs(mLights));
}

void LightsListMenu::addLight(const QString& ID) {
    // check if light exists
    auto result = std::find(mLights.begin(), mLights.end(), ID);
    if (result == mLights.end()) {
        mLights.push_back(ID);
    }
    mLightContainer->updateLightWidgets(mComm->lightsByIDs(mLights));
    mLightContainer->showLights(mComm->lightsByIDs(mLights), mRowHeight);
}

void LightsListMenu::removeLight(const QString& ID) {
    auto result = std::find(mLights.begin(), mLights.end(), ID);
    if (result != mLights.end()) {
        mLights.erase(result);
    }
    mLightContainer->showLights(mComm->lightsByIDs(mLights), mRowHeight);
}

void LightsListMenu::showGroup(const std::vector<QString>& IDs) {
    mLights = IDs;
    mLightContainer->hideLights();
    mLightContainer->updateLightWidgets(mComm->lightsByIDs(IDs));
    mLightContainer->showLights(mComm->lightsByIDs(IDs), mRowHeight);
}

void LightsListMenu::clear() {
    mLightContainer->hideLights();
}
