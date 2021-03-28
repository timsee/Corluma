#include "controllerwidget.h"
#include <QPainter>
#include <QStyleOption>
#include "comm/hue/bridge.h"

ControllerWidget::ControllerWidget(QWidget* parent, CommLayer* comm, cor::LightList* selectedLights)
    : QWidget(parent),
      mComm{comm},
      mSelectedLights{selectedLights},
      mRenderTimer{new QTimer(this)},
      mTopWidget{new cor::TopWidget("", ":images/arrowLeft.png", this)},
      mArduCorWidget{new DisplayArduCorControllerWidget(this, comm, selectedLights)},
      mNanoleafWidget{new DisplayNanoleafControllerWidget(this, comm)},
      mHueBridgeWidget{new DisplayHueBridgeWidget(this, comm, selectedLights)} {
    connect(mTopWidget, SIGNAL(clicked(bool)), this, SLOT(handleBackButtonPressed(bool)));

    connect(mNanoleafWidget,
            SIGNAL(deleteNanoleaf(QString, QString)),
            this,
            SLOT(handleDeleteNanoleaf(QString, QString)));

    connect(mArduCorWidget,
            SIGNAL(deleteController(QString, EProtocolType)),
            this,
            SLOT(handleDeleteController(QString, EProtocolType)));

    connect(mHueBridgeWidget,
            SIGNAL(deleteController(QString, EProtocolType)),
            this,
            SLOT(handleDeleteController(QString, EProtocolType)));

    connect(mHueBridgeWidget,
            SIGNAL(deleteLight(QString)),
            this,
            SLOT(handleDeleteHueLight(QString)));

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
        // get recent metadata from discovery, instead of cached metadata from the nanoleaf widget
        auto metadataResult =
            mComm->nanoleaf()->findNanoLeafLight(mNanoleafWidget->metadata().serialNumber());
        auto updatedMetadata = metadataResult.first;
        if (!metadataResult.second) {
            updatedMetadata = mNanoleafWidget->metadata();
        }
        if (lightResult.second) {
            mNanoleafWidget->updateLeafMetadata(
                updatedMetadata,
                mNanoleafWidget->discoveryState(),
                mSelectedLights->doesLightExist(mNanoleafWidget->metadata().serialNumber()));
        }
    }

    if (mHueBridgeWidget->isVisible()) {}
}


void ControllerWidget::handleBackButtonPressed(bool) {
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
    mHueBridgeWidget->showWidget();
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

void ControllerWidget::updateLightNames(EProtocolType) {
    // qDebug() << "TODO: update the light names for ControllerWidget";
}

void ControllerWidget::handleDeleteHueLight(QString uniqueID) {
    qDebug() << " TODO: delete hue " << uniqueID;
    auto light = mComm->lightByID(uniqueID);
    if (light.isValid()) {
        mComm->hue()->deleteLight(light);
    } else {
        qDebug() << "requested to delete an unknown hue: " << uniqueID;
    }
}

void ControllerWidget::handleDeleteNanoleaf(QString serialNumber, QString IP) {
    // delete the light from the comm layer, which signals to delete it from groups
    bool result = mComm->nanoleaf()->deleteNanoleaf(serialNumber, IP);
    if (!result) {
        qDebug() << "WARNING: error deleting " << serialNumber << " IP: " << IP;
    } else {
        // delete the light
        emit deleteLight(serialNumber);
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
        auto bridgeResult = mComm->hue()->discovery()->bridgeFromDiscoveryID(uniqueID);
        if (bridgeResult.second) {
            auto bridge = bridgeResult.first;
            // gather light UUIDs from bridge
            // TODO: this currently errors out for hue not found.
            //            for (const auto& light : bridge.lights().items()) {
            //                lightNames.push_back(light.uniqueID());
            //            }
            result = mComm->hue()->discovery()->deleteBridge(bridgeResult.first);
        }
    }

    if (!result) {
        qDebug() << "WARNING: error deleting " << uniqueID
                 << " protocol: " << protocolToString(protocol);
    } else {
        qDebug() << "INFO: Deleted controller: " << uniqueID
                 << " protocol: " << protocolToString(protocol);
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
}

void ControllerWidget::handleDeletedLights(const std::vector<QString>& keys) {
    mHueBridgeWidget->removeLights(keys);
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
