#include "controllerwidget.h"
#include <QPainter>
#include <QStyleOption>

ControllerWidget::ControllerWidget(QWidget* parent, CommLayer* comm, cor::LightList* selectedLights)
    : QWidget(parent),
      mComm{comm},
      mSelectedLights{selectedLights},
      mRenderTimer{new QTimer(this)},
      mTopWidget{new cor::TopWidget("", ":images/arrowLeft.png", this)},
      mArduCorWidget{new DisplayArduCorControllerWidget(this, comm, selectedLights)},
      mNanoleafWidget{new DisplayNanoleafControllerWidget(this, comm)},
      mHueBridgeWidget{new DisplayHueBridgeWidget(this, comm, selectedLights)} {
    connect(mTopWidget, SIGNAL(clicked(bool)), this, SLOT(backButtonPressed(bool)));

    connect(mNanoleafWidget,
            SIGNAL(deleteNanoleaf(QString, QString)),
            this,
            SLOT(handleDeleteNanoleaf(QString, QString)));
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

void ControllerWidget::showPage(QPoint topLeft) {
    setVisible(true);
    setGeometry(topLeft.x(), topLeft.y(), width(), height());
    raise();
    show();
    isOpen(true);
}

void ControllerWidget::hidePage() {
    setVisible(false);
    isOpen(false);
}


void ControllerWidget::renderUI() {
    if (mNanoleafWidget->isVisible()) {
        auto lightResult = mComm->nanoleaf()->lightFromMetadata(mNanoleafWidget->metadata());
        if (lightResult.second) {
            mNanoleafWidget->updateLeafMetadata(
                mNanoleafWidget->metadata(),
                mNanoleafWidget->discoveryState(),
                mSelectedLights->doesLightExist(mNanoleafWidget->metadata().serialNumber()));
        }
    }

    if (mHueBridgeWidget->isVisible()) {}
}


void ControllerWidget::backButtonPressed(bool) {
    emit backButtonPressed();
}

void ControllerWidget::showArduCor(const cor::Controller& controller, cor::EArduCorStatus status) {
    mArduCorWidget->updateController(controller, status);
    mArduCorWidget->setVisible(true);
    mNanoleafWidget->setVisible(false);
    mHueBridgeWidget->setVisible(false);
}

void ControllerWidget::showNanoleaf(const nano::LeafMetadata& metadata,
                                    nano::ELeafDiscoveryState discoveryState) {
    mNanoleafWidget->updateLeafMetadata(metadata,
                                        discoveryState,
                                        mSelectedLights->doesLightExist(metadata.serialNumber()));
    mArduCorWidget->setVisible(false);
    mNanoleafWidget->setVisible(true);
    mHueBridgeWidget->setVisible(false);
}

void ControllerWidget::showHueBridge(const hue::Bridge& bridge) {
    mHueBridgeWidget->updateBridge(bridge);
    mArduCorWidget->setVisible(false);
    mNanoleafWidget->setVisible(false);
    mHueBridgeWidget->setVisible(true);
}

void ControllerWidget::changeRowHeight(int height) {
    mArduCorWidget->changeRowHeight(height);
    mNanoleafWidget->changeRowHeight(height);
    mHueBridgeWidget->changeRowHeight(height);
}

void ControllerWidget::handleDeleteLight(QString uniqueID) {
    // delete the light
    emit deleteLight(uniqueID);

    auto light = mComm->lightByID(uniqueID);

    // close the page, it will no longer exist
    emit backButtonPressed();
}

void ControllerWidget::handleDeleteNanoleaf(QString serialNumber, QString IP) {
    // delete the light
    emit deleteLight(serialNumber);

    // delete the light from the comm layer, which signals to delete it from groups
    bool result = mComm->nanoleaf()->deleteNanoleaf(serialNumber, IP);
    if (!result) {
        qDebug() << "WARNING: error deleting " << serialNumber << " IP: " << IP;
    } else {
        // close the page, it will no longer exist
        emit backButtonPressed();
    }
}

void ControllerWidget::handleDeleteController(QString uniqueID, EProtocolType protocol) {
    // get controller names, if they exist
    std::vector<QString> lightNames;
    bool result = false;
    if (protocol == EProtocolType::arduCor) {
        auto controllerResult =
            mComm->arducor()->discovery()->findControllerByControllerName(uniqueID);
        if (controllerResult.second) {
            lightNames = controllerResult.first.names();
        }
        result = mComm->arducor()->deleteController(uniqueID);

    } else if (protocol == EProtocolType::hue) {
        //  mComm->hue()->discovery()->deleteBridge(bridge);
    }

    if (!result) {
        qDebug() << "WARNING: error deleting " << uniqueID << " IP: " << protocolToString(protocol);
    } else {
        emit deleteLight(uniqueID);
        for (const auto& light : lightNames) {
            emit deleteLight(light);
        }

        // close the page, it will no longer exist
        emit backButtonPressed();
    }
}

void ControllerWidget::highlightLights() {
    if (mArduCorWidget->isVisible()) {
        mArduCorWidget->highlightLights();
    }

    if (mHueBridgeWidget->isVisible()) {
        mHueBridgeWidget->highlightLights();
    }
}

void ControllerWidget::lightClicked(QString lightKey, bool) {
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

void ControllerWidget::controllerClicked(QString controller,
                                         EProtocolType protocol,
                                         bool selectAll) {
    if (protocol == EProtocolType::arduCor) {
        auto controllerResult =
            mComm->arducor()->discovery()->findFoundControllerByControllerName(controller);
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

void ControllerWidget::resizeEvent(QResizeEvent*) {
    auto yPos = 0u;
    mTopWidget->setGeometry(0, 0, this->width(), this->height() / 12);
    yPos += mTopWidget->height();

    mArduCorWidget->setGeometry(0, yPos, this->width(), this->height() * 11 / 12);
    mNanoleafWidget->setGeometry(0, yPos, this->width(), this->height() * 11 / 12);
    mHueBridgeWidget->setGeometry(0, yPos, this->width(), this->height() * 11 / 12);
}

void ControllerWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}
