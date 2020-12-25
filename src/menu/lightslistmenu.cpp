#include "lightslistmenu.h"
#include <QScrollBar>
#include <QScroller>

LightsListMenu::LightsListMenu(QWidget* parent, bool allowInteraction)
    : QWidget(parent),
      mScrollArea{new QScrollArea(this)},
      mLightContainer{new MenuLightContainer(mScrollArea, allowInteraction)},
      mRowHeight{10},
      mSingleLightMode{false} {
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


void LightsListMenu::resize(const QRect& inputRect, int buttonHeight) {
    setGeometry(inputRect);
    int offsetY = 0u;
    mRowHeight = buttonHeight;

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

void LightsListMenu::updateLights() {
    mLightContainer->updateLightWidgets(mLights);
}

void LightsListMenu::addLight(const cor::Light& light) {
    auto vectorLight = cor::findLightInVectorByID(mLights, light);
    if (vectorLight.isValid()) {
        auto result = std::find(mLights.begin(), mLights.end(), vectorLight);
        if (result != mLights.end()) {
            mLights.erase(result);
        }
    }
    mLights.push_back(light);
    mLightContainer->updateLightWidgets(mLights);
    mLightContainer->showLights(mLights, mRowHeight);
}

void LightsListMenu::removeLight(const cor::Light& light) {
    auto vectorLight = cor::findLightInVectorByID(mLights, light);
    auto result = std::find(mLights.begin(), mLights.end(), vectorLight);
    if (result != mLights.end()) {
        mLights.erase(result);
    } else {
        qDebug() << "ERROR: light not found, shouldn't get here " << light;
    }
    mLightContainer->showLights(mLights, mRowHeight);
}

void LightsListMenu::showLights(const std::vector<cor::Light>& lights) {
    mLights = lights;
    mLightContainer->updateLightWidgets(lights);
    mLightContainer->showLights(lights, mRowHeight);
}

void LightsListMenu::clear() {
    mLightContainer->showLights({}, mRowHeight);
}
