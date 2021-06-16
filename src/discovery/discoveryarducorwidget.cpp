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
      mSelectedLights{selectedLights},
      mListWidget{new cor::ListWidget(this, cor::EListType::linear)},
      mPlaceholderWidget{
          new ListPlaceholderWidget(this,
                                    "No ArduCor lights have been discovered. Press the plus button "
                                    "and enter an IP address to connect to a new light.")},
      mHasLights{false} {
    mListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    QScroller::grabGesture(mListWidget->viewport(), QScroller::LeftMouseButtonGesture);
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
    mHasLights = (!foundControllers.empty() || !undiscoveredControllers.empty());

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

    if (mHasLights) {
        mListWidget->setVisible(true);
        mPlaceholderWidget->setVisible(false);
    } else {
        mListWidget->setVisible(false);
        mPlaceholderWidget->setVisible(true);
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
    auto yPos = height() * 0.05;
    auto lightRect =
        QRect(int(width() * 0.03), yPos, int(width() - width() * 0.06), int(height() * 0.9));
    mListWidget->setGeometry(lightRect);
    mPlaceholderWidget->setGeometry(lightRect);
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
    mListWidget->mainWidget()->setFixedWidth(width() * 0.9);

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

void DiscoveryArduCorWidget::deleteLight(const cor::LightID& light) {
    std::vector<QString> widgetsToRemove;
    for (auto widget : mListWidget->widgets()) {
        auto arduCorWidget = dynamic_cast<DisplayPreviewArduCorWidget*>(widget);
        if (arduCorWidget->controller().name() == light.toString()) {
            auto lightResult = std::find(mIgnoredLights.begin(), mIgnoredLights.end(), light);
            if (lightResult == mIgnoredLights.end()) {
                mIgnoredLights.push_back(light.toString());
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
    std::stringstream sstream;
    sstream << "<b>General Tips</b>";
    sstream << "<ul>";
    sstream << "<li> The Arduino should be running v3.3.3 or later of <a"
               "href=\"https://github.com/timsee/arducor\">ArduCor</a>.</li>";
    sstream << "<li> Both UDP and HTML is supported from Arduino Yuns. UDP is recommended due to "
               "less latency and higher bandwidth.</li>";
    sstream
        << "<li> ArduCor samples that use a Raspberry Pi as passthrough are also supported.</li>";
#ifndef MOBILE_BUILD
#ifdef USE_SERIAL
    sstream << "<li> This build supports serial connections. Lights connected via a serial port "
               "will be discovered automatically.</li>";
#else
    sstream << "<li> This build does <b>not</b> support serial connections directly, serial lights "
               "must pass through a Raspberry Pi.</li>";
#endif // USE_SERIAL
#endif // MOBILE_BUILD
    sstream << "<li> The names of the lights are assumed to be unique. Controlling multiple lights "
               "with the same name may result in unexpected behavior.</li>";
    sstream << "</ul>";

    sstream << "<b>Debugging Connections</b>";
    sstream << "<ul>";
    if (mComm->discoveryErrorsExist(EProtocolType::arduCor)) {
        sstream << "<li> ERROR: Cannot connect to the UDP port for UDP connections. Please close "
                   "any applications that may be connected to 10008.</li>";
    } else {
        sstream << "<li> UDP port is bound, UDP lights can connect succesfully.</li>";
    }
#ifdef USE_EXPERIMENTAL_FEATURES
    sstream << "<li> Discovery Time: "
            << cor::makePrettyTimeOutput(mComm->arducor()->discovery()->lastDiscoveryTime())
                   .toStdString()
            << " </li>";
    sstream << "<li> Last Send Time: "
            << cor::makePrettyTimeOutput(mComm->arducor()->lastSendTime()).toStdString()
            << " </li>";
    sstream << "<li> Last Receive Time: "
            << cor::makePrettyTimeOutput(mComm->arducor()->lastReceiveTime()).toStdString()
            << " </li>";
#endif
    sstream << "</ul>";
    return QString(sstream.str().c_str());
}
