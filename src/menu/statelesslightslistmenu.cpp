#include "statelesslightslistmenu.h"
#include <QScrollBar>
#include <QScroller>

StatelessLightsListMenu::StatelessLightsListMenu(QWidget* parent,
                                                 CommLayer* comm,
                                                 bool allowInteraction)
    : QWidget(parent),
      mComm{comm},
      mScrollArea{new QScrollArea(this)},
      mLightContainer{
          new MenuLightContainer(mScrollArea, allowInteraction, "StatelessLightsListMenu")},
      mRowHeight{10} {
    mLightContainer->displayState(false);
    mLightContainer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mLightContainer, SIGNAL(clickedLight(QString)), this, SLOT(lightClicked(QString)));

    mScrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QScroller::grabGesture(mScrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    mScrollArea->setWidget(mLightContainer);
    mScrollArea->setFrameStyle(QFrame::NoFrame);
    mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mScrollArea->horizontalScrollBar()->setEnabled(false);
    mScrollArea->horizontalScrollBar()->setVisible(false);
}


void StatelessLightsListMenu::resize(const QRect& inputRect, int buttonHeight) {
    setGeometry(inputRect);
    int offsetY = 0u;
    mRowHeight = buttonHeight;
    mLightContainer->changeRowHeight(mRowHeight);

    QRect rect = QRect(0, offsetY, this->width(), this->height() - offsetY);
    int scrollAreaWidth = int(rect.width() * 1.2);
    mScrollArea->setGeometry(rect.x(), rect.y(), scrollAreaWidth, rect.height());
    mLightContainer->setFixedWidth(rect.width());
    auto heightCount = mLights.size();
    if (heightCount == 0) {
        heightCount = 1;
    }
    mLightContainer->setFixedHeight(heightCount * buttonHeight);
    mLightContainer->moveLightWidgets(QSize(this->width(), buttonHeight), QPoint(0, 0));
}


void StatelessLightsListMenu::updateLights() {
    mLightContainer->updateLights(mComm->lightsByIDs(mLights));
}

void StatelessLightsListMenu::addLight(const QString& ID) {
    // check if light exists
    auto result = std::find(mLights.begin(), mLights.end(), ID);
    if (result == mLights.end()) {
        mLights.push_back(ID);
    }
    mLightContainer->addLights(mComm->lightsByIDs(mLights));
}

void StatelessLightsListMenu::removeLight(const QString& ID) {
    auto result = std::find(mLights.begin(), mLights.end(), ID);
    if (result != mLights.end()) {
        mLights.erase(result);
    }
    mLightContainer->clear();
    mLightContainer->addLights(mComm->lightsByIDs(mLights));
}

void StatelessLightsListMenu::addLights(const std::vector<QString>& IDs) {
    mLights = IDs;
    mLightContainer->addLights(mComm->lightsByIDs(mLights));
}

void StatelessLightsListMenu::clear() {
    mLightContainer->clear();
}
