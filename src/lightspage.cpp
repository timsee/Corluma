#include "lightspage.h"
#include "discovery/discoveryhuewidget.h"
#include "mainwindow.h"

LightsPage::LightsPage(QWidget* parent,
                       CommLayer* comm,
                       cor::LightList* lights,
                       AppSettings* appSettings)
    : QWidget(parent),
      mComm{comm},
      mSelectedLights{lights},
      mControllerWidget{new ControllerWidget(parent, comm, lights)},
      mDiscoveryWidget{new DiscoveryWidget(this, lights, comm, appSettings, mControllerWidget)} {
    connect(mDiscoveryWidget,
            SIGNAL(showControllerWidget()),
            this,
            SLOT(shouldShowControllerWidget()));
    connect(mDiscoveryWidget->hueWidget(),
            SIGNAL(selectLight(QString)),
            this,
            SLOT(selectLight(QString)));
    connect(mDiscoveryWidget->hueWidget(),
            SIGNAL(deselectLight(QString)),
            this,
            SLOT(deselectLight(QString)));

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

    // add light signals
    connect(mControllerWidget->arduCorWidget(),
            SIGNAL(selectLight(QString)),
            this,
            SLOT(selectLight(QString)));
    connect(mControllerWidget->arduCorWidget(),
            SIGNAL(deselectLight(QString)),
            this,
            SLOT(deselectLight(QString)));

    connect(mControllerWidget->nanoleafWidget(),
            SIGNAL(selectLight(QString)),
            this,
            SLOT(selectLight(QString)));
    connect(mControllerWidget->nanoleafWidget(),
            SIGNAL(deselectLight(QString)),
            this,
            SLOT(deselectLight(QString)));

    connect(mControllerWidget->hueWidget(),
            SIGNAL(selectLight(QString)),
            this,
            SLOT(selectLight(QString)));
    connect(mControllerWidget->hueWidget(),
            SIGNAL(deselectLight(QString)),
            this,
            SLOT(deselectLight(QString)));

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


    connect(mControllerWidget,
            SIGNAL(deleteLight(QString)),
            mDiscoveryWidget,
            SLOT(deleteLight(QString)));
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

void LightsPage::deselectLight(QString lightKey) {
    auto light = mComm->lightByID(lightKey);
    auto state = light.state();
    if (light.isReachable()) {
        mSelectedLights->removeLight(light);
        emit deselectLights({lightKey});
    }
}

void LightsPage::selectLight(QString lightKey) {
    auto light = mComm->lightByID(lightKey);
    auto state = light.state();
    if (light.isReachable()) {
        mSelectedLights->addLight(light);
        emit selectLights({lightKey});
    }
}

void LightsPage::selectAllControllerLights(QString controller, EProtocolType protocol) {
    auto lightResult = getLightsFromController(controller, protocol);
    if (lightResult.second) {
        mSelectedLights->addLights(lightResult.first);
        emit selectLights(cor::lightVectorToIDs(lightResult.first));
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
    mControllerWidget->raise();
    mDiscoveryWidget->hide();
    resize();
}

void LightsPage::hideControllerWidget() {
    mDiscoveryWidget->show();
    mControllerWidget->setVisible(false);
}
