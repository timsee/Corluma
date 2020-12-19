#include "lightspage.h"
#include "mainwindow.h"

LightsPage::LightsPage(QWidget* parent,
                       CommLayer* comm,
                       cor::LightList* lights,
                       AppSettings* appSettings)
    : QWidget(parent),
      mControllerWidget{new ControllerWidget(parent, comm, lights)},
      mDiscoveryWidget{new DiscoveryWidget(this, lights, comm, appSettings, mControllerWidget)} {
    connect(mDiscoveryWidget,
            SIGNAL(showControllerWidget()),
            this,
            SLOT(shouldShowControllerWidget()));

    mControllerWidget->setVisible(false);
    connect(mControllerWidget, SIGNAL(backButtonPressed()), this, SLOT(hideControllerWidget()));

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
