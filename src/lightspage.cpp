#include "lightspage.h"
#include "discovery/discoveryhuewidget.h"
#include "mainwindow.h"
#include "topmenu.h"

LightsPage::LightsPage(QWidget* parent,
                       CommLayer* comm,
                       PaletteData* palettes,
                       cor::LightList* lights,
                       AppSettings* appSettings)
    : QWidget(parent),
      mComm{comm},
      mSelectedLights{lights},
      mControllerWidget{new ControllerWidget(parent, palettes, comm, lights)},
      mDiscoveryWidget{new DiscoveryWidget(this, lights, comm, appSettings, mControllerWidget)} {
    connect(mDiscoveryWidget,
            SIGNAL(showControllerWidget()),
            this,
            SLOT(shouldShowControllerWidget()));

    connect(mDiscoveryWidget,
            SIGNAL(connectionStateChanged(EProtocolType, EConnectionState)),
            this,
            SLOT(handleConnectionStateChanged(EProtocolType, EConnectionState)));

    connect(mDiscoveryWidget->hueWidget(),
            SIGNAL(selectLight(cor::LightID)),
            this,
            SLOT(selectLight(cor::LightID)));
    connect(mDiscoveryWidget->hueWidget(),
            SIGNAL(deselectLight(cor::LightID)),
            this,
            SLOT(deselectLight(cor::LightID)));

    connect(mDiscoveryWidget->hueWidget(),
            SIGNAL(selectControllerLights(QString, EProtocolType)),
            this,
            SLOT(selectAllControllerLights(QString, EProtocolType)));

    connect(mDiscoveryWidget->hueWidget(),
            SIGNAL(deselectControllerLights(QString, EProtocolType)),
            this,
            SLOT(deselectAllControllerLights(QString, EProtocolType)));

    connect(mDiscoveryWidget->hueWidget(),
            SIGNAL(deleteController(QString, EProtocolType)),
            this,
            SLOT(deleteControllerFromDiscovery(QString, EProtocolType)));

    mControllerWidget->setVisible(false);
    connect(mControllerWidget, SIGNAL(backButtonPressed()), this, SLOT(hideControllerWidget()));
    connect(mControllerWidget,
            SIGNAL(deleteLight(cor::LightID)),
            this,
            SLOT(handleDeleteLight(cor::LightID)));

    // add light signals
    connect(mControllerWidget->arduCorWidget(),
            SIGNAL(selectLight(cor::LightID)),
            this,
            SLOT(selectLight(cor::LightID)));
    connect(mControllerWidget->arduCorWidget(),
            SIGNAL(deselectLight(cor::LightID)),
            this,
            SLOT(deselectLight(cor::LightID)));

    connect(mControllerWidget->nanoleafWidget(),
            SIGNAL(selectLight(cor::LightID)),
            this,
            SLOT(selectLight(cor::LightID)));
    connect(mControllerWidget->nanoleafWidget(),
            SIGNAL(deselectLight(cor::LightID)),
            this,
            SLOT(deselectLight(cor::LightID)));
    connect(mControllerWidget->nanoleafWidget(),
            SIGNAL(selectEffect(cor::LightID, QString)),
            this,
            SLOT(selectEffect(cor::LightID, QString)));

    connect(mControllerWidget->hueWidget(),
            SIGNAL(selectLight(cor::LightID)),
            this,
            SLOT(selectLight(cor::LightID)));
    connect(mControllerWidget->hueWidget(),
            SIGNAL(deselectLight(cor::LightID)),
            this,
            SLOT(deselectLight(cor::LightID)));

    // add controller signals
    connect(mControllerWidget->arduCorWidget(),
            SIGNAL(selectControllerLights(QString, EProtocolType)),
            this,
            SLOT(selectAllControllerLights(QString, EProtocolType)));
    connect(mControllerWidget->arduCorWidget(),
            SIGNAL(deselectControllerLights(QString, EProtocolType)),
            this,
            SLOT(deselectAllControllerLights(QString, EProtocolType)));

    connect(mControllerWidget->hueWidget(),
            SIGNAL(selectControllerLights(QString, EProtocolType)),
            this,
            SLOT(selectAllControllerLights(QString, EProtocolType)));
    connect(mControllerWidget->hueWidget(),
            SIGNAL(deselectControllerLights(QString, EProtocolType)),
            this,
            SLOT(deselectAllControllerLights(QString, EProtocolType)));
    connect(mControllerWidget->hueWidget(),
            SIGNAL(controllerNameChanged(QString, QString)),
            this,
            SLOT(handleControllerNameChanged(QString, QString)));
    connect(mControllerWidget->hueWidget(),
            SIGNAL(lightNameChanged(cor::LightID, QString)),
            this,
            SLOT(handleLightNameChanged(cor::LightID, QString)));

    connect(mControllerWidget,
            SIGNAL(deleteLight(cor::LightID)),
            mDiscoveryWidget,
            SLOT(deleteLight(cor::LightID)));

    // connect the comm layer when it finds new lights
    connect(mComm,
            SIGNAL(lightsAdded(std::vector<cor::LightID>)),
            this,
            SLOT(handleNewLightsFound(std::vector<cor::LightID>)));
}

