/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "discoveryarducorwidget.h"
#include <QMessageBox>
#include <QScroller>
#include "controllerwidget.h"
#include "display/displaypreviewarducorwidget.h"
#include "mainwindow.h"
#include "utils/qt.h"

DiscoveryArduCorWidget::DiscoveryArduCorWidget(QWidget* parent,
                                               CommLayer* comm,
                                               cor::LightList* selectedLights,
                                               ControllerWidget* controllerPage)
    : DiscoveryTypeWidget(parent, comm, controllerPage),
      mSelectedLights{selectedLights} {
    mListWidget = new cor::ListWidget(this, cor::EListType::linear);
    mListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    QScroller::grabGesture(mListWidget->viewport(), QScroller::LeftMouseButtonGesture);

    mTopLabel = new QLabel(this);
    mTopLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mTopLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
}

void DiscoveryArduCorWidget::checkIfIPExists(const QString& IP) {
    if (!mComm->arducor()->discovery()->doesIPExist(IP)) {
        mComm->arducor()->discovery()->addManualIP(IP);
        closeIPWidget();
    } else {
        QMessageBox reply;
        reply.setText("IP Address already exists.");
        reply.exec();
    }
}

void DiscoveryArduCorWidget::handleDiscovery(bool) {
    const auto& foundControllers = mComm->arducor()->discovery()->controllers().items();
    for (const auto& controller : foundControllers) {
        handleController(controller, cor::EArduCorStatus::connected);
    }

    const auto& undiscoveredControllers = mComm->arducor()->discovery()->undiscoveredControllers();
    for (const auto& controller : undiscoveredControllers) {
        handleController(controller, cor::EArduCorStatus::searching);
    }

    // handle button updates
    if (mComm->discoveryErrorsExist(EProtocolType::arduCor)) {
        emit connectionStatusChanged(EProtocolType::arduCor, EConnectionState::connectionError);
    } else if (undiscoveredControllers.empty() && !foundControllers.empty()) {
        emit connectionStatusChanged(EProtocolType::arduCor, EConnectionState::discovered);
    } else if (!foundControllers.empty()) {
        emit connectionStatusChanged(EProtocolType::arduCor, EConnectionState::discovering);
    } else {
        emit connectionStatusChanged(EProtocolType::arduCor, EConnectionState::off);
    }
}


void DiscoveryArduCorWidget::handleController(const cor::Controller& controller,
                                              cor::EArduCorStatus status) {
    // check if light already exists in list
    bool foundWidget = false;
    int i = 0;
    for (const auto& widget : mListWidget->widgets()) {
        auto arduCorWidget = dynamic_cast<DisplayPreviewArduCorWidget*>(widget);
        if (arduCorWidget->controller().name().isEmpty() && widget->key() == controller.name()) {
        } else if (widget->key() == controller.name()) {
            // standard case, theres a unique ID for this bridge
            foundWidget = true;
            auto lights = mComm->arducor()->lightsFromNames(controller.names());
            arduCorWidget->updateController(controller, lights, status);
        }
        ++i;
    }

    // if it doesnt exist and it isn't ignored, add it
    if (!foundWidget
        && (std::find(mIgnoredLights.begin(), mIgnoredLights.end(), controller.name())
            == mIgnoredLights.end())) {
        auto widget = new DisplayPreviewArduCorWidget(controller,
                                                      status,
                                                      mSelectedLights,
                                                      mListWidget->mainWidget());
        widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        connect(widget, SIGNAL(clicked(QString)), this, SLOT(controllerClicked(QString)));
        mListWidget->insertWidget(widget);
        resize();
    }
}

// ----------------------------
// Helpers
// ----------------------------


bool DiscoveryArduCorWidget::doesYunControllerExistAlready(const QString& name) {
    bool deviceFound = mComm->arducor()->discovery()->controllers().item(name.toStdString()).second;
    if (deviceFound) {
        return true;
    }

    for (const auto& undiscoveredController :
         mComm->arducor()->discovery()->undiscoveredControllers()) {
        if (undiscoveredController.name() == name) {
            deviceFound = true;
        }
    }
    return deviceFound;
}

void DiscoveryArduCorWidget::resizeEvent(QResizeEvent*) {
    resize();
}

void DiscoveryArduCorWidget::resize() {
    auto yPos = 0u;
    mTopLabel->setGeometry(0, 0, int(width() * 0.7), int(height() * 0.1));
    yPos += mTopLabel->height();
    mListWidget->setGeometry(int(width() * 0.025), yPos, int(width() * 0.95), int(height() * 0.9));
    mGreyout->resize();

    // call resize function of each widget
    auto yHeight = 0u;
    QSize widgetSize(mListWidget->width(), int(mListWidget->height() * 0.33));
    for (auto widget : mListWidget->widgets()) {
        auto arduCorWidget = dynamic_cast<DisplayPreviewArduCorWidget*>(widget);
        arduCorWidget->setGeometry(0, yHeight, widgetSize.width(), widgetSize.height());
        arduCorWidget->resize();
        yHeight += arduCorWidget->height();
    }
    mListWidget->mainWidget()->setFixedHeight(yHeight);
    mListWidget->mainWidget()->setFixedWidth(width());

    // resize the help view
    resizeHelpView();
}

void DiscoveryArduCorWidget::controllerClicked(QString controller) {
    for (auto widget : mListWidget->widgets()) {
        auto arduCorWidget = dynamic_cast<DisplayPreviewArduCorWidget*>(widget);
        if (arduCorWidget->controller().name() == controller) {
            emit showControllerWidget();
            mControllerPage->showArduCor(arduCorWidget->controller(), arduCorWidget->status());
        }
    }
}

void DiscoveryArduCorWidget::deleteLight(const QString& light) {
    std::vector<QString> widgetsToRemove;
    for (auto widget : mListWidget->widgets()) {
        auto arduCorWidget = dynamic_cast<DisplayPreviewArduCorWidget*>(widget);
        if (arduCorWidget->controller().name() == light) {
            auto lightResult = std::find(mIgnoredLights.begin(), mIgnoredLights.end(), light);
            if (lightResult == mIgnoredLights.end()) {
                mIgnoredLights.push_back(light);
                widgetsToRemove.emplace_back(arduCorWidget->key());
            }
        }
    }
    bool shouldResize = !widgetsToRemove.empty();
    for (auto widgetToRemove : widgetsToRemove) {
        mListWidget->removeWidget(widgetToRemove);
    }
    if (shouldResize) {
        resize();
    }
}

void DiscoveryArduCorWidget::greyOutClicked() {
    mGreyout->greyOut(false);
    if (mIPWidget->isOpen()) {
        closeIPWidget();
    }
}

void DiscoveryArduCorWidget::highlightLights() {
    for (auto widget : mListWidget->widgets()) {
        auto arduCorWidget = dynamic_cast<DisplayPreviewArduCorWidget*>(widget);
        arduCorWidget->highlightLights();
    }
}

QString DiscoveryArduCorWidget::discoveryHelpHTML() {
    return "TODO";
}
