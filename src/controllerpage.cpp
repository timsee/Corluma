#include "controllerpage.h"
#include <QPainter>
#include <QStyleOption>

ControllerPage::ControllerPage(QWidget* parent, CommLayer* comm, cor::LightList* selectedLights)
    : QWidget(parent),
      mComm{comm},
      mSelectedLights{selectedLights},
      mRenderTimer{new QTimer(this)},
      mTopWidget{new cor::TopWidget("", ":images/arrowLeft.png", this)},
      mArduCorWidget{new DisplayArduCorControllerWidget(this, comm, selectedLights)},
      mNanoleafWidget{new DisplayNanoleafControllerWidget(this, comm)},
      mHueBridgeWidget{new DisplayHueBridgeWidget(this, comm, selectedLights)} {
    connect(mTopWidget, SIGNAL(clicked(bool)), this, SLOT(backButtonPressed(bool)));

    connect(mNanoleafWidget, SIGNAL(deleteLight(QString)), this, SLOT(handleDeleteLight(QString)));
    connect(mNanoleafWidget,
            SIGNAL(lightClicked(QString, bool)),
            this,
            SLOT(lightClicked(QString, bool)));

    connect(mArduCorWidget,
            SIGNAL(deleteController(QString, EProtocolType)),
            this,
            SLOT(handleDeleteController(QString, EProtocolType)));
    connect(mArduCorWidget,
            SIGNAL(controllerClicked(QString, EProtocolType, bool)),
            this,
            SLOT(controllerClicked(QString, EProtocolType, bool)));
    connect(mArduCorWidget,
            SIGNAL(lightClicked(QString, bool)),
            this,
            SLOT(lightClicked(QString, bool)));

    connect(mHueBridgeWidget,
            SIGNAL(deleteController(QString, EProtocolType)),
            this,
            SLOT(handleDeleteController(QString, EProtocolType)));
    connect(mHueBridgeWidget,
            SIGNAL(controllerClicked(QString, EProtocolType, bool)),
            this,
            SLOT(controllerClicked(QString, EProtocolType, bool)));
    connect(mHueBridgeWidget,
            SIGNAL(lightClicked(QString, bool)),
            this,
            SLOT(lightClicked(QString, bool)));

    connect(mRenderTimer, SIGNAL(timeout()), this, SLOT(renderUI()));
    mRenderTimer->start(333);
}

void ControllerPage::showPage(QPoint topLeft) {
    setVisible(true);
    setGeometry(topLeft.x(), topLeft.y(), width(), height());
    raise();
    show();
    isOpen(true);
}

void ControllerPage::hidePage() {
    setVisible(false);
    isOpen(false);
}


void ControllerPage::renderUI() {
    if (mNanoleafWidget->isVisible()) {
        auto lightResult = mComm->nanoleaf()->lightFromMetadata(mNanoleafWidget->metadata());
        if (lightResult.second) {
            mNanoleafWidget->updateLeafMetadata(
                mNanoleafWidget->metadata(),
                mSelectedLights->doesLightExist(mNanoleafWidget->metadata().serialNumber()));
        }
    }

    if (mHueBridgeWidget->isVisible()) {}
}


void ControllerPage::backButtonPressed(bool) {
    emit backButtonPressed();
}

void ControllerPage::showArduCor(const cor::Controller& controller) {
    mArduCorWidget->updateController(controller);
    mArduCorWidget->setVisible(true);
    mNanoleafWidget->setVisible(false);
    mHueBridgeWidget->setVisible(false);
}

void ControllerPage::showNanoleaf(const nano::LeafMetadata& metadata) {
    mNanoleafWidget->updateLeafMetadata(metadata,
                                        mSelectedLights->doesLightExist(metadata.serialNumber()));
    mArduCorWidget->setVisible(false);
    mNanoleafWidget->setVisible(true);
    mHueBridgeWidget->setVisible(false);
}

void ControllerPage::showHueBridge(const hue::Bridge& bridge) {
    mHueBridgeWidget->updateBridge(bridge);
    mArduCorWidget->setVisible(false);
    mNanoleafWidget->setVisible(false);
    mHueBridgeWidget->setVisible(true);
}

void ControllerPage::changeRowHeight(int height) {
    mArduCorWidget->changeRowHeight(height);
    mNanoleafWidget->changeRowHeight(height);
    mHueBridgeWidget->changeRowHeight(height);
}

void ControllerPage::handleDeleteLight(QString uniqueID) {
    // delete the light
    emit deleteLight(uniqueID);
    // close the page, it will no longer exist
    emit backButtonPressed();
}

void ControllerPage::handleDeleteController(QString uniqueID, EProtocolType protocol) {
    qDebug() << " delete controller " << uniqueID;
    if (protocol == EProtocolType::arduCor) {
        // mComm->arducor()->deleteController(uniqueID);
    } else if (protocol == EProtocolType::hue) {
        //
    }
}

void ControllerPage::highlightLights() {
    if (mArduCorWidget->isVisible()) {
        mArduCorWidget->highlightLights();
    }

    if (mHueBridgeWidget->isVisible()) {
        mHueBridgeWidget->highlightLights();
    }
}

void ControllerPage::lightClicked(QString lightKey, bool) {
    auto light = mComm->lightByID(lightKey);
    auto state = light.state();
    if (light.isReachable()) {
        if (mSelectedLights->doesLightExist(light)) {
            mSelectedLights->removeLight(light);
            emit lightSelected(lightKey, false);
        } else {
            mSelectedLights->addLight(light);
            emit lightSelected(lightKey, true);
        }
    }
}

void ControllerPage::controllerClicked(QString controller, EProtocolType protocol, bool selectAll) {
    if (protocol == EProtocolType::arduCor) {
        auto controllerResult =
            mComm->arducor()->discovery()->findControllerByControllerName(controller);
        if (controllerResult.second) {
            auto lights = mComm->arducor()->lightsFromNames(controllerResult.first.names());
            if (selectAll) {
                mSelectedLights->addLights(lights);
            } else {
                mSelectedLights->removeLights(lights);
            }
            for (auto light : lights) {
                emit lightSelected(light.uniqueID(), selectAll);
            }
        }
    } else if (protocol == EProtocolType::hue) {
        auto controllerResult = mComm->hue()->discovery()->bridgeFromID(controller);
        if (controllerResult.second) {
            auto lights = mComm->hue()->lightsFromMetadata(controllerResult.first.lights().items());
            if (selectAll) {
                mSelectedLights->addLights(lights);
            } else {
                mSelectedLights->removeLights(lights);
            }
            for (auto light : lights) {
                emit lightSelected(light.uniqueID(), selectAll);
            }
        }
    }
}

void ControllerPage::resizeEvent(QResizeEvent*) {
    auto yPos = 0u;
    mTopWidget->setGeometry(0, 0, this->width(), this->height() / 12);
    yPos += mTopWidget->height();

    mArduCorWidget->setGeometry(0, yPos, this->width(), this->height() * 11 / 12);
    mNanoleafWidget->setGeometry(0, yPos, this->width(), this->height() * 11 / 12);
    mHueBridgeWidget->setGeometry(0, yPos, this->width(), this->height() * 11 / 12);
}

void ControllerPage::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}