void LightsPage::setupTopMenu(TopMenu* topMenu) {
    connect(topMenu,
            SIGNAL(buttonPressed(QString)),
            mDiscoveryWidget,
            SLOT(floatingLayoutButtonPressed(QString)));
}

void LightsPage::resize() {
    mDiscoveryWidget->setGeometry(QRect(0, 0, width(), height()));
    if (mControllerWidget->isVisible()) {
        auto mainWindow = cor::mainWindow();
        if (mainWindow->leftHandMenu()->alwaysOpen()) {
            auto rect = QRect(mainWindow->leftHandMenu()->width(),
                              0,
                              mainWindow->width() - mainWindow->leftHandMenu()->width(),
                              mainWindow->height());
            mControllerWidget->setGeometry(rect);
        } else {
            auto rect = QRect(0, 0, mainWindow->width(), mainWindow->height());
            mControllerWidget->setGeometry(rect);
        }
    }
}

void LightsPage::handleNewLightsFound(std::vector<cor::LightID> uniqueIDs) {
    auto lights = mComm->lightsByIDs(uniqueIDs);
    std::vector<cor::LightID> hueLights;
    for (auto light : lights) {
        if (light.protocol() == EProtocolType::hue) {
            hueLights.push_back(light.uniqueID());
        }
    }
    if (!hueLights.empty()) {
        mControllerWidget->hueWidget()->newHuesFound(hueLights);
        mDiscoveryWidget->hueWidget()->newHuesFound(hueLights);
    }
}

void LightsPage::deselectLight(cor::LightID lightKey) {
    auto light = mComm->lightByID(lightKey);
    if (light.isReachable()) {
        mSelectedLights->removeLight(light);
        emit deselectLights({lightKey});
    }
}

void LightsPage::selectLight(cor::LightID lightKey) {
    auto light = mComm->lightByID(lightKey);
    if (light.isReachable()) {
        mSelectedLights->addLight(light);
        emit selectLights({lightKey});
    }
}

void LightsPage::selectEffect(cor::LightID lightKey, QString effectKey) {
    auto light = mComm->lightByID(lightKey);
    auto state = light.state();
    if (light.isReachable()) {
        state.isOn(true);
        state.effect(effectKey);
        light.state(state);
        mSelectedLights->addEffect(light);
        emit selectLights({lightKey});
    }
}

void LightsPage::selectAllControllerLights(QString controller, EProtocolType protocol) {
    auto lightResult = getLightsFromController(controller, protocol);
    if (lightResult.second) {
        std::vector<cor::Light> reachableLights;
        for (auto light : lightResult.first) {
            if (light.isReachable()) {
                reachableLights.push_back(light);
            }
        }
        mSelectedLights->addLights(reachableLights);
        emit selectLights(cor::lightVectorToIDs(reachableLights));
    }
}

void LightsPage::deselectAllControllerLights(QString controller, EProtocolType protocol) {
    auto lightResult = getLightsFromController(controller, protocol);
    if (lightResult.second) {
        mSelectedLights->removeLights(lightResult.first);
        emit deselectLights(cor::lightVectorToIDs(lightResult.first));
    }
}

std::pair<std::vector<cor::Light>, bool> LightsPage::getLightsFromController(
    const QString& controllerName,
    EProtocolType protocol) {
    if (protocol == EProtocolType::arduCor) {
        auto controllerResult =
            mComm->arducor()->discovery()->findFoundControllerByControllerName(controllerName);
        if (controllerResult.second) {
            auto lights = mComm->arducor()->lightsFromNames(controllerResult.first.names());
            return std::make_pair(lights, true);
        }
    } else if (protocol == EProtocolType::hue) {
        auto controllerResult = mComm->hue()->discovery()->bridgeFromID(controllerName);
        if (controllerResult.second) {
            auto lights = mComm->hue()->lightsFromMetadata(controllerResult.first.lights().items());
            return std::make_pair(lights, true);
        }
    }
    return std::make_pair(std::vector<cor::Light>{}, false);
}

void LightsPage::deleteControllerFromDiscovery(QString key, EProtocolType protocol) {
    mControllerWidget->handleDeleteController(key, protocol);
}

void LightsPage::handleControllerNameChanged(QString key, QString name) {
    qDebug() << "INFO: Updated name of bridge: " << key << " to " << name;
    mDiscoveryWidget->hueWidget()->handleBridgeNameUpdate(key, name);
}

void LightsPage::handleLightNameChanged(cor::LightID key, QString name) {
    auto light = mComm->lightByID(key);
    if (light.protocol() == EProtocolType::hue) {
        // get hue light from key
        std::vector<HueMetadata> hueLights = mComm->hue()->discovery()->lights();
        HueMetadata light;
        bool lightFound = false;
        for (auto hue : hueLights) {
            if (hue.uniqueID() == key) {
                lightFound = true;
                light = hue;
            }
        }

        if (lightFound) {
            /// this sends a rename packet, which asks the Hue Bridge to rename the light. The
            /// bridge will send a sucess packet, which will be handled by Commhue.
            mComm->hue()->renameLight(light, name);
        } else {
            qDebug() << " could NOT change this key: " << key.toString() << " to this name "
                     << name;
        }
    }
}

void LightsPage::handleDeleteLight(cor::LightID key) {
    emit deleteLights({key});
}

void LightsPage::handleConnectionStateChanged(EProtocolType type, EConnectionState state) {
    emit connectionStateChanged(type, state);
}

void LightsPage::updateLightNames(EProtocolType protocol) {
    mDiscoveryWidget->updateLightNames(protocol);
    mControllerWidget->updateLightNames(protocol);
}

void LightsPage::handleDeletedLights(const std::vector<cor::LightID>& keys) {
    mDiscoveryWidget->handleDeletedLights(keys);
    mControllerWidget->handleDeletedLights(keys);
}

void LightsPage::resizeEvent(QResizeEvent*) {
    resize();
}

void LightsPage::showWidgets() {
    mDiscoveryWidget->setVisible(true);
    mDiscoveryWidget->show();
}

void LightsPage::hideWidgets() {
    mDiscoveryWidget->hide();
    mDiscoveryWidget->setVisible(false);
    mControllerWidget->setVisible(false);
}

void LightsPage::shouldShowControllerWidget() {
    mControllerWidget->setVisible(true);
    mDiscoveryWidget->isOpen(false);
    mControllerWidget->isOpen(true);
    mControllerWidget->raise();
    mDiscoveryWidget->hide();
    resize();
}

void LightsPage::hideControllerWidget() {
    mDiscoveryWidget->show();
    mDiscoveryWidget->isOpen(true);
    mControllerWidget->isOpen(false);
    mControllerWidget->setVisible(false);
}
